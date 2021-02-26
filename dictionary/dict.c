#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "dict.h"

void l(char *word)
{
    trie_insert(s_strdup(word));
}

void load_some_sample_data()
{
    l("abacot");
    l("abacterial");
    l("abactinal");
    l("abactinally");
    l("abaction");
    l("abactor");
    l("abaculi");
    l("abaculus");
    l("abacus");
    l("abacuses");

    l("grinagog");
    l("grinch");
    l("grincome");
    l("grind");
    l("grindable");
    l("grindal");
    l("grinded");
    l("grindelia");
    l("grindelwald");
    l("grinder");
    l("grindery");
}

int main(int argc, char *argv[])
{
    // better to use getopt or similar

    if (argc == 2 && !strcmp(argv[1], "test"))
    {
        allocator_init_test();
        trie_test();
        return 0;
    }

    if (argc == 2 && !strcmp(argv[1], "load"))
    {
        allocator_init_server();
        tree_init_server();
        load_some_sample_data();
        // traverse_and_dump_from_root();

        printf("Server started.\n");
        while (1)
        {
            // traverse_and_dump_from_root();
            sleep(60);
        }
        return 0;
    }

    if (argc == 3)
    {
        allocator_init_client();
        tree_init_client();
        char *word = argv[2];
        if (!strcmp(argv[1], "search"))
        {
            struct trie_node_t *n = trie_lookup(word);
            if (n)
            {
                printf("Word found [%s]\n", n->value);
                return 0;
            }
            else
            {
                printf("Word [%s] NOT found\n", word);
                return 1;
            }
        }
        if (!strcmp(argv[1], "insert"))
        {
            struct trie_node_t *n = trie_insert(s_strdup(word));
            if (n)
            {
                printf("Word added [%s]\n", n->value);
                return 0;
            }
            else
            {
                printf("Word [%s] already there.\n", word);
                return 1;
            }
        }
        if (!strcmp(argv[1], "delete"))
        {
            trie_delete(word);
            printf("Word deleted.\n");
            return 0;
        }
    }

    printf("Usage: dict load \nThen   dict [insert|lookup|delete] word.\nOr     dict test.\n");
    return 3;
}
