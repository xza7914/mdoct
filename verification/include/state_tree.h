
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.20
 */

#ifndef __STATE_TREE_H__
#define __STATE_TREE_H__

#include <stdbool.h>

struct item
{
    uint32_t signal_id;
    struct tree_node *child_node;
};

struct tree_node
{
    uint32_t excepted_func_id;
    bool (*check_func)(void **argv);

    uint32_t children_num;
    struct item *children;
};

struct tree_node* build_tree();
struct tree_node *push_forward(struct tree_node *node, uint32_t signal_id);

#endif