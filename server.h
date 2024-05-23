#include "common.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "user_link.c"

#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>

#include <sys/epoll.h>

#include <sys/signal.h>

#include <event2/event.h>

// 用户表
LoginInfo UserTableArr[] = {
    { "hdy", "123456" },
    { "hdy1", "123456" },
    { "hdy2", "123456" },
};
typedef struct libevent_data_s libevent_data;


typedef struct {
    struct sockaddr_in addr;
    int s_s; 

    int port;

    int clientNum; // 客户端的最大数量

    UserLink *userLink;

    pthread_t acceptTh; // 线程id
    pthread_t workerTh;

    fd_set fds;

    int epFd; // epoll 套接字

    struct event_base *evBase;
    // struct event **evArr; // event2 的事件
    libevent_data *evArr; // 保存多个

} ServerInfo ;

// libevent,回调时,携带的数据
struct  libevent_data_s{
    // ServerInfo *server;
    int fd; // 携带的数据
    struct event *ev;
    
} ;

void init_all(ServerInfo *server);

void init_server(ServerInfo *server);

void *accept_connect(void *arg);

void init_fd_set(ServerInfo *server);

void change_fd_set(ServerInfo *server);

void read_from_fd(ServerInfo *server, int fd);

int handle_login(ServerInfo *server, int fd, SendMsg *msg);


int handle_quit(ServerInfo *server, int fd);


void handle_send(ServerInfo *server, int fd, SendMsg *msg);


void *read_from_client(void *arg);

void handle_all_friend(ServerInfo *server , int fd);


void send_login_result(ServerInfo *server , int fd, int result) ;


void destory_all(int sigNum);

void init_epoll(ServerInfo *server);
void *read_from_client_epoll(void *arg);


void init_libevent(ServerInfo *server) ;


// void libevent_callback(int fd, int event_type, void *arg);
void libevent_callback(int fd, short int event_type, void *arg);

void *read_from_client_libevent(void *arg);


void destroy_libevent(ServerInfo *server) ;

