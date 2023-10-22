
/**
 * @author xieziang <xza7914@163.com>
 * @date 2023.10.22
 */

#ifndef __STUBS_H__
#define __STUBS_H__

#include <stdlib.h>
#include "main_header.h"

void* malloc(size_t size);
int (*TransferEncryptM_NEW)(unsigned char *pDataBuf, int *pDataLen, pMESSAGE_HEAD pMessageHead, APP_HEAD *app);

#endif