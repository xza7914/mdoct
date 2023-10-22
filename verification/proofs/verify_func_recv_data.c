
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.20
 */

#include "proof_helper.h"
#include "main_header.h"
#include "queue.h"

extern struct MSG_NODE *list_head = NULL;
extern struct MSG_NODE *list_end = NULL;

void producer_thread()
{
    for (;;)
    {
        struct MSG_STRUCT *msg = malloc(sizeof(struct MSG_STRUCT));
        __CPROVER_assume(msg != NULL);
        // ...

        uint16_t len = nondet_uint16();
        
        struct tree_node *root = malloc(sizeof(struct tree_node));
        __CPROVER_assume(root != NULL);

        root = push_forward(root, msg->mtype);
        que_push_back(root);

        AddListTill(*msg, len);
    }
}

void harness()
{
    producer_thread();
    __CPROVER_assume(list_head != NULL);
    func_msg_queue(list_head->t_msg,list_head->len);
     
}