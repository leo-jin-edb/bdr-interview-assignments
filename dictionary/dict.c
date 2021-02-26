#include <stdio.h>
#include <string.h>
#include "dict.h"

void l(char *word)
{
    trie_insert(strdup(word));
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
        trie_test();
        return 0;
    }

    load_some_sample_data();

    if (argc == 3)
    {
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
            struct trie_node_t *n = trie_insert(word);
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
            printf("Word deleted.\n");
            return 0;
        }
    }

    printf("Usage: dict [insert|lookup|delete] word.\nOr dict test.\n");
    return 3;
}
