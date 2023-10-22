
#ifndef __MAIN_HEADER_H__
#define __MAIN_HEADER_H__

#include <stdlib.h>

#define BUF_SIZE 64
#define LINK_MAX_LENGTH 2000
#define TRY_SEND_TIME 3

struct MSG_STRUCT
{
	long int mtype;
	char buff[BUF_SIZE];
};

struct MSG_NODE
{
	struct MSG_STRUCT t_msg; // 数据域
	unsigned short len;
	struct MSG_NODE *next; // 指针域
};

void AddListTill(struct MSG_STRUCT t_msg, unsigned short len);
void DeleteListHead();
int LinkLength(struct MSG_NODE *t_link);
void func_msg_queue(struct MSG_STRUCT t_msg_struct, unsigned short len);
void *func_recv_data(void *arg);

#endif