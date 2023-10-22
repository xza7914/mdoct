
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.22
 */

#ifndef __FUNC_SIGNALS_H__
#define __FUNC_SIGNALS_H__

#include <stddef.h>

struct signal
{
    uint32_t singal_id;
    void *return_value;

    uint32_t expected_func_id;
    bool (*check_func)(void **argv);
};

struct signal_list
{
    size_t len;
    struct signal* signals;
};

struct signal_list get_signal_list_by_id(uint32_t id);

#endif