
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.20
 */

#ifndef __STATE_TREE_H__
#define __STATE_TREE_H__

struct item
{
    int msg_id;
    struct node *child;
};

struct node
{
    struct item *children;
};

struct node* build_tree();

#endif