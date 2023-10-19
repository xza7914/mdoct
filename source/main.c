#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

//消息队列
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdarg.h> //va_start

#define LINK_MAX_LENGTH 2000
#define TRY_SEND_TIME        3

struct MSG_STRUCT{
	long int mtype;
	char buff[8192];
};

struct MSG_NODE{
	struct MSG_STRUCT t_msg;//数据域
    unsigned short len;
	struct MSG_NODE* next;//指针域
};

//定义链表头尾指针
struct MSG_NODE* list_head = NULL;
struct MSG_NODE* list_end = NULL;

#pragma  pack(push, 1)
typedef struct        // 42B
{
    unsigned char AppType;        
    unsigned char Level;            
    unsigned char Date[8];        
    unsigned char Time[8];        // 时间     
    unsigned char Manage[8];        
    unsigned int TxtLen;            // 数据长度(原文字节数)
    unsigned char *pSrcAddID;        
    int DestNum;        
    unsigned char *pDestAddress;  
} MESSAGE_HEAD, *pMESSAGE_HEAD;

typedef struct {
    int app1;
    int app2;
    int app3;
    int app4;
} APP_HEAD, *PAPP_HEAD;

//状态
typedef struct j_status{
    //// 可读写
    uint32_t red_state;      // 0: 未启动；1：启动成功
    uint32_t black_state;    // 0: 未启动；1：启动成功
    uint8_t  id[32];     // 
    uint8_t  date[64];   // 字符串形式，格式如"2022/04/12 10:00:00"
    //// 以下为只读
	uint32_t core_state;     
    uint32_t disk_num;       
    uint8_t  key1_date[64]; 
    uint8_t  key2_date[64];     
}__attribute__((packed)) j_status_t;

#pragma   pack(pop)


/***************************************************************
            Dynamic linked library functions
***************************************************************/
typedef int (*HisNTUDeviceExist)();     // 返回true(1) 表示设备存在
typedef int (*HGetMmjId4Bytes)(unsigned char *mmjid);
typedef int (*HSetMmjId4Bytes)(unsigned char *mmjid);

typedef int (*HGetIntFromMmjIdString)(char *cstr, unsigned char *mmjid);    
typedef int (*HTransferEncryptM)(unsigned char *pDataBuf, int *pDataLen, pMESSAGE_HEAD pMessageHead, APP_HEAD *app);

typedef int (*HTransferDecryptM)(unsigned char *pDataBuf, int *pDataLen, APP_HEAD *app);

typedef int (*HTransferEncryptM_CC)(unsigned char *pDataBuf, int *pDataLen, pMESSAGE_HEAD pMessageHead);

typedef int (*HTransferDecryptM_CC)(unsigned char *pDataBuf, int *pDataLen);

typedef void (*CALL_BACK)(unsigned char *buffer, int length, APP_HEAD *app);

typedef int (*HInitClient)(unsigned short src_id);

typedef int (*HInitServer)(unsigned short src_id, CALL_BACK func);

typedef int (*HGetVersion)(char *version);

typedef int (*HSetInfo)(j_status_t *status);
typedef int (*HGetInfo)(j_status_t *status);

HisNTUDeviceExist isNTUDeviceExist;
HGetMmjId4Bytes GetMmjId4Bytes;
HSetMmjId4Bytes SetMmjId4Bytes;
HGetIntFromMmjIdString GetIntFromMmjIdString;
HTransferEncryptM TransferEncryptM_NEW;
HTransferDecryptM TransferDecryptM_NEW;
HTransferEncryptM_CC TransferEncryptM_CC;
HTransferDecryptM_CC TransferDecryptM_CC;

HInitClient InitClient;
HInitServer InitServer;
HGetVersion GetVerion;

HSetInfo SetInfo;
HGetInfo GetInfo;

//全局变量
static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_srcid = 0;
static unsigned long round_cnt = 1;
int isBusy = 0;
char strSrcId[32]={0};
unsigned char srcId[4] = {0x00, 0x00, 0x00, 0x00};//本机ID(十六进制)

static int msgid_recv = 0;
static int msgid_send = 0;
unsigned char dstId[4] = {0x00, 0x00, 0x00, 0x00};//对端ID(十六进制)
char str_mmjid[128]={0};
j_status_t t_mmj_st;
char file_name[128]="sdio";

void Printf(const char* format,...){
	struct tm *pTempTm;
	struct timeval time;
	gettimeofday(&time,NULL);
	pTempTm = localtime(&time.tv_sec);
    if (pTempTm == NULL)
    {
        printf("localtime failed!!!\r\n");
        return;
    }
    char time_str[128]={0};
    sprintf(time_str,"%04d-%02d-%02d %02d:%02d:%02d.%03ld",pTempTm->tm_year+1900,pTempTm->tm_mon+1,pTempTm->tm_mday,pTempTm->tm_hour,pTempTm->tm_min,pTempTm->tm_sec,time.tv_usec/1000);
    printf("[%s] [%s] ",time_str,file_name);
	va_list args;
	va_start(args,format);
	vprintf(format,args);
	va_end(args);
}

typedef struct _Param {
    APP_HEAD *app;
    int *args;
    int seq;
    int sub;
    int cost;
} Param;

//*******************************链表函数*******************************************
//向尾部添加节点
void AddListTill(struct MSG_STRUCT t_msg,unsigned short len){
	//创建节点
	struct MSG_NODE* temp = (struct MSG_NODE*)malloc(sizeof(struct MSG_NODE));//
    if (temp == NULL)
    {
        Printf("向尾部添加节点,申请动态内存失败！！！\r\n");
        return ;
    }
	//节点数据赋值
	temp->t_msg = t_msg;
    temp->len = len;
	temp->next = NULL;
	//判断是否存在链表
	if(list_head == NULL){
		list_head = temp;
	}else{
		list_end->next = temp;
	}
	list_end = temp;
    // free(temp);
}

//删除头部节点
void DeleteListHead(){
	//记录旧头
	struct MSG_NODE* temp = list_head;
	//链表检测
	if(list_head == NULL){
		Printf("链表为空\r\n");
		return;
	}
	list_head = list_head->next;
	free(temp);
}

//查询链表长度
int LinkLength(struct MSG_NODE *t_link){
	int k = 0;
	while (t_link != NULL)
	{
		k++;
		t_link = t_link->next;
	}
	return k;
}
//*********************************************************************************

//打印报文
void print(unsigned short len, unsigned char *data) 
{
    unsigned int i;
    //打印数据
    printf("报文数据[%d]: \n", len);
    for(i=0; i< len; i++) {
        if (i%16 == 0 && i != 0)
            printf("\n");
        printf("%02x  ", data[i]);
    }
    printf("\n");
}



int parse_data(unsigned char* buffer,int len)
{           
    print(len,buffer);
    unsigned char head_32[32] = {0};
    unsigned char head_16[16] = {0};
    unsigned char tail_16[16] = {0};
    int data_valid_len = 0;
    int data_len = len-32-16;

    unsigned char data[data_len];
    unsigned char data_new[data_len];
    bzero(data,data_len);
    bzero(data_new,data_len);
    unsigned char data_tmp[128] = {0};
    
    memcpy(head_32, buffer, 32);
    memcpy(&data_valid_len,head_32+8,sizeof(data_valid_len));
    memcpy(head_16, buffer+16, 16);
    memcpy(data, buffer+32, data_len);
    memcpy(tail_16,buffer+len-16,16);
    memcpy(head_32,head_16,16);
    memcpy(head_32+16,tail_16,16);
    

    int i = 0;
    int j = 0;

    for(i = 0; i< data_len;i=i+32)
    {
        memcpy(data_tmp, data+i, 32);
        for(j = 0; j< 32;j++)
        {
            data_tmp[j] = data_tmp[j]^head_32[j];
        }
        memcpy(data_new+i,data_tmp,32);
    }

    memcpy(buffer,data_new,data_valid_len);

    return data_valid_len;
}

int GetCurentTime(char *strDate, char *strTime) {
    time_t systime;
    struct tm *t;
    time(&systime);
    t = localtime(&systime);
    if (t == NULL)
    {
        Printf("getcurrenttime->localtime failed!!!\r\n");
        return -1;
    }
    
    sprintf(strDate, "%04d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    sprintf(strTime, "%02d%02d%02d00", t->tm_hour, t->tm_min, t->tm_sec);

    return 0;
}


int64_t getFileLength(const char *fileName) {
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL) {
        return -1;
    }

    fseek(pFile, 0, SEEK_END);
    int64_t length = ftell(pFile);
    fclose(pFile);

    return length;
}


int loadLibraryFunction() {
    void *hDynamicLib = NULL;
    char *error;

    // 加载DLL
    hDynamicLib = dlopen("./libdcsext.so", RTLD_LAZY);
    if (hDynamicLib == NULL) {
        if ((hDynamicLib = dlopen("libdcsext.so", RTLD_LAZY)) == NULL) {
            Printf("libdcsext.so load failed:%s\n", dlerror());
            return -1;
        }
    }

    
    isNTUDeviceExist = (HisNTUDeviceExist) dlsym(hDynamicLib, "isNTUDeviceExist");
    if ((error = dlerror()) != NULL) {
        Printf("Function isNTUDeviceExist() not found\n");
        return -1;
    }
	
    GetMmjId4Bytes = (HGetMmjId4Bytes) dlsym(hDynamicLib, "GetMmjId4Bytes");
    if ((error = dlerror()) != NULL) {
        Printf("Function GetMmjId4Bytes() not found\n");
        return -1;
    }

	
    SetMmjId4Bytes = (HGetMmjId4Bytes) dlsym(hDynamicLib, "SetMmjId4Bytes");
    if ((error = dlerror()) != NULL) {
        Printf("Function SetMmjId4Bytes() not found\n");
        return -1;
    }

	
    GetIntFromMmjIdString = (HGetIntFromMmjIdString) dlsym(hDynamicLib, "GetIntFromMmjIdString");
    if ((error = dlerror()) != NULL) {
        Printf("Function GetIntFromMmjIdString() not found\n");
        return -1;
    }

    TransferEncryptM_NEW = (HTransferEncryptM) dlsym(hDynamicLib, "TransferEncryptM");
    if ((error = dlerror()) != NULL) {
        Printf("Function TransferEncryptM() not found\n");
        return -1;
    }

    TransferDecryptM_NEW = (HTransferDecryptM) dlsym(hDynamicLib, "TransferDecryptM");
    if ((error = dlerror()) != NULL) {
        Printf("Function TransferDecryptM() not found\n");
        return -1;
    }


    TransferEncryptM_CC = (HTransferEncryptM_CC) dlsym(hDynamicLib, "TransferEncryptM_CC");
    if ((error = dlerror()) != NULL) {
        Printf("Function TransferEncryptM_CC() not found\n");
        return -1;
    }

    TransferDecryptM_CC = (HTransferDecryptM_CC) dlsym(hDynamicLib, "TransferDecryptM_CC");
    if ((error = dlerror()) != NULL) {
        Printf("Function TransferDecryptM_CC() not found\n");
        return -1;
    }

    InitServer = (HInitServer) dlsym(hDynamicLib, "InitServer");
    if ((error = dlerror()) != NULL) {
        Printf("Function InitServer() not found\n");
        return -1;
    }


    InitClient = (HInitClient) dlsym(hDynamicLib, "InitClient");
    if ((error = dlerror()) != NULL) {
        Printf("Function InitClient() not found\n");
        return -1;
    }

    GetVerion = (HGetVersion) dlsym(hDynamicLib, "GetDcsDllVersion");
    if ((error = dlerror()) != NULL) {
        Printf("Function GetDcsDllVersion() not found\n");
        return -1;
    }


    GetInfo = (HGetInfo) dlsym(hDynamicLib, "GetInfo");
    if ((error = dlerror()) != NULL) {
        Printf("Function GetInfo() not found\n");
        return -1;
    }

	SetInfo = (HSetInfo) dlsym(hDynamicLib, "SetInfo");
    if ((error = dlerror()) != NULL) {
        Printf("Function SetInfo() not found\n");
        return -1;
    }

    return 0;
}


//读取文件
//返回值 0：成功 
//      -1：打开文件失败 -2：申请临时内存失败
int readfile(char* filepath,char* buff,int *len){
	FILE *fp = NULL;
	fp = fopen(filepath,"rb+");
	if(fp == NULL){
		Printf("fopen failed,errno=%d\n",errno);
        return -1;
	}
	fseek(fp,0L,SEEK_END);
	int flen = ftell(fp);
	unsigned char *p = (unsigned char*)calloc(flen,sizeof(char));
	if(p==NULL){
		fclose(fp);
		return -2;
	}
	fseek(fp,0L,SEEK_SET);
	fread(p,flen,1,fp);

	memcpy(buff,p,flen);
	*len = flen;
	if(p != NULL){
		free(p);
	}
	fclose(fp);
    return 0;

}

//写数据到文件
//0：写成功   -1：打开文件失败  -2：写失败  -3：异常
int writefile(char* filepath,char* buff,int len){
	// if(access((const char*)filepath,F_OK) == -1){
	// 	Printf("access failed,errno=%d\n",errno);
	// 	return -1;
	// }
	int fd = open(filepath,O_RDWR | O_CREAT | O_TRUNC,0666);
	if(fd == -1){
		Printf("open file failed ,errno=%d\n",errno);
		return -1;
	}
	int size = write(fd,buff,len);
	if(size == -1){
		Printf("write file failed,errno=%d\n",errno);
        close(fd);
		return -2;
	}
	close(fd);
	return 0;
}

//写结构体到文件
int WriteStructToFile(j_status_t t_mmj,char* file){
   FILE *p=fopen(file,"wb");
    if(p==NULL){
        return -1;
    } 
    fwrite(&t_mmj,1,sizeof(struct j_status),p);
    fclose(p);
    return 0;
}


//ret_type: 0:   成功
void sendMsgRec(unsigned int msg_type, char ret_type,char* file_buff){
    struct MSG_STRUCT t_msg_struct;
    t_msg_struct.mtype = msg_type;
    unsigned short addr = 0;
    t_msg_struct.buff[0]=ret_type;
    addr+=1;
    strncpy(t_msg_struct.buff+addr,file_buff,strlen(file_buff));
    addr+=strlen(file_buff);
    msgid_send = msgget(1246,0666|IPC_CREAT);
    if(msgid_send == -1){
        Printf("msgget failed...\n");
    }
    int ret = msgsnd(msgid_send,(void*)&t_msg_struct,addr,0); 
    if(ret == -1){
        Printf("msgsnd failed...\n");
    }    
}


//读取ID
int readMMJId(char* t_file_path){
    char mmjid[128]={0};
    int mmjid_len = 0;
    if(!readfile(t_file_path,mmjid,&mmjid_len)){
        GetIntFromMmjIdString(mmjid,dstId);
        Printf(">>>>>>>>>>>>>>系统初始化,对端ID[%s][%x %x %x %x]",mmjid,dstId[0],dstId[1],dstId[2],dstId[3]);
        return 0;
    }else{
        return -1;
    }
}
//参数： filename 使用全路径名

int DYS_Test_CC(char * filename)
{
    MESSAGE_HEAD MessageHead;
//    unsigned char srcId[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned char desId[10];
    int ret = -1;
    unsigned char *dataBuf = NULL;
    uint64_t start1, end1= 0;
 

    memset(&MessageHead, 0, sizeof(MessageHead));
    // 设置报头各字段
    MessageHead.AppType = 0xFA; //CC
    MessageHead.Level = 0x01; 

    // 设置日期和时间
    // 日期为PC机系统日期，这个需要确保正常。时间精度无要求,填全零不影响功能。
    char strDate[64];
    char strTime[64];
    memset(strDate,0x00,sizeof(strDate));
    memset(strTime,0x00,sizeof(strTime));
    GetCurentTime(strDate, strTime);
    memcpy(MessageHead.Date, strDate, 8);
    memcpy(MessageHead.Time, strTime, 8);

    memset(MessageHead.Manage, 0, 8);

    MessageHead.pSrcAddID = srcId;

    MessageHead.DestNum = 0x1;
    memcpy(desId, srcId, 4);
    MessageHead.pDestAddress = srcId;

    int length = 0;
    char file_name[512]={0};
    length = (int) getFileLength(filename);
    //==========================================================================================
    // 将来对文件大小进行判断，大于多少 进行分包 加密后进行分包存储，分文件以enc.x（x 为 分包文件的序号）
    //==========================================================================================
    if (length <= 0) {
        Printf("\t this file is null.\n");
        return -1;
    }
    else
    {
        Printf("\t this file length: %d.\n",length);
    }
    unsigned int dataLen = 0;
    unsigned int trans_times = 0;
    dataLen = length;
    MessageHead.TxtLen = dataLen;
    unsigned char * buffer = malloc(dataLen);
    if (buffer == NULL) {
        Printf("\t分配内存%d失败,无法进行测试\t\n", dataLen);;
        return -1;
    }

    dataBuf = (unsigned char *) malloc(
            dataLen + 256);  
    if (dataBuf == NULL)
    {
        Printf("动态分配内存失败\n");
        free(buffer);
        return -1;
    }
    int index_while = 0;

    //首先将缓存文件内容的内存清0
    memset(buffer,0,dataLen);
    FILE *fp = fopen(filename, "rb");
    if (fp != NULL) {
        fread(buffer, dataLen, 1, fp);
        fclose(fp);
    }
    else
    {
        Printf("打开文件失败\r\n");
        free(dataBuf);
        free(buffer);
        return -1;
    }

    memset(dataBuf,0,dataLen + 256);

    memcpy(dataBuf, buffer ,dataLen);

    ret = TransferEncryptM_CC(dataBuf, (int *) &dataLen, &MessageHead);
    if (ret) {
        Printf("错误:错误码 = 0x%x\n", ret);
        free(dataBuf);
        free(buffer);
        return -1;
    }
    else
    {
        Printf("\t TransferEncryptM_CC length: %d.\n",dataLen);
        memset(file_name,0,512);
        sprintf(file_name, "%s.enc", filename);
        FILE *fp = fopen(file_name, "wb");
        if (fp != NULL) {
            fwrite(dataBuf, dataLen, 1, fp);
            fclose(fp);
        }
        free(buffer);
        free(dataBuf);
    }
    return 0;
}

int DYS_Test_DC(char* dir)
{
    int ret;
    unsigned char *dataBuf = NULL;
    int length = 0;
    char filename[512] = {0};
    //==========================================================================================
    // 首先创建一个新文件，分别对目录下的以enc.x 进行统计
    //==========================================================================================

    Printf("current file path and name is %s\r\n",dir);
    char* s_str = strrchr(dir,'.');
    if (s_str == NULL)
    {
        Printf("DYS_Test_DC->strrchr failed!!!\r\n");
        return -1;
    }
    int index =  strlen(dir)-strlen(s_str);
    strncpy(filename,dir,index);
//    Printf("文件名称:%s\r\n",filename);

    FILE *fp_w = fopen(filename, "wb");
    if (fp_w == NULL)
    {
        Printf("DYS_Test_DC->fopen failed!!!\r\n");
        return -1;
    }
    
    length = (int) getFileLength(dir);
    if (length <= 0) {
        Printf("\t this file is null.\n");
        fclose(fp_w);
        return -1;
    }
    unsigned int dataLen  = length;
    // 原文长度,存放在buffer 中
    unsigned char * buffer = malloc(length);
    if (buffer == NULL) {
        Printf("\t分配内存%d失败,无法进行测试\t\n", length);
        fclose(fp_w);
        return -1;
    }
    FILE *fp = fopen(dir, "rb");
    if (fp != NULL) {
        fread(buffer, length, 1, fp);
        fclose(fp);
    }else{
        Printf("DYS_Test_DC->[FILE *fp = fopen] failed!!!\r\n");
        fclose(fp);
        free(buffer);
        return -1;
    }

    dataBuf = (unsigned char *) malloc(length );
    if (dataBuf == NULL) {
        Printf("动态分配内存失败\n");
        fclose(fp_w);
        free(buffer);
        return -1;
    }
    memset(dataBuf,0,length);

    memcpy(dataBuf, buffer ,length);

    ret = TransferDecryptM_CC(dataBuf, (int *) &dataLen);
    if (ret) {
        Printf("错误:错误码 = 0x%x\n", ret);
        free(dataBuf);
        free(buffer);
        fclose(fp_w);
        return -1;
    }
    fwrite(dataBuf, dataLen, 1, fp_w);
    free(buffer);
    free(dataBuf);
    fclose(fp_w);
    return 0;
}



int commitTest(unsigned char* buff,int dataLen, APP_HEAD *app, int type) {
    MESSAGE_HEAD MessageHead;
    char md5[30] = {0};
    
//    uint32_t dstId[128] = {0};
    int i, ret;
    unsigned char *dataBuf = NULL;
    uint64_t start = 0, end = 0, cost = 0;
    double cost_v = 0.0;


    memset(&MessageHead, 0, sizeof(MESSAGE_HEAD));
    // 设置报头各字段
    MessageHead.AppType = 0xF0; 
    MessageHead.Level = type; 

    // 设置日期和时间
    // 日期为PC机系统日期，这个需要确保正常。时间精度无要求,填全零不影响功能。
    char strDate[64];
    char strTime[64];
    memset(strDate,0x00,sizeof(strDate));
    memset(strTime,0x00,sizeof(strTime));
    GetCurentTime(strDate, strTime);
    memcpy(MessageHead.Date, strDate, 8);
    memcpy(MessageHead.Time, strTime, 8);

    memset(MessageHead.Manage, 0, 8);


    MessageHead.pSrcAddID = srcId;

    MessageHead.DestNum = 0x1;



    MessageHead.pDestAddress = (unsigned char*)dstId;

 
    // 原文长度
    MessageHead.TxtLen = dataLen;


    dataBuf = (unsigned char *) malloc(
            dataLen + 256);  // !!
    if (dataBuf == NULL) {
        Printf("动态分配内存失败\n");
        return -1;
    }
    memcpy(dataBuf,buff,dataLen);


    if (g_srcid == 1) {
        ret = TransferEncryptM_NEW(dataBuf, &dataLen, &MessageHead, app);  
        if (ret) {
            Printf("加密错误:错误码 = 0x%x\n", ret);
            free(dataBuf);
            return ret;
        }
    }


    else {
        if(type == 0x01){
            Printf("正在解数据．．．\r\n");
            ret = TransferDecryptM_NEW(dataBuf, &dataLen, app);
            if (ret) {
                Printf("解错误:错误码 = 0x%x\n", ret);
                return ret;
            }else{
                Printf("解数据结束．．．\r\n");
            }           
        }else if(type == 0x05){
            ret = TransferEncryptM_NEW(dataBuf, &dataLen, &MessageHead, app);  
            if (ret) {
                Printf("错误:错误码 = 0x%x\n", ret);
                free(dataBuf);
                return ret;
            }            
        }

    }

    if (dataBuf != NULL) {
        free(dataBuf);
    }

    return 0;
}



//处理队列消息函数
void func_msg_queue(struct MSG_STRUCT t_msg_struct,unsigned short len){
    int ret=0;
    int response = -1;
    int try_num = 0;
    APP_HEAD app;
    bzero(&app, sizeof(APP_HEAD));
    ret = len;
    //判断消息类型
    if(t_msg_struct.mtype == 0x05){
        app.app1 = 5555;
        for(try_num = 0; try_num < TRY_SEND_TIME; try_num++){            
            response = commitTest((unsigned char*)t_msg_struct.buff,ret,&app,t_msg_struct.mtype);
            if (response != 0)
            {
                pthread_mutex_lock(&print_mutex);
                Printf("********发送新消息,发送失败,commitTest failed[第%d次],errornum=%d,当前链表长度[%d]\r\n",try_num+1,response,LinkLength(list_head));
                pthread_mutex_unlock(&print_mutex);
                print(len,(unsigned char*)t_msg_struct.buff);
            }else{
                pthread_mutex_lock(&print_mutex);
                Printf("发送新消息,发送成功,消息类型[0x%02x],消息长度[%d],当前链表长度[%d]!!!\r\n",t_msg_struct.mtype,len,LinkLength(list_head));
                pthread_mutex_unlock(&print_mutex);
                break;
            }            
        }
    }else if(t_msg_struct.mtype == 0x01){   
        app.app1 = 1111;
        for(try_num = 0; try_num < TRY_SEND_TIME; try_num++){
            response = commitTest((unsigned char*)t_msg_struct.buff,ret,&app,t_msg_struct.mtype);
            
            if (response != 0)
            {
                pthread_mutex_lock(&print_mutex);
                Printf("********发送新消息,发送失败,commitTest failed[第%d次],errornum=%d,当前链表长度[%d]\r\n",try_num+1,response,LinkLength(list_head));
                pthread_mutex_unlock(&print_mutex);
                print(len,(unsigned char*)t_msg_struct.buff);
            }else{
                pthread_mutex_lock(&print_mutex);
                Printf("发送新消息,发送成功,消息类型[0x%02x],消息长度[%d],当前链表长度[%d]!!!\r\n",t_msg_struct.mtype,len,LinkLength(list_head));
                pthread_mutex_unlock(&print_mutex);
                break;
            }   
                     
        }
    }else if(t_msg_struct.mtype == 0x12){
        j_status_t mmj_status;
        memset(&mmj_status, 0, sizeof(j_status_t));
        GetInfo(&mmj_status);
        sendMMJStatus(mmj_status,t_msg_struct.mtype);     
    }else if(t_msg_struct.mtype == 0x13){
        j_status_t mmj_status;
        unsigned short addr = 0;
        char time_str[512]={0};
        struct tm *pTempTm;
        struct timeval time;
        gettimeofday(&time,NULL);
        pTempTm = localtime(&time.tv_sec);
        if(pTempTm !=NULL){
            sprintf(time_str,"%04d-%02d-%02d %02d:%02d:%02d",pTempTm->tm_year+1900,pTempTm->tm_mon+1,pTempTm->tm_mday,pTempTm->tm_hour,pTempTm->tm_min,pTempTm->tm_sec);
        }
        memset(&mmj_status, 0, sizeof(j_status_t));
        memcpy(&mmj_status.red_state,t_msg_struct.buff+addr,sizeof(mmj_status.red_state));
        addr += sizeof(mmj_status.red_state);
        memcpy(&mmj_status.black_state,t_msg_struct.buff+addr,sizeof(mmj_status.black_state));
        addr += sizeof(mmj_status.black_state);
        if (strlen(time_str) > 64)
        {
            strncpy(mmj_status.date,time_str,64);
        }else{
            strncpy(mmj_status.date,time_str,strlen(time_str));
        }
        SetInfo(&mmj_status); 
    }else if(t_msg_struct.mtype == 0x03){
        char mmjid[128]={0};
        int t_result = -1;
        if(ret <= sizeof(mmjid) && ret > 0){
            memcpy(mmjid,t_msg_struct.buff,ret);
            GetIntFromMmjIdString(mmjid,dstId);  
            t_result = 0;          
        }else{
            Printf("********ID异常,请查看是否设置正确\r\n");
            t_result = 1;
        }
        sendCheckResult(t_result);
    }else{
        Printf("********mmj收到未定义消息类型\n");
    }       
}

//线程:处理消息链表
void* func_recv_data(void* arg){
    while(1){

        if(list_head != NULL){
            if(g_srcid == 1 && t_mmj_st.black_state == 1){
                Printf("消息链表不为空,发送新消息(红区->黑区) 消息类型:0x%02x,长度:%d\r\n",list_head->t_msg.mtype,list_head->len);
            }else if(g_srcid == 2 && t_mmj_st.red_state == 1){
                Printf("消息链表不为空,发送新消息(黑区->红区) 消息类型:0x%02x,长度:%d\r\n",list_head->t_msg.mtype,list_head->len);
            }
            func_msg_queue(list_head->t_msg,list_head->len);
            pthread_mutex_lock(&print_mutex); 
            DeleteListHead();
            pthread_mutex_unlock(&print_mutex);            
        }else{
            usleep(20*1000);
        }       
    }
    return NULL;
}


//线程:接收队列消息
void* func_rec_msg(void* arg){
    int ret = 0;
    int link_num = 0;
    struct MSG_STRUCT t_msg_struct;
    while(1){
        ret = msgrcv(msgid_recv,(void*)&t_msg_struct,8192,0,0);
        if(ret == -1){
            Printf("msgrcv failed...\n");
        }
        pthread_mutex_lock(&print_mutex);
        link_num = LinkLength(list_head);
        Printf("收到新消息,当前链表长度为%d\r\n",link_num);
        if(link_num < LINK_MAX_LENGTH){
            if(g_srcid == 1){
                if(t_mmj_st.black_state == 0){
                    if(t_msg_struct.mtype == 0x01 || t_msg_struct.mtype == 0x05 || t_msg_struct.mtype == 0x03 ){
                        AddListTill(t_msg_struct,ret);
                        Printf("当前红黑两区未同步,该类型报文[0x%02x]有效,添加到链表\r\n",t_msg_struct.mtype); 
                    }else{
                        Printf("当前红黑两区未同步,丢弃该类型报文[0x%02x]\r\n",t_msg_struct.mtype); 
                    }
                }else if(t_mmj_st.black_state == 1){
                    AddListTill(t_msg_struct,ret);
//                    Printf("当前红黑两区已同步,添加到链表\r\n");
                }else{
                    Printf("黑区状态未知,丢弃报文\r\n");
                }
            }else if(g_srcid == 2){
                if(t_mmj_st.red_state == 0){
                    if(t_msg_struct.mtype == 0x01){
                        AddListTill(t_msg_struct,ret);
                        Printf("当前红黑两区未同步,该类型报文[0x%02x]有效,添加到链表\r\n",t_msg_struct.mtype); 
                    }else{
                        Printf("当前红黑两区未同步,丢弃该类型报文[0x%02x]\r\n",t_msg_struct.mtype); 
                    }
                }else if(t_mmj_st.red_state == 1){
                    AddListTill(t_msg_struct,ret);
//                    Printf("当前红黑两区已同步,添加到链表\r\n");
                }else{
                    Printf("红区状态未知,丢弃报文\r\n");
                }
            }            
        }else{
            Printf(">>>>>>>>>>>当前链表已满[%d],丢弃该类型报文[0x%02x]\r\n",LINK_MAX_LENGTH,t_msg_struct.mtype);
        }
        pthread_mutex_unlock(&print_mutex);
    }
}

//回调函数
void recv_call_over(unsigned char *buffer, int length, APP_HEAD *app) {
//    pthread_mutex_lock(&print_mutex);
    // char t_buf[256]={0};
    int ret_len = 0;
    struct MSG_STRUCT t_msg_struct;
    if(app->app1 == 1111){
        t_msg_struct.mtype = 0x01;
        ret_len = length;
    }else if(app->app1 == 5555){
        t_msg_struct.mtype = 0x05;
        ret_len = parse_data(buffer,length);
    }
    Printf("收到隔离模块消息,消息类型:0x%02x,长度:%d\r\n",t_msg_struct.mtype,ret_len);
    print(ret_len,buffer);
    memcpy(t_msg_struct.buff,buffer,ret_len);
    if(length <= 8192){
        msgid_send = msgget(1236,0666|IPC_CREAT);
        if(msgid_send == -1){
            Printf("msgget failed...\n");
        }
        int ret = msgsnd(msgid_send,(void*)&t_msg_struct,ret_len,0); 
        if(ret == -1){
           Printf("msgsnd failed...\n");
        }
    }else{
        Printf("received data len = %d,bigger than 8192\n",ret_len);
    }

//    pthread_mutex_unlock(&print_mutex);
}


void* func_dy(void*arg){
    struct MSG_STRUCT t_msg_struct;
    char rec_file_name[512]={0};
    char tmp_file_name[256]={0};
    int msg_rec = 0;
    int rec = -1;
    int msgid_dy = msgget(1245,0666|IPC_CREAT);
    if(msgid_dy == -1){
        Printf("msgget failed...\n");
    }
    while(1)
    {
        memset(t_msg_struct.buff,0,1024);
        msg_rec = msgrcv(msgid_dy,(void*)&t_msg_struct,8192,0,0);
        if(msg_rec == -1){
            Printf("msgrcv failed...\n");
            continue;
        } 
        if(t_msg_struct.mtype == 0x11){//
            memset(tmp_file_name,0,sizeof(tmp_file_name));
            char* s_str = strrchr(t_msg_struct.buff,'/');
            if (s_str == NULL)
            {
                Printf("func_dy->strrchr failed!!!\r\n");
                rec = 0x01;
                Printf("*****************存储,失败[%d]*****************\r\n",rec);
            }else{
                strcpy(tmp_file_name,s_str+1);
                rec = DYS_Test_CC(t_msg_struct.buff);
                if(rec<0){
                    rec = 0x01;
                    Printf("*****************存储,失败[%d]*****************\r\n",rec);
                }
            }
            sendMsgRec(t_msg_struct.mtype,rec,tmp_file_name);
        }else if(t_msg_struct.mtype == 0x15){//
            char* s_str = strrchr(t_msg_struct.buff,'/');
            if (s_str == NULL)
            {
                rec = 0x01;
                Printf("func_dy->strrchr failed!!!\r\n");
                Printf("*****************存储,失败[%d]*****************\r\n",rec);                    
            }else{
                strcpy(tmp_file_name,s_str+1);
                s_str = strrchr(tmp_file_name,'.');
                if (s_str == NULL)
                {
                    rec = 0x01;
                    Printf("func_dy->strrchr failed!!!\r\n");
                    Printf("*****************存储,失败[%d]*****************\r\n",rec);                    
                }else{
                    int tmp_len = strlen(tmp_file_name)-strlen(s_str);
                    memset(rec_file_name,0,sizeof(rec_file_name));
                    strncpy(rec_file_name,tmp_file_name,tmp_len);
                    rec = DYS_Test_DC(t_msg_struct.buff);
                    if(rec<0){
                        rec = 0x01;
                        Printf("*****************存储,失败[%d]*****************\r\n",rec);
                    }                         
                }
            }
            sendMsgRec(t_msg_struct.mtype,rec,rec_file_name);
        }
    }

}


void sendMMJStatus(j_status_t t_mmj_status,unsigned char type){
    char sendMsg[1024]={0};
    unsigned short addr = 0;
    struct MSG_STRUCT t_msg_struct;
    memset(t_msg_struct.buff,0,sizeof(t_msg_struct.buff));

    memcpy(sendMsg+addr,&t_mmj_status.red_state,sizeof(t_mmj_status.red_state));
    addr+=sizeof(t_mmj_status.red_state);
    memcpy(sendMsg+addr,&t_mmj_status.black_state,sizeof(t_mmj_status.black_state));
    addr+=sizeof(t_mmj_status.black_state);
    memcpy(sendMsg+addr,&t_mmj_status.id,sizeof(t_mmj_status.id));
    addr+=sizeof(t_mmj_status.id);
    memcpy(sendMsg+addr,&t_mmj_status.date,sizeof(t_mmj_status.date));
    addr+=sizeof(t_mmj_status.date);
    memcpy(sendMsg+addr,&t_mmj_status.core_state,sizeof(t_mmj_status.core_state));
    addr+=sizeof(t_mmj_status.core_state);
    memcpy(sendMsg+addr,&t_mmj_status.disk_num,sizeof(t_mmj_status.disk_num));
    addr+=sizeof(t_mmj_status.disk_num);
    memcpy(sendMsg+addr,&t_mmj_status.key1_date,sizeof(t_mmj_status.key1_date));
    addr+=sizeof(t_mmj_status.key1_date);
    memcpy(sendMsg+addr,&t_mmj_status.key2_date,sizeof(t_mmj_status.key2_date));
    addr+=sizeof(t_mmj_status.key2_date);
    
    memcpy(t_msg_struct.buff,sendMsg,addr);
    t_msg_struct.mtype = type;
    msgid_send = msgget(1236,0666|IPC_CREAT);
    if(msgid_send == -1){
        Printf("msgget failed...\n");
    }
    int ret = msgsnd(msgid_send,(void*)&t_msg_struct,addr,0); 
    if(ret == -1){
        Printf("msgsnd failed...\n");
    }    
}


void sendCheckResult(int type){
    struct MSG_STRUCT t_msg_struct;
    memset(t_msg_struct.buff,0,sizeof(t_msg_struct.buff));
    t_msg_struct.mtype = 0x03;
    t_msg_struct.buff[0] = type;
    msgid_send = msgget(1236,0666|IPC_CREAT);
    if(msgid_send == -1){
        Printf("msgget failed...\n");
    }
    int ret = msgsnd(msgid_send,(void*)&t_msg_struct,1,0); 
    if(ret == -1){
        Printf("msgsnd failed...\n");
    }  
}



//***********************************main**************************************************
int main(int argc, char *argv[]) {
    int responce_set = -1;
    if(argc != 2){
        fprintf(stderr, "Usage: %s {AP|CP}\n", argv[0]);
        exit(EXIT_FAILURE);            
    }
    if(strcmp(argv[1],"AP") == 0 || strcmp(argv[1],"ap") == 0 || strcmp(argv[1],"Ap") == 0 || strcmp(argv[1],"aP") == 0 ){
        g_srcid = 1;     
    }else if(strcmp(argv[1],"CP") == 0 || strcmp(argv[1],"cp") == 0 || strcmp(argv[1],"Cp") == 0 || strcmp(argv[1],"cP") == 0){
        g_srcid = 2;      
    }else{
        exit(EXIT_FAILURE);
    }
    Printf(">>>>>>>>>>>>>>系统初始化,动态加载libdcsext.so中函数..\n");
    loadLibraryFunction();
    Printf(">>>>>>>>>>>>>>系统初始化,完成动态加载libdcsext.so中函数\n");
    msgid_recv = msgget(1235,0666|IPC_CREAT);
    if(msgid_recv == -1){
        Printf("msgget failed...\n");
    }
    int server = 0;
    server = 1;
    if (InitServer(g_srcid, recv_call_over) == -1) {
        Printf(">>>>>>>>>>>>>>系统初始化,初始化InitServer()函数错误\n");
        return -1;
    }
    Printf(">>>>>>>>>>>>>>系统初始化,完成初始化InitServer()函数\n");    
    pthread_t th_dy;
    pthread_create(&th_dy,NULL,func_dy,NULL);//调用式
    pthread_t th_recv;
    pthread_create(&th_recv,NULL,func_recv_data,NULL);
    pthread_t th_msg;
    pthread_create(&th_msg,NULL,func_rec_msg,NULL);
    //获取本机状态
    memset(&t_mmj_st, 0, sizeof(j_status_t));
    GetInfo(&t_mmj_st);
    if(strlen(t_mmj_st.id) == 0){
        Printf(">>>>>>>>>>>>>>系统初始化,ID为空,退出!!!\r\n");//是否需要重新读取三次
        exit(-1);
    }else{
        strcpy(strSrcId,t_mmj_st.id);
        GetIntFromMmjIdString(strSrcId,srcId);
        SetMmjId4Bytes(srcId);
        Printf(">>>>>>>>>>>>>>系统初始化,获取到本机ID[%s][%x %x %x %x]<<<<<<<<<<<<<<<<\r\n",strSrcId,srcId[0],srcId[1],srcId[2],srcId[3]);
        usleep(10*1000);  
    }
    //同步设置
    if(g_srcid == 1){
        WriteStructToFile(t_mmj_st,"Info/mmj_info/local_mmj.dat");
        //获取对端ID
        Printf(">>>>>>>>>>>>>>系统初始化,设置对端ID\r\n");
        if(readMMJId("./Info/mmj_info/mmjstdid.txt") == -1){
            char mmjid[]="10.0.0.1.1.5";
            GetIntFromMmjIdString(mmjid,dstId);//默认配置
            Printf(">>>>>>>>>>>>>>系统初始化,设置默认对端ID[%s][%x %x %x %x]",mmjid,dstId[0],dstId[1],dstId[2],dstId[3]);
        }
        unsigned short addr = 0;
        char time_str[512]={0};
        struct tm *pTempTm;
        struct timeval time;
        gettimeofday(&time,NULL);
        pTempTm = localtime(&time.tv_sec);
        if(pTempTm !=NULL){
            sprintf(time_str,"%04d-%02d-%02d %02d:%02d:%02d",pTempTm->tm_year+1900,pTempTm->tm_mon+1,pTempTm->tm_mday,pTempTm->tm_hour,pTempTm->tm_min,pTempTm->tm_sec);
        }
        t_mmj_st.red_state = 1;
        t_mmj_st.black_state = 0;
        char t_mmj_id[64]="2.0.0.1.1.1";
        strncpy(t_mmj_st.id,t_mmj_id,strlen(t_mmj_id));
        if (strlen(time_str) > 64)
        {
            strncpy(t_mmj_st.date,(const char*)time_str,64);
        }else{
            strncpy(t_mmj_st.date,(const char*)time_str,strlen(time_str));
        }

        for (int i = 0; i < 3; i++)
        {
            responce_set = SetInfo(&t_mmj_st);
            if (responce_set == 0)
            {
                Printf(">>>>>>>>>>>>>>系统初始化,设置信息成功\r\n");
                break;
            }else{
                Printf(">>>>>>>>>>>>>>系统初始化,第%d次设置信息时错误，请重试．．．\r\n",i+1);
                usleep(1000*10);
            }
            
        }
        unsigned int t_black_stat = 0;
        while(!t_black_stat){
            GetInfo(&t_mmj_st);
            t_black_stat = t_mmj_st.black_state;
            usleep(1000*10);
        }
        Printf(">>>>>>>>>>>>>>系统初初始化,对端状态已具备<<<<<<<<<<<<<<<<\r\n");
    }
    if(g_srcid == 2){
        unsigned int t_red_stat = 0;
        t_mmj_st.black_state = 1;
        for (int i = 0; i < 3; i++)
        {
            responce_set = SetInfo(&t_mmj_st);
            if (responce_set == 0)
            {
                Printf(">>>>>>>>>>>>>>系统初始化,设置信息成功\r\n");
                break;
            }else{
                Printf(">>>>>>>>>>>>>>系统初始化,设置信息时错误[]，请重试．．．\r\n",i+1);
                usleep(1000*10);
            }
        }
//        SetInfo(&t_mmj_st);
        while (!t_red_stat)
        {
            GetInfo(&t_mmj_st);
            t_red_stat = t_mmj_st.red_state;
            usleep(1000*10);
        }
        Printf(">>>>>>>>>>>>>>系统初始化,对端红区状态已具备<<<<<<<<<<<<<<<<\r\n");
    }

    pthread_join(th_dy,NULL);
    pthread_join(th_recv,NULL);
    pthread_join(th_msg,NULL);
    return 0;
}

