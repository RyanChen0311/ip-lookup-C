/**
 * @file main.c
 * @brief IP Lookup Engine — entry point
 *
 * Orchestrates the full pipeline for a given BGP route collector
 * (RRC region):
 *
 *   1. Convert CIDR prefix list  → binary form
 *   2. Convert IP trace          → binary form
 *   3. Build binary trie + run Longest Prefix Match on every destination
 *   4. Write resolved next-hop addresses to output files
 *
 * Usage:
 *   @code
 *   ./iplookup <region> <date_tag>
 *   @endcode
 *
 *   region   : rrc00 | rrc01 | rrc03 | rrc04 | rrc05 | Final
 *   date_tag : e.g. 20211122
 *
 * Example:
 *   @code
 *   ./iplookup rrc04 20211122
 *   @endcode
 *
 * @author Ryan Chen
 * @date 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "lookup.h"

/* --------------------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------------------- */

#define MAX_PATH 256

/** Supported RRC region names */
static const char *VALID_REGIONS[] = {
    "rrc00", "rrc01", "rrc03", "rrc04", "rrc05", "Final", NULL
};

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/**
 * @brief Validate that @p region is one of the supported region names.
 * @return 1 if valid, 0 otherwise.
 */
static int is_valid_region(const char *region)
{
    for (int i = 0; VALID_REGIONS[i] != NULL; i++) {
        if (strcmp(region, VALID_REGIONS[i]) == 0) return 1;
    }
    return 0;
}

/**
 * @brief Open a file and abort with a message if it fails.
 */
static FILE *xfopen(const char *path, const char *mode)
{
    FILE *f = fopen(path, mode);
    if (!f) {
        fprintf(stderr, "[error] cannot open file: %s\n", path);
        exit(EXIT_FAILURE);
    }
    return f;
}

/**
 * @brief Run the pipeline for a single (region, date_tag) pair.
 */
static void run_pipeline(const char *region, const char *date_tag)
{
    char prefix_in[MAX_PATH], trace_in[MAX_PATH];
    char prefix_bin[MAX_PATH], trace_bin[MAX_PATH];
    char nexthop_out[MAX_PATH];

    /* Build file paths */
    snprintf(prefix_in,   sizeof(prefix_in),   "./%s/prefix_%s.txt",  region, date_tag);
    snprintf(trace_in,    sizeof(trace_in),     "./%s/trace_%s.txt",   region, date_tag);
    snprintf(prefix_bin,  sizeof(prefix_bin),   "./data/prefix_bin_%s_%s.txt", region, date_tag);
    snprintf(trace_bin,   sizeof(trace_bin),    "./data/trace_bin_%s_%s.txt",  region, date_tag);
    snprintf(nexthop_out, sizeof(nexthop_out),  "./data/nexthop_%s_%s.txt",    region, date_tag);

    printf("\n=== Pipeline: %s / %s ===\n", region, date_tag);

    /* --- Step 1: Convert prefix file to binary form --- */
    {
        FILE *in  = xfopen(prefix_in,  "r");
        FILE *out = xfopen(prefix_bin, "w");
        int n = convert_prefixes(in, out);
        fclose(in); fclose(out);
        printf("[convert] %d prefixes → %s\n", n, prefix_bin);
    }

    /* --- Step 2: Convert trace file to binary form --- */
    {
        FILE *in  = xfopen(trace_in,  "r");
        FILE *out = xfopen(trace_bin, "w");
        int n = convert_trace(in, out);
        fclose(in); fclose(out);
        printf("[convert] %d trace entries → %s\n", n, trace_bin);
    }

    /* --- Step 3 & 4: Build trie and run LPM lookup --- */
    {
        FILE *rt  = xfopen(prefix_bin,  "r");
        FILE *tr  = xfopen(trace_bin,   "r");
        FILE *nh  = xfopen(prefix_in,   "r");  /* original file as nexthop source */
        FILE *out = xfopen(nexthop_out, "w");

        int rc = run_iplookup(rt, tr, nh, out);
        fclose(rt); fclose(tr); fclose(nh); fclose(out);

        if (rc != 0) {
            fprintf(stderr, "[error] lookup failed for %s/%s\n", region, date_tag);
            exit(EXIT_FAILURE);
        }
        printf("[lookup] results → %s\n", nexthop_out);
    }
}

/* --------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s <region> <date_tag>\n"
                "  region   : rrc00 | rrc01 | rrc03 | rrc04 | rrc05 | Final\n"
                "  date_tag : e.g. 20211122\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    const char *region   = argv[1];
    const char *date_tag = argv[2];

    if (!is_valid_region(region)) {
        fprintf(stderr, "[error] unknown region '%s'\n", region);
        fprintf(stderr, "        valid regions: rrc00 rrc01 rrc03 rrc04 rrc05 Final\n");
        return EXIT_FAILURE;
    }

    run_pipeline(region, date_tag);

    printf("\nDone.\n");
    return EXIT_SUCCESS;
}
