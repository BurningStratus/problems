#include <cstdio>
#include <ctype.h>
#include <random>
#include <cmath>
#include <stack>

unsigned char random_uint8 (void) {
    // intended C++ way of generating pseudo-random numbers
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution <unsigned char> dist_char;
    return dist_char (rng);
}

struct node {
    node* left = nullptr;
    node* right = nullptr;
    node* parent = nullptr;
    int depth = 0;  // to track the level of a tree
    int balance = 0; // balance factor

    // causes SIGSEGV, but for a simple demo
    // we deliberately dont care about ownership and memory leaks. OS kills it.
    // ~node () {
    //    if (left != nullptr)    delete (left);
    //    if (right != nullptr)   delete (right);
    //};

    unsigned char data;

    node (unsigned char data)
        : data(data)
    {}
};

struct tree {
    node* root = nullptr;
    // ~tree () { if (root != nullptr) delete (root); };

    void insert (unsigned char data);
    bool search (unsigned char data);
    int height (node *n);
    bool is_balanced (node *root);
    void print (node* root, int space = 0);

    node* rotate_leftchild (node* x);
    node* rotate_rightchild (node* x);
    node* double_rotate_leftchild (node* x);
    node* double_rotate_rightchild (node* x);
};

node* tree::rotate_leftchild (node* x)
{   // rotate right, with left child, AVL case 1
    if (x == nullptr || x->left == nullptr) return x;
	node *l = x->left;
	x->left = l->right;
	l->right = x;
	return l;
}

node* tree::double_rotate_leftchild (node* x)
{   // perform double rotation right, AVL case 2
    if (x == nullptr || x->left == nullptr) return x;
    x->left = rotate_rightchild (x->left);
    return rotate_leftchild (x);
}

node* tree::rotate_rightchild (node* x)
{   // rotate left, with right child, AVL case 4
    if (x == nullptr || x->right == nullptr) return x;
    node* l = x->right;
    x->right = l->left;
    l->left = x;
    return l;
}

node* tree::double_rotate_rightchild (node* x)
{   // perform double rotation right, AVL case 2
    if (x == nullptr || x->right == nullptr) return x;
    x->right = rotate_leftchild (x->right);
    return rotate_rightchild (x);
}

// with 5 nodes?
// with 10 nodes?
void tree::insert (unsigned char data)
{
    if (root == nullptr) {
        root = new node (data);
        return;
    }
    std::stack <node *> path {};

    node* current = root;
    node* parent = nullptr;

    // traverse the tree
    int depth = 0;
    path.push (current); // push the root
    while (current)
    {
        parent = current;
        if (current->data == data) return; // no duplicates

        if (data < current->data) {
            if (current->left == nullptr) {
                current->left = new node(data);
                current->depth = depth;
                break;
            }
            current = current->left;
        } else {
            if (current->right == nullptr) {
                current->right = new node(data);
                current->depth = depth;
                break;
            }
            current = current->right;
        }

        depth ++;
        path.push (current); // keep track of path taken
    }

    // balance
    if (is_balanced (root)) return; // zaebis' !
    
    // int diff = height(root->right) - height (root->left); // either -x or +x
    /// std::printf ("unbalanced: %s\n", (diff < 0) ? "left" : "right" );
    std::printf ("count: %d\n", path.size());

    // measure the height of each subtree
    // rotate only the first unbalanced
    // check only the path taken

    node *to_balance = nullptr;
    while (!path.empty())
    {
        // measure the height
        to_balance = path.top ();
        path.pop ();

        // recalculate the balance factor
        to_balance->balance = height(to_balance->left) - height(to_balance->right);

        if (!is_balanced (to_balance)) break; // found the unbalanced node
    }
    std::printf ("first unbalanced node is: %d\n", to_balance->data);
    if (!path.empty()) {
        parent = path.top (); // node will need to be reconnected
    } else {
        // to_balance is the root. handle this case differently

        if (to_balance->balance >= 2) { // left-heavy
            // rotate right, adjust root
            root = rotate_leftchild (to_balance);
            std::printf ("to_balance's parent is root, left\n");
        } else if (to_balance->balance <= -2) { // right-heavy
            // rotate left, adjust root
            root = rotate_rightchild (to_balance);
            std::printf ("to_balance's parent is root, right\n");
        }
        return;
    }
    
    // 4 cases: ll, rl, lr, rr.
    if (to_balance->balance >= 2 && data > to_balance->data)
    {   // left-heavy, R
        parent->left = double_rotate_rightchild (to_balance);
        std::printf ("rotate: LR %d\n", to_balance->balance);
    }
    else if (to_balance->balance >= 2 && data < to_balance->data)
    {   // left-heavy, L
        parent->left = rotate_leftchild (to_balance);
        std::printf ("rotate: LL %d\n", to_balance->balance);
    }
    else if (to_balance->balance <= -2 && data > to_balance->data)
    {   // right-heavy, R
        parent->right = rotate_rightchild (to_balance);
        std::printf ("rotate: RR %d\n", to_balance->balance);
    }
    else if (to_balance->balance <= -2 && data < to_balance->data)
    {   // right-heavy, L
        parent->right = double_rotate_leftchild (to_balance);
        std::printf ("rotate: RL %d\n", to_balance->balance);
    }
    else {
        std::printf ("nothing to balance: %d\n", to_balance->balance);
    }

    
#if 0
    case 0: // LL
        current = rotate_leftchild(current); // right rotation
        break;
    case 1: // RL
        current->right = rotate_leftchild(current->right);  // right rotation
        current = rotate_rightchild(current);               // left rotation
        break;
    case 2: // LR
        current->left = rotate_rightchild(current->left);   // left rotation
        current = rotate_leftchild(current);                // right rotation
        break;
    case 3: // RR
        current = rotate_rightchild(current); // left rotation
        break;
    }
#endif
}

bool tree::search (unsigned char data) {
    node* current = root;
    node* parent = nullptr;

    // traverse the tree
    int depth = 0;
    while (current)
    {
        parent = current;
        depth ++;

        if (data == current->data)
            return true;

        if (data < current->data) {
            if (current->left == nullptr) return false;

            current = current->left;
        } else {
            if (current->right == nullptr) return false;
            current = current->right;
        }

        if (current->left == nullptr)
            return false;
    }
    return false;
}

int tree::height (node* n)
{
    if (n == nullptr) return 0;
    // height = 1 + max (left, right);
    return (1 + std::max (height (n->left), height (n->right)));
}

bool tree::is_balanced (node* root)
{
    if (root == nullptr)
        return true;

    // get the height of left and right subtrees
    int h_left = height(root->left);
    int h_right = height(root->right);
	
    if (abs(h_left - h_right ) > 1)
        return false;

    // recursively check the left and right subtrees
    return is_balanced (root->left) && is_balanced (root->right);
}

void tree::print (node* root, int space)
{
    if (root == nullptr) return;

    space += 5;

    print (root->right, space);
    std::printf ("\n");

    for (int i = 5; i < space; i++)
        std::printf (" ");

    std::printf ("* %d\n", root->data);

    print (root->left, space);
}

int main ()
{
    tree t {};

    for (unsigned char a = 0; a < 5; ++a ) {
        t.insert (random_uint8 ());
    }

    // keep one value, so we could test the 'search' operation
    // add it after random inserts
    unsigned char store_one = random_uint8();
    t.insert (store_one);

    std::printf ("search: \n");
    t.print (t.root);
    std::printf ("balanced?: %d\n", t.is_balanced (t.root));

    t.search (store_one);
    t.search ('t');
    t.search (35);


    std::printf ("========================================================== \n");
    tree t_balanced {};
    t_balanced.insert (10);
    t_balanced.insert (80);
    t_balanced.insert (110);
    t_balanced.insert (70);

    t_balanced.print (t_balanced.root);

    std::printf ("balanced?: %d\n", t_balanced.is_balanced (t_balanced.root));

    return 0;
}

