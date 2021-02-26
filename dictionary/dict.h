struct trie_node_t *trie_insert(char *value);
struct trie_node_t *trie_lookup(char *value);
void trie_delete(char *value);
void trie_test();

#define s_calloc(a) calloc(a, 1)
#define s_free(a) free(a)

#define ALPHABET_SIZE 26

struct trie_node_t
{
    struct trie_node_t *children[ALPHABET_SIZE];
    char *value;
};