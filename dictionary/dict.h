struct trie_node_t *trie_insert(char *value);
struct trie_node_t *trie_lookup(char *value);
void trie_delete(char *value);
void trie_test();

void *s_calloc(int size);
void *s_malloc(int size);
void s_free(void *ptr);
char *s_strdup(char *str);
void allocator_init_server();
void allocator_init_client();
void allocator_init_test();
void traverse_and_dump_from_root();

void **trie_root_ptr();

void tree_init_server();
void tree_init_client();
void load_some_sample_data();
void trie_lock();
void trie_unlock();

#define ALPHABET_SIZE 26

struct trie_node_t
{
    struct trie_node_t *children[ALPHABET_SIZE];
    char *value;
};