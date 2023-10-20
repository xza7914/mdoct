/**
 * @author zhangjianyu <220155249@qq.com>
 * @date 2023.10.20
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "state_tree.h"
#include "proof_helper.h"
#include "main_header.h"

#define QUEUE_SIZE LINK_MAX_LENGTH
#define ARRAY_SIZE (QUEUE_SIZE + 1)

bool que_empty();
bool que_full();
void que_push_back(struct node *a);
void que_pop_front();
struct node *que_get_front();

#endif