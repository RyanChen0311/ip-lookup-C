/**
 * @file ip_types.h
 * @brief Core data type definitions for IP Lookup Engine
 *
 * Defines the fundamental data structures used throughout the
 * binary trie-based IP lookup implementation.
 *
 * @author Ryan Chen
 * @date 2022
 */

#ifndef IP_TYPES_H
#define IP_TYPES_H

#include <stdint.h>

/** Maximum length of a 32-bit binary string representation */
#define IP_BINARY_LEN   32

/** Maximum number of routing table entries */
#define MAX_PREFIXES    1000000

/** Maximum number of next-hop entries */
#define MAX_NEXTHOPS    1000000

/** Length of a single octet in binary string form */
#define OCTET_BITS      8

/**
 * @brief A 32-bit IP address stored in binary string form with prefix length.
 *
 * ip[] holds the binary representation of the address (e.g. "11000000101010000000000100000001")
 * without a null terminator — all 32 chars are significant.
 */
typedef struct {
    char ip[IP_BINARY_LEN]; /**< Binary string representation (no null terminator) */
    int  len;               /**< Prefix length in bits (0–32) */
} BinIPWithLen;

/**
 * @brief A single node in the binary trie.
 *
 * data == 0 means this is an internal (non-leaf) node.
 * data > 0  means this node stores a routing table index.
 */
typedef struct TrieNode {
    int              data;  /**< Routing table index; 0 if not a prefix endpoint */
    struct TrieNode *left;  /**< Child for bit value 0 */
    struct TrieNode *right; /**< Child for bit value 1 */
} TrieNode;

/**
 * @brief Metadata for a single routing prefix entry.
 */
typedef struct {
    int  index;                    /**< Row index in the next-hop table */
    int  len;                      /**< Prefix length in bits */
    char bstring[IP_BINARY_LEN];   /**< Binary string of the prefix */
} PrefixInfo;

#endif /* IP_TYPES_H */
