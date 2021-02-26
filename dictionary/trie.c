#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dict.h"

/* Simple trie management. Note that the functions take owenership of any strings passed.

   In this simple case, there is one global trie.
   */

void traverse_and_dump(struct trie_node_t *parent, int indent);

struct trie_node_t root;

int trie_slot(char c)
{
    int i = c - 'a';
    assert(i < ALPHABET_SIZE);
    assert(i >= 0);
    return i;
}

// for debugging only, really
char trie_unslot(int i)
{
    char c = i + 'a';
    return c;
}

struct trie_node_t *new_node(struct trie_node_t *parent, char position)
{
    struct trie_node_t *child = s_calloc(sizeof(struct trie_node_t));
    int slot = trie_slot(position);
    assert(parent->children[slot] == NULL);
    parent->children[slot] = child;
    return child;
}

struct trie_node_t *trie_insert(char *value)
{
    struct trie_node_t *node = &root;

    char *walk = value;
    while (*walk)
    {
        int slot = trie_slot(*walk);
        if (node->children[slot])
        {
            node = node->children[slot];
        }
        else
        {
            node = new_node(node, *walk);
        }
        walk++;
    }
    if (node->value)
    {
        /* alrady there */
        assert(!strcmp(node->value, value));
        return NULL;
    }
    node->value = value;
    return node;
}

struct trie_node_t *trie_lookup(char *value)
{
    struct trie_node_t *node = &root;

    char *walk = value;
    while (*walk)
    {
        int slot = trie_slot(*walk);
        if (node->children[slot])
        {
            node = node->children[slot];
        }
        else
        {
            return NULL;
        }
        walk++;
    }
    if (node->value && !strcmp(value, node->value))
        return node;
    else
        return NULL;
}

int is_leaf_node(struct trie_node_t *node)
{
    int i;
    for (i = 0; i < ALPHABET_SIZE; i++)
        if (node->children[i])
            return 0;
    return 1;
}

int _trie_delete(struct trie_node_t *node, char *value)
{
    if (*value == 0)
    {
        // we have found the node
        s_free(node->value);
        node->value = NULL;
    }
    else
    {
        int slot = trie_slot(*value);
        // go one level deeper (but also handle if the item does not exist)
        if ((node->children[slot]) && _trie_delete(node->children[slot], value + 1))
        {
            s_free(node->children[slot]);
            node->children[slot] = NULL;
        }
    }

    return is_leaf_node(node) && (node->value == NULL);
}

void trie_delete(char *value)
{
    _trie_delete(&root, value);
}

void traverse_and_dump(struct trie_node_t *parent, int indent)
{
    assert(parent);
    int i;
    char ind[200] = "";
    for (i = 0; i <= indent; i++)
        strcat(ind, " ");

    printf("%sNode: %p[%s]\n", ind, parent, parent->value ? parent->value : "<>");
    for (i = 0; i < ALPHABET_SIZE; i++)
    {
        if (parent->children[i])
        {
            printf("%s%c:\n", ind, trie_unslot(i));
            traverse_and_dump(parent->children[i], indent + 1);
        }
    }
}

#define trie_insert_(a) trie_insert(s_strdup(a))
void trie_test()
{
    // traverse_and_dump(&root, 0);

    assert(trie_lookup("a") == NULL);
    assert(trie_insert_("abcdf") == trie_lookup("abcdf"));
    assert(trie_lookup("a") == NULL);
    assert(trie_insert_("bbcdf") == trie_lookup("bbcdf"));
    assert(trie_insert_("bc") == trie_lookup("bc"));
    assert(trie_insert_("bccdaaaaaaaaf") == trie_lookup("bccdaaaaaaaaf"));
    assert(trie_insert_("bccdaaabaaaaf") == trie_lookup("bccdaaabaaaaf"));
    trie_delete("bccdaaabaaaaf");
    assert(NULL == trie_lookup("bccdaaabaaaaf"));
    assert(NULL == trie_lookup("bccdaaabaxaaf"));
    trie_delete("bccdaaaaaaaaf");
    trie_delete("bccdaaaaxxaaf");
    trie_delete("bc");
    trie_delete("bbcdf");
    trie_delete("abcdf");
    assert(is_leaf_node(&root));
    assert(trie_insert_("bccdaaaaaaaaf") == trie_lookup("bccdaaaaaaaaf"));
    trie_delete("bccdaaaaaaaaf");
    assert(NULL == trie_lookup("bccdaaaaaaaaf"));
    assert(is_leaf_node(&root));

    printf("Tests passed.\n");
}
