#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
// #include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int c_s;

    int ip; // 远程ip
    int port; // 远程客户端

    int pageNum; // 当前,锁在的页面
    int loginStatus; // 登录的状态

    struct sockaddr_in addr;

    pthread_t readTh; // 读取的线程id
    pthread_t uiTh; // ui线程

} ClientInfo;

void init_all(ClientInfo *client);

void init_page(ClientInfo *client) ;

void init_client(ClientInfo *client);

void *read_from_server(void *arg);

void *ui_page(void *arg);


void chat_page(ClientInfo *client);


void login_page(ClientInfo *client);

void all_friend_send(ClientInfo *client);


void chat_one_friend_send(ClientInfo *client);


void login_send(ClientInfo *client);

void quit_send(ClientInfo *client);


