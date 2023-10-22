
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.22
 */

#include "state_tree.h"
#include <stdlib.h>

struct tree_node *push_forward(struct tree_node *node, uint32_t signal_id)
{
    for (int i = 0; i < node->children_num; ++i)
    {
        if (node->children[i].signal_id == signal_id)
        {
            return node->children[i].child_node;
        }
    }

    __CPROVER_assert(0, "unexpected signal_id");
    return NULL;
}