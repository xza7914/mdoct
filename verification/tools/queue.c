
/**
 * @author zhangjianyu <220155249@qq.com>
 * @date 2023.10.20
 */

#include "queue.h"

static struct tree_node *queue[ARRAY_SIZE];
static uint32_t left, right;

bool que_empty()
{
    return left == right;
}

bool que_full()
{
    return (left + ARRAY_SIZE - right) % ARRAY_SIZE == 1;
}

void que_push_back(struct tree_node *a)
{
    if (full())
        return;
    queue[right] = a;
    right = (right + 1) % (ARRAY_SIZE);
}

void que_pop_front()
{
    if (empty())
        return;
    left = (left + 1) % (ARRAY_SIZE);
}

struct tree_node *que_get_front()
{
    return queue[left];
}

void que_set_front(struct tree_node *node) {
    queue[left] = node;
}
