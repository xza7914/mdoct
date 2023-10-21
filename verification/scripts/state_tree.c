
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.22
 */

#include "state_tree.h"
#include <stdlib>

struct tree_node *push_forward(struct tree_node *node, int msg_id) {
    for (int i = 0; i < node->len; ++i) {
        if (node->children[i].msg_id == msg_id) {
            return node->children[i].child;
        }
    }

    __CPROVER_assert(0, "unexpected msg_id");
    return NULL;
}