
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.22
 */

#include "stubs.h"
#include "proof_helper.h"
#include "func_signals.h"
#include "queue.h"

#define FUNC_ID_OF_MALLOC 0

static void *route(uint32_t func_id)
{
    struct signal_list list = get_signal_list_by_id(func_id);

    uint32_t t = nondet_int32();
    __CPROVER_assume(t <= list.len);

    struct signal sig = list.signals[t];
    struct tree_node *node = que_get_front();
    que_set_front(push_forward(node, signal));

    return sig.return_value;
}

void *malloc(size_t size)
{
    return route(FUNC_ID_OF_MALLOC);
}
