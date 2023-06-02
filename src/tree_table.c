#include "tree_table.h"

bool treetable_is_tree(TreeTable * this) {
    return this->children != NULL;
}

bool treetable_is_terminal(TreeTable * this) {
    return this->children == NULL;
}

bool treetable_has_value(TreeTable * this) {
	return this->object != NULL;
}

bool treetable_is_root(TreeTable * this) {
	return this->parent == NULL;
}
