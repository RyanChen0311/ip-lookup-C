/**
 * @file convert.h
 * @brief IP address format conversion utilities
 *
 * Provides functions to convert dotted-decimal IPv4 prefixes and
 * trace files into their 32-bit binary string representations
 * suitable for ingestion by the binary trie.
 *
 * Input format (prefix file):
 *   @code
 *   192.168.1.0/24
 *   10.0.0.0/8
 *   @endcode
 *
 * Output format (binary file):
 *   @code
 *   11000000101010000000000100000000 24
 *   00001010000000000000000000000000 08
 *   @endcode
 *
 * @author Ryan Chen
 * @date 2022
 */

#ifndef CONVERT_H
#define CONVERT_H

#include <stdio.h>

/**
 * @brief Convert a decimal integer (0–255) to an 8-character binary string.
 *
 * The result is written into @p buf, which must be at least 8 bytes.
 * No null terminator is appended.
 *
 * @param dec  Decimal value to convert (0–255).
 * @param buf  Output buffer (minimum 8 bytes, not null-terminated).
 * @return     Pointer to @p buf.
 */
char *dec_to_bin8(int dec, char *buf);

/**
 * @brief Convert a stream of IPv4 prefixes to binary string form.
 *
 * Reads CIDR-notation lines (e.g. "10.0.0.0/8") from @p in and writes
 * the corresponding 32-char binary string and prefix length to @p out,
 * one entry per line.  The first line of @p in is treated as a header
 * and is skipped.
 *
 * @param in   Open readable FILE* for the prefix input.
 * @param out  Open writable FILE* for the binary output.
 * @return     Number of prefixes successfully converted, or -1 on error.
 */
int convert_prefixes(FILE *in, FILE *out);

/**
 * @brief Convert a stream of destination IP addresses to binary string form.
 *
 * Reads space-separated IPv4 addresses from @p in (one per line) and
 * writes the 32-char binary string to @p out, one per line.
 * The first line of @p in is treated as a header and is skipped.
 *
 * @param in   Open readable FILE* for the trace input.
 * @param out  Open writable FILE* for the binary output.
 * @return     Number of addresses successfully converted, or -1 on error.
 */
int convert_trace(FILE *in, FILE *out);

#endif /* CONVERT_H */
