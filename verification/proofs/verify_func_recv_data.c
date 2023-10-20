
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.20
 */

#include "proof_helper.h"
#include "main_header.h"

extern struct MSG_NODE *list_head = NULL;
extern struct MSG_NODE *list_end = NULL;

void producer_thread()
{
    for (;;)
    {
        struct MSG_STRUCT *msg = malloc(sizeof(struct MSG_STRUCT));
        // ...
        __CPROVER_assume(msg != NULL);

        uint16_t len = nondet_uint16();

        AddListTill(*msg, len);
    }
}

void harness()
{
}