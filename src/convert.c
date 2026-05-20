/**
 * @file convert.c
 * @brief IPv4 address / prefix to binary string conversion
 *
 * Converts human-readable routing data (CIDR prefixes and destination
 * IP traces) into the 32-character binary string format consumed by
 * the trie-based lookup engine.
 *
 * @author Ryan Chen
 * @date 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "convert.h"
#include "ip_types.h"

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/**
 * @brief Convert a decimal octet (0–255) to 8 binary characters in @p buf.
 *
 * Uses bit-shifting for clarity and correctness — no string reversal needed.
 * Output is NOT null-terminated; all 8 positions are written.
 */
char *dec_to_bin8(int dec, char *buf)
{
    for (int i = 7; i >= 0; i--) {
        buf[i] = (dec & 1) ? '1' : '0';
        dec >>= 1;
    }
    return buf;
}

/**
 * @brief Convert a dotted-decimal IPv4 string to 32-char binary in @p out.
 *
 * @p addr must point to a string like "192.168.1.0".
 * @p out  must be at least IP_BINARY_LEN (32) bytes.
 * @return 0 on success, -1 on parse error.
 */
static int ipv4_to_binary(const char *addr, char *out)
{
    char tmp[16];
    strncpy(tmp, addr, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char buf[OCTET_BITS];
    int  pos  = 0;
    char *tok = strtok(tmp, ".");

    while (tok && pos < IP_BINARY_LEN) {
        int octet = atoi(tok);
        if (octet < 0 || octet > 255) return -1;
        dec_to_bin8(octet, buf);
        memcpy(out + pos, buf, OCTET_BITS);
        pos += OCTET_BITS;
        tok  = strtok(NULL, ".");
    }

    if (pos != IP_BINARY_LEN) return -1;
    return 0;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

int convert_prefixes(FILE *in, FILE *out)
{
    if (!in || !out) return -1;

    char  line[1024];
    char  binary[IP_BINARY_LEN + 1];  /* +1 for null terminator in fprintf */
    int   count = 0;

    binary[IP_BINARY_LEN] = '\0';

    /* Skip header line */
    if (!fgets(line, sizeof(line), in)) return 0;

    while (fgets(line, sizeof(line), in)) {
        /* Strip trailing newline */
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        /* Split on '/' to get address and prefix length */
        char *slash  = strchr(line, '/');
        if (!slash) continue;

        *slash = '\0';
        const char *addr_part   = line;
        int         prefix_len  = atoi(slash + 1);

        if (ipv4_to_binary(addr_part, binary) != 0) {
            fprintf(stderr, "[convert] invalid prefix: %s/%d\n",
                    addr_part, prefix_len);
            continue;
        }

        fprintf(out, "%s %d\n", binary, prefix_len);
        count++;
    }

    return count;
}

int convert_trace(FILE *in, FILE *out)
{
    if (!in || !out) return -1;

    char line[1024];
    char binary[IP_BINARY_LEN + 1];
    int  count = 0;

    binary[IP_BINARY_LEN] = '\0';

    /* Skip header line */
    if (!fgets(line, sizeof(line), in)) return 0;

    while (fgets(line, sizeof(line), in)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        /* Each line may have one or more space-separated IP addresses */
        char *tok = strtok(line, " \t");
        while (tok) {
            if (ipv4_to_binary(tok, binary) == 0) {
                fprintf(out, "%s\n", binary);
                count++;
            } else {
                fprintf(stderr, "[convert] invalid trace address: %s\n", tok);
            }
            tok = strtok(NULL, " \t");
        }
    }

    return count;
}
