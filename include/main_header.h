
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

#pragma pack(push, 1)
typedef struct // 42B
{
    unsigned char AppType;
    unsigned char Level;
    unsigned char Date[8];
    unsigned char Time[8]; // 时间
    unsigned char Manage[8];
    unsigned int TxtLen; // 数据长度(原文字节数)
    unsigned char *pSrcAddID;
    int DestNum;
    unsigned char *pDestAddress;
} MESSAGE_HEAD, *pMESSAGE_HEAD;

typedef struct
{
    int app1;
    int app2;
    int app3;
    int app4;
} APP_HEAD, *PAPP_HEAD;

// 状态
typedef struct j_status
{
    //// 可读写
    uint32_t red_state;   // 0: 未启动；1：启动成功
    uint32_t black_state; // 0: 未启动；1：启动成功
    uint8_t id[32];       //
    uint8_t date[64];     // 字符串形式，格式如"2022/04/12 10:00:00"
                          //// 以下为只读
    uint32_t core_state;
    uint32_t disk_num;
    uint8_t key1_date[64];
    uint8_t key2_date[64];
} __attribute__((packed)) j_status_t;

#pragma pack(pop)

void AddListTill(struct MSG_STRUCT t_msg, unsigned short len);
void DeleteListHead();
int LinkLength(struct MSG_NODE *t_link);
void func_msg_queue(struct MSG_STRUCT t_msg_struct, unsigned short len);
void *func_recv_data(void *arg);

#endif