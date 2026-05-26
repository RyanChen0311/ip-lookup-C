/**
 * @file trie.c
 * @brief Binary trie implementation for Longest Prefix Match (LPM)
 *
 * Implements a 32-level binary trie that mirrors the bit structure of an
 * IPv4 address.  Each left child corresponds to bit 0, each right child
 * to bit 1.  A non-zero data field at a node marks it as the terminal
 * for a prefix of length equal to that node's depth.
 *
 * The search algorithm performs LPM by tracking the most-recently-seen
 * non-zero index as it descends, returning that index when it runs out
 * of trie nodes — identical to what real router ASICs do in hardware.
 *
 * @author Ryan Chen
 * @date 2022
 */

#include <stdlib.h>
#include <stdio.h>
#include "trie.h"
#include "ip_types.h"

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/**
 * @brief Allocate and zero-initialise a new trie node.
 * @return Pointer to the new node, or NULL on allocation failure.
 */
static TrieNode *node_alloc(void)
{
    TrieNode *n = (TrieNode *)malloc(sizeof(TrieNode));
    if (n) {
        n->data  = 0;
        n->left  = NULL;
        n->right = NULL;
    }
    return n;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void trie_insert(TrieNode **tree, PrefixInfo val, int level)
{
    /* Reached the prefix length — record the index here */
    if (level == val.len) {
        if (!(*tree)) {
            *tree = node_alloc();
            if (!(*tree)) {
                fprintf(stderr, "[trie] allocation failure at depth %d\n", level);
                return;
            }
        }
        /* Only write if the slot is empty (first-write-wins) */
        if ((*tree)->data == 0) {
            (*tree)->data = val.index;
        }
        return;
    }

    /* Allocate an internal node if needed */
    if (!(*tree)) {
        *tree = node_alloc();
        if (!(*tree)) {
            fprintf(stderr, "[trie] allocation failure at depth %d\n", level);
            return;
        }
    }

    /* Descend based on the current bit */
    if (val.bstring[level] == '1') {
        trie_insert(&(*tree)->right, val, level + 1);
    } else {
        trie_insert(&(*tree)->left,  val, level + 1);
    }
}

int trie_search(TrieNode **tree, const char *addr, int level)
{
    TrieNode *cur       = *tree;
    int       best_match = 0;
    int       depth     = level;

    while (cur && depth < IP_BINARY_LEN) {
        if (cur->data > 0) {
            best_match = cur->data;
        }
        cur = (addr[depth] == '1') ? cur->right : cur->left;
        depth++;
    }
    return best_match;
}

void trie_free(TrieNode *tree)
{
    if (!tree) return;
    trie_free(tree->left);
    trie_free(tree->right);
    free(tree);
}
