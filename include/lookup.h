/**
 * @file lookup.h
 * @brief IP lookup engine — routing-table build and packet forwarding
 *
 * Encapsulates the end-to-end lookup pipeline:
 *   1. Load a binary-form routing table and build the trie.
 *   2. Load the next-hop address table into memory.
 *   3. For each destination in the trace, perform LPM and emit the
 *      resolved next-hop address to an output file.
 *
 * @author Ryan Chen
 * @date 2022
 */

#ifndef LOOKUP_H
#define LOOKUP_H

#include <stdio.h>

/**
 * @brief Run the full IP lookup pipeline on one date's worth of data.
 *
 * Reads the binary routing table from @p routing_table, the binary
 * trace (destination addresses) from @p trace, the human-readable
 * next-hop table from @p nexthop_src, and writes resolved next-hop
 * addresses to @p out.
 *
 * Timing statistics for each phase (memory allocation, table loading,
 * trie construction, and lookup) are printed to stdout.
 *
 * @param routing_table  Open readable FILE* — binary prefix + length, one per line.
 * @param trace          Open readable FILE* — 32-char binary destination addresses.
 * @param nexthop_src    Open readable FILE* — human-readable next-hop addresses.
 * @param out            Open writable FILE* — resolved next-hop output.
 * @return               0 on success, -1 on allocation failure.
 */
int run_iplookup(FILE *routing_table, FILE *trace, FILE *nexthop_src, FILE *out);

#endif /* LOOKUP_H */
