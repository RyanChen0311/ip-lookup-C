/**
 * @file trie.h
 * @brief Binary trie interface for Longest Prefix Match (LPM)
 *
 * Provides insert, search, and cleanup operations for the binary trie
 * used in IP route lookup.  The trie represents the IPv4 routing table
 * as a 32-level binary tree where each level corresponds to one bit of
 * the destination address.
 *
 * @author Ryan Chen
 * @date 2022
 */

#ifndef TRIE_H
#define TRIE_H

#include "ip_types.h"

/**
 * @brief Insert a prefix into the binary trie.
 *
 * Walks the trie bit-by-bit according to @p val.bstring up to depth
 * @p val.len, allocating internal nodes as needed, then records
 * @p val.index at the terminal node.  If a node already exists at
 * that position the existing index is silently preserved (first-write
 * wins — matches standard longest-prefix-match semantics for
 * non-overlapping tables).
 *
 * @param tree  Pointer to the root pointer (may be updated on first insert).
 * @param val   Prefix to insert (binary string + length + index).
 * @param level Current recursion depth (call with 0).
 */
void trie_insert(TrieNode **tree, PrefixInfo val, int level);

/**
 * @brief Search the trie for the longest matching prefix of @p addr.
 *
 * Performs a longest-prefix-match by walking the trie bit-by-bit and
 * keeping track of the last matching index seen (the "best" match).
 *
 * @param tree   Pointer to the root pointer.
 * @param addr   32-character binary string of the destination address.
 * @param level  Current recursion depth (call with 0).
 * @return       Routing table index of the longest matching prefix,
 *               or 0 if no prefix matched (default route).
 */
int trie_search(TrieNode **tree, const char *addr, int level);

/**
 * @brief Recursively free all nodes of the trie.
 *
 * @param tree Root of the trie (or any subtree).
 */
void trie_free(TrieNode *tree);

#endif /* TRIE_H */
