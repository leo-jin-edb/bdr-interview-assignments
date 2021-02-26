struct trie_node_t *trie_insert(char *value);
struct trie_node_t *trie_lookup(char *value);
void trie_delete(char *value);
void trie_test();

void *s_calloc(int size);
void *s_malloc(int size);
void s_free(void *ptr);
char *s_strdup(char *str);
void allocator_init();

#define ALPHABET_SIZE 26

struct trie_node_t
{
    struct trie_node_t *children[ALPHABET_SIZE];
    char *value;
};