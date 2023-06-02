#ifndef __TREE_TABLE_H__
#define __TREE_TABLE_H__

#include <stdbool.h>
#include "debug.h"
#include <stdlib.h>

typedef struct TreeTable TreeTable;

struct TreeTable {
    TreeTable * parent;
    //int size; //implicit 0 if children = NULL, otherwise fixed by application
    int parent_index;
    //pointer to object
    void * object;
    TreeTable ** children;
};

typedef struct CodeWord CodeWord;

struct CodeWord {
    int cw_value;
    int cw_bits;
};

bool treetable_is_tree(TreeTable * this);
bool treetable_is_terminal(TreeTable * this);
bool treetable_has_value(TreeTable * this);
bool treetable_is_root(TreeTable * this);


#endif //__TREE_TABLE_H__