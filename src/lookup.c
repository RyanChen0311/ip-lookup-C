/**
 * @file lookup.c
 * @brief IP lookup engine — trie construction and packet forwarding
 *
 * Implements the full lookup pipeline:
 *   Phase 1 — Allocate the next-hop string table.
 *   Phase 2 — Load next-hop addresses from the routing dump.
 *   Phase 3 — Parse the binary prefix file and populate prefix structs.
 *   Phase 4 — Build the binary trie from prefix structs.
 *   Phase 5 — For each destination in the trace, run LPM and write the
 *              resolved next-hop to the output file.
 *
 * Wall-clock timing for each phase is printed to stdout so performance
 * can be measured and compared across routing table snapshots.
 *
 * @author Ryan Chen
 * @date 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lookup.h"
#include "trie.h"
#include "ip_types.h"

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/** Print a timing result for a named phase. */
static void print_timing(const char *phase, clock_t elapsed)
{
    printf("[timing] %-28s %8d ticks  (%.4f s)\n",
           phase, (int)elapsed, (double)elapsed / CLOCKS_PER_SEC);
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

int run_iplookup(FILE *routing_table, FILE *trace,
                 FILE *nexthop_src,   FILE *out)
{
    if (!routing_table || !trace || !nexthop_src || !out) {
        fprintf(stderr, "[lookup] one or more file handles are NULL\n");
        return -1;
    }

    clock_t t;
    int i;

    /* ------------------------------------------------------------------
     * Phase 1 — allocate next-hop string table
     * ------------------------------------------------------------------ */
    t = clock();
    char **nexthop_table = (char **)malloc(sizeof(char *) * MAX_NEXTHOPS);
    if (!nexthop_table) {
        fprintf(stderr, "[lookup] failed to allocate nexthop table\n");
        return -1;
    }
    for (i = 0; i < MAX_NEXTHOPS; i++) {
        nexthop_table[i] = (char *)malloc(IP_BINARY_LEN + 1);
        if (!nexthop_table[i]) {
            fprintf(stderr, "[lookup] nexthop row %d allocation failed\n", i);
            /* Free already-allocated rows */
            for (int j = 0; j < i; j++) free(nexthop_table[j]);
            free(nexthop_table);
            return -1;
        }
        nexthop_table[i][0] = '\0';
    }
    print_timing("alloc nexthop table", clock() - t);

    /* ------------------------------------------------------------------
     * Phase 2 — load next-hop addresses
     * ------------------------------------------------------------------ */
    t = clock();
    char  line[1024];
    int   nh_count = 0;

    /* Skip header */
    if (fgets(line, sizeof(line), nexthop_src)) {
        while (fgets(line, sizeof(line), nexthop_src) &&
               nh_count < MAX_NEXTHOPS) {
            line[strcspn(line, "\r\n")] = '\0';
            /* Each line: <binary-prefix> <length>  — we want the prefix part */
            char *tok = strtok(line, " \t");
            if (tok) {
                strncpy(nexthop_table[nh_count], tok,
                        IP_BINARY_LEN);
                nexthop_table[nh_count][IP_BINARY_LEN] = '\0';
            }
            nh_count++;
        }
    }
    printf("[lookup] loaded %d next-hop entries\n", nh_count);
    print_timing("load nexthop table", clock() - t);

    /* ------------------------------------------------------------------
     * Phase 3 — parse binary prefix file into prefix structs
     * ------------------------------------------------------------------ */
    t = clock();
    PrefixInfo *prefixes = (PrefixInfo *)malloc(MAX_PREFIXES * sizeof(PrefixInfo));
    if (!prefixes) {
        fprintf(stderr, "[lookup] failed to allocate prefix array\n");
        for (i = 0; i < MAX_NEXTHOPS; i++) free(nexthop_table[i]);
        free(nexthop_table);
        return -1;
    }

    int p = 0;
    while (fgets(line, sizeof(line), routing_table) && p < MAX_PREFIXES) {
        line[strcspn(line, "\r\n")] = '\0';

        char *bin_part = strtok(line, " \t");
        char *len_part = strtok(NULL, " \t");

        if (!bin_part || !len_part) continue;

        prefixes[p].index = p;
        prefixes[p].len   = atoi(len_part);
        strncpy(prefixes[p].bstring, bin_part, IP_BINARY_LEN);
        p++;
    }
    printf("[lookup] loaded %d routing prefixes\n", p);
    print_timing("parse routing table", clock() - t);

    /* ------------------------------------------------------------------
     * Phase 4 — build binary trie
     * ------------------------------------------------------------------ */
    t = clock();
    TrieNode *root = NULL;
    for (i = 0; i < p; i++) {
        trie_insert(&root, prefixes[i], 0);
    }
    print_timing("build binary trie", clock() - t);
    free(prefixes);

    /* ------------------------------------------------------------------
     * Phase 5 — lookup every destination in the trace
     * ------------------------------------------------------------------ */
    t = clock();
    int    lookups  = 0;
    int    misses   = 0;
    char   dest[64];

    while (fgets(dest, sizeof(dest), trace)) {
        dest[strcspn(dest, "\r\n")] = '\0';
        if (strlen(dest) < IP_BINARY_LEN) continue;

        int idx = trie_search(&root, dest, 0);

        if (idx > 0 && idx < MAX_NEXTHOPS && nexthop_table[idx][0] != '\0') {
            fprintf(out, "%s\n", nexthop_table[idx]);
        } else {
            /* Default route (index 0) */
            fprintf(out, "%s\n", nexthop_table[0]);
            misses++;
        }
        lookups++;
    }

    print_timing("LPM lookup (all trace)", clock() - t);
    printf("[lookup] total lookups: %d  |  default-route fallbacks: %d\n",
           lookups, misses);

    /* ------------------------------------------------------------------
     * Cleanup
     * ------------------------------------------------------------------ */
    trie_free(root);
    for (i = 0; i < MAX_NEXTHOPS; i++) free(nexthop_table[i]);
    free(nexthop_table);

    return 0;
}
