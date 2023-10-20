
/**
 * @author zhangjianyu <220155249@qq.com>
 * @date 2023.10.20
 */

#include "queue.h"

struct node* queue[ARRAY_SIZE];
int left, right;

bool que_empty() {
    return left == right;
}

bool que_full() {
    return (left + ARRAY_SIZE - right) % ARRAY_SIZE == 1;
}

void que_push_back(struct node *a) {
    if (full()) return;
    queue[right] = a;
    right = (right + 1) % (ARRAY_SIZE);
}

void que_pop_front() {
    if (empty()) return;
    left = (left + 1) % (ARRAY_SIZE);
}

struct node *que_get_front() {
    return queue[left];
}
