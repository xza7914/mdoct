
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.22
 */

#include "func_signals.h"
#include "stdlib.h"

static struct signal_list *signal_lists = {NULL};

struct signal_list get_signal_list_by_id(uint32_t id)
{
    return signal_lists[id];
}