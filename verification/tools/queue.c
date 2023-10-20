
/**
 * @author zhangjianyu <220155249@qq.com>
 * @date 2023.10.20
 */

#include "queue.h"

struct node queue[ARRAY_SIZE];
int left, right;

bool empty() {
    return left == right;
}

bool full() {
    return (left + ARRAY_SIZE - right) % ARRAY_SIZE == 1;
}

void push_back(struct node a) {
    if (full()) return;
    queue[right] = a;
    right = (right + 1) % (ARRAY_SIZE);
}

void pop_front() {
    if (empty()) return;
    left = (left + 1) % (ARRAY_SIZE);
}

int get_front_id() {
    return left;
}
