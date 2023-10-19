
#ifndef __STATE_TREE_H__
#define __STATE_TREE_H__

struct item {
    int msg_id;
    struct node *child;
};

struct node {
    struct item *children;
};

#endif