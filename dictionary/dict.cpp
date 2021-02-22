#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <assert.h>

using std::string;
using std::pair;

#undef DICT_REHASH
enum CMD {INSERT, SEARCH, DELETE};

class Node {
    Node *left;
    Node *right;
    string word;
    int occurrences;
    int lDepth;
    int rDepth;
public:
    static const int MAX_BTREE_IMBALANCE = 5;

    Node (const string& w) : left(0), right(0), word(w), occurrences(1), lDepth(0), rDepth(0) {}
    ~Node();
    bool store (const string& w, int& depth, Node** head);
    void store (Node* node, int& depth, Node** head);
    Node* find (const string& w) const;
    bool remove();
    void rebalance (Node** head);
    Node* remove (Node* node);
    bool isDeleted() {return (occurrences == 0);}
    int maxDepth() {return std::max(lDepth, rDepth);}
    string& getWord() {return word;}
};

Node::~Node()
{
    Node* node = NULL;
    while ((node = this->remove(node)) != this) {
    	delete node;
    }
}

bool Node::store (const string& w, int& depth, Node** head=NULL)
{
    bool stored;

    ++depth;
    int result = w.compare(word);
    if (result == 0) {
	++occurrences;
	return (occurrences == 1);
    }

    if (result < 0) {
	if (left)
	    stored = left->store(w, depth);
	else
	    left = new Node(w);
    }
    else if (result > 0) {
	if (right)
	    stored = right->store(w, depth);
	else
	    right = new Node(w);
    }

    if (head) {
	if (result < 0 && depth > lDepth)
	    lDepth = depth;
	else if (result > 0 && depth > rDepth)
	    rDepth = depth;
	if (std::abs(lDepth - rDepth) > MAX_BTREE_IMBALANCE)
	    rebalance (head);
    }

    return stored;
}

void Node::store (Node* n, int& depth, Node** head=NULL)
{
    ++depth;
    int result = n->word.compare(word);

    if (result < 0) {
	if (left)
	    left->store(n, depth);
	else
	    left = n;
    }
    else if (result > 0) {
	if (right)
	    right->store(n, depth);
	else
	    right = n;
    }

    if (head) {
	if (result < 0 && depth > lDepth)
	    lDepth = depth;
	else if (result > 0 && depth > rDepth)
	    rDepth = depth;
	if (std::abs(lDepth - rDepth) > MAX_BTREE_IMBALANCE)
	    rebalance (head);
    }
}

Node* Node::find (const string& w) const
{
    int result = w.compare(word);
    if (result == 0)
	return const_cast<Node*>(this);
    else if (result < 0 && left)
	return left->find(w);
    else if (result > 0 && right)
	return right->find(w);

    return NULL;
}

bool Node::remove()
{
    if (isDeleted())
	return false;

    --occurrences;
    return isDeleted();
}

Node* Node::remove (Node* node)	// argument only used for overload function resolution
{
    if (left) {
	node = left->remove(node);
	if (node == left)
	    left = NULL;
	return node;
    }

    if (right) {
	node = right->remove(node);
	if (node == right)
	    right = NULL;
	return node;
    }

    lDepth = rDepth = 0;
    return const_cast<Node*>(this);
}

/* Rotate the binary tree to create a more balanced tree */

void Node::rebalance (Node** head)
{
    if (lDepth > rDepth) {
	left->lDepth = lDepth - 1;
	left->rDepth = rDepth + 1;
	lDepth = rDepth = 0;
	*head = left;
	left = left->right;
        (*head)->right = const_cast<Node*>(this);
    } else if (rDepth > lDepth) {	
	right->rDepth = rDepth - 1;
	right->lDepth = lDepth + 1;
	lDepth = rDepth = 0;
	*head = right;
	right = right->left;
        (*head)->left = const_cast<Node*>(this);
    }
}

struct DictOverflow {
    DictOverflow() {}
};

class Dict {
    static const int MAX_WORDS = 1000000;
    static const int MAX_BTREE_DEPTH = 10;
    static const int MIN_SLOTS = 1001;
    static const int MAX_SLOTS = 65537;
#ifdef DICT_REHASH
    static const int DEFAULT_SLOTS = MIN_SLOTS;
    static int primes[] = {MIN_SLOTS,4007,16033,MAX_SLOTS}; // quadruple slots on rehash()

    std::shared_mutex mutex;
#else
    static const int DEFAULT_SLOTS = MAX_SLOTS;
#endif
    std::atomic<int> totalWords;
    std::vector<pair<std::shared_mutex, Node*>> *slots;

    int hash (const string& word, int capacity) {
	int h = MAX_SLOTS;
	for (int i = 0; i < word.length(); ++i)
	    h += word[i] * (1 << (i % 32));
	return ((h >= 0) ? h : -h) % capacity;
    }

#ifdef DICT_REHASH
    void rehash (int capacity);
#endif

public:
    Dict(int n);
    ~Dict();
    void insert (const string& w);
    bool search (const string& w);
    void delete_ (const string& w);
};

Dict::Dict(int n=DEFAULT_SLOTS) : totalWords(0)
{
    slots = new std::vector<pair<std::shared_mutex, Node*>>(n);
}

Dict::~Dict() {delete slots;}

void Dict::insert (const string& w)
{
    Node **btree;
    int depth;

    if (totalWords >= MAX_WORDS)
	throw DictOverflow();

#ifdef DICT_REHASH
    std::shared_lock dictLock(mutex);
#endif
    int h = hash(w, slots->capacity());
    std::unique_lock slotLock((*slots)[h].first);

    btree = &(*slots)[h].second;
    if (*btree) {
	depth = -1;
	if ((*btree)->store(w, depth, btree)) {
	    ++totalWords;
#ifdef DICT_REHASH
	    if (slots->capacity() < MAX_SLOTS &&
		(*btree)->maxDepth() > MAX_BTREE_DEPTH) {
		int capacity = slots->capacity();
	    	slotLock.unlock();
	    	dictLock.unlock();
	    	rehash(capacity);
	    }
#endif
	}
    }
    else {
	*btree = new Node(w);
	++totalWords;
    }
}

bool Dict::search (const string& w)
{
    Node *btree, *node;
#ifdef DICT_REHASH
    std::shared_lock dictLock(mutex);
#endif
    int h = hash(w, slots->capacity());
    std::shared_lock slotLock((*slots)[h].first);

    if ((btree = (*slots)[h].second))
	if ((node = btree->find(w)))
	    return !node->isDeleted();
    return false;
}

void Dict::delete_ (const string& w)
{
    Node *btree, *node;
#ifdef DICT_REHASH
    std::shared_lock dictLock(mutex);
#endif
    int h = hash(w, slots->capacity());
    std::unique_lock slotLock((*slots)[h].first);

    if ((btree = (*slots)[h].second)) {
	if ((node = btree->find(w))) {
	    if (node->remove())
	    	--totalWords;
	}
    }
}

#ifdef DICT_REHASH
void Dict::rehash (int capacity)
{
    int prime, h, depth;
    Node** new_head;
    std::vector<pair<std::shared_mutex, Node*>> *new_slots;

    std::unique_lock dictLock(mutex);
    if (capacity != slots->capacity() || slots->capacity() == MAX_SLOTS)
	return;
    for (int i = 1; i < primes.size(); ++i) {
	if ((prime = primes[i]) > slots->capacity())
	    break;
	}
    
    new_slots = new std::vector<pair<std::shared_mutex, Node*>>(prime);
    for (int j = 0; j < slots->size(); ++j)
	if ((Node* btree = (*slots)[j].second)) {
	    while ((Node* node = btree->remove(btree))) {
	    	h = hash(node->getWord(), new_slots->capacity());
		new_head = &(*new_slots)[h].second;
		if (node->isDeleted())
		    delete node;
		else if ((Node* new_btree = *new_head)) {
		    depth = 0;
		    new_btree->store(node, depth, new_head);
		}
		else
		    *new_head = node;

		if (node == btree)
		    break;
	    }
	    
	    (*slots)[j].second = NULL;
	}

    delete slots;
    slots = new_slots;
    }
}
#endif
   
struct CmdPckt {
    Dict* dict;
    CMD	cmd;
    string word;
    CmdPckt(Dict* d, CMD c, string& w) : dict(d), cmd(c), word(w) {};
};

void dictionary (CmdPckt* pkct)
{
    bool found;
    Dict* dict = pkct->dict;
    string& word = pkct->word;
    switch (pkct->cmd) {
	case INSERT:
	    try {
	        dict->insert(word);
	    } catch (DictOverflow o) {
		std::cerr << "dictionary insert overflow";
	    }
	    break;
	case SEARCH:
	    found = dict->search(word);
	    break;
	case DELETE:
	    dict->delete_(word);
	    break;
	default:
	    assert(false);
	    break;
    }
    delete pkct;
}

int main (int argc, char* argv[])
{
    string command, word;
    CMD	cmd;
    Dict* words = new Dict;

    if (argc > 1) {
	for (int i = 1; i + 1 < argc; i += 2) {
	    command = argv[i];
	    word = argv[i+1];
	    if (command.compare("insert") == 0)
	    	cmd = INSERT;
	    else if (command.compare("search") == 0)
	    	cmd = SEARCH;
	    else if (command.compare("delete") == 0)
	    	cmd = DELETE;
	    else {
	    	std::cerr << "bad dictionary command";
	    	continue;
	    }

	    CmdPckt *packet = new CmdPckt(words, cmd, word);
	    std::thread(dictionary, packet);
	}
    }
    else {
	while (std::cin >> command && std::cin >> word) {
	    if (command.compare("insert") == 0)
	    	cmd = INSERT;
	    else if (command.compare("search") == 0)
	    	cmd = SEARCH;
	    else if (command.compare("delete") == 0)
	    	cmd = DELETE;
	    else {
	    	std::cerr << "bad dictionary command";
	    	continue;
	    }

	    CmdPckt *packet = new CmdPckt(words, cmd, word);
	    std::thread(dictionary, packet);
	}
    }
}
