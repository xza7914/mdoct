
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.20
 */

#ifndef __STATE_TREE_H__
#define __STATE_TREE_H__

struct item
{
    int msg_id;
    struct tree_node *child;
};

struct tree_node
{
    int len;
    struct item *children;
};

struct tree_node* build_tree();
struct tree_node *push_forward(struct tree_node *node, int msg_id);

#endif