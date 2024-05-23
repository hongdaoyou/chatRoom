#include "server.h"

#define EVENT_TYPE_SELECT 1
#define EVENT_TYPE_EPOLL 2
#define EVENT_TYPE_EVENT2 3

#define EVENT_TYPE EVENT_TYPE_EVENT2

ServerInfo server;

// 全部,初始化
void init_all(ServerInfo *server) {
    // 初始化链表
    init_link(&server->userLink); // 要改变,它的指针值. 那么,就要传入其地址

    // 初始化,服务器
    init_server(server);

    init_epoll(server);
    init_libevent(server);

    // 初始化,连接线程
    pthread_create(&server->acceptTh, NULL, accept_connect,(void *)server );

    // 初始化, 工作线程,用来读写
    #if EVENT_TYPE == EVENT_TYPE_SELECT
    pthread_create(&server->workerTh, NULL, read_from_client,(void *)server );
    
    #elif EVENT_TYPE == EVENT_TYPE_EPOLL
    pthread_create(&server->workerTh, NULL, read_from_client_epoll,(void *)server );

    #elif EVENT_TYPE == EVENT_TYPE_EVENT2

    pthread_create(&server->workerTh, NULL, read_from_client_libevent,(void *)server );

    #endif

    // 注册,信号函数
    signal(SIGUSR1, destory_all);

    // 等待线程的结束
    pthread_join(server->acceptTh, NULL);
    pthread_join(server->workerTh, NULL);

    return;
}

// 初始化, libevent库
void init_libevent(ServerInfo *server) {
    #if EVENT_TYPE == EVENT_TYPE_EVENT2

    server->evBase = event_base_new();

    // 保存,客户端
    // server->evArr = (libevent_data **)malloc(server->clientNum * sizeof(libevent_data));
    server->evArr = (libevent_data *)malloc(server->clientNum * sizeof(libevent_data));

    #endif
}

void destroy_libevent(ServerInfo *server) {
    #if EVENT_TYPE == EVENT_TYPE_EVENT2

    free(server->evArr);

    event_base_free(server->evBase);
    #endif
}

// 添加,fd
void add_event_libevent(ServerInfo *server, int fd) {
    #if EVENT_TYPE == EVENT_TYPE_EVENT2
    libevent_data * eventData = &server->evArr[fd];
    
    // eventData->server = server;  // 即使指针,作为回调. 传递.也会出错.
    // printf("33\n");
    
    eventData->fd = fd;

    printf("I create event: %d\n", fd);
    // 创建,事件
    eventData->ev = event_new(server->evBase, fd, EV_READ | EV_PERSIST, libevent_callback, (void *)&eventData);

    // if (server->evArr[fd]. == NULL) {
    //     printf("create event fail\n");
    // }

    // 添加到, 事件集合中
    int ret = event_add(server->evArr[fd].ev, NULL);
    if (ret != 0) {
        perror("event_add");
    }

    #endif
}  

void remove_lib_event2(ServerInfo *server, int fd) {
    #if EVENT_TYPE == EVENT_TYPE_EVENT2

    event_del(server->evArr[fd].ev);    

    #endif
}


// libevent 的回调
void libevent_callback(int fd, short int event_type, void *arg) {
    libevent_data *data = (libevent_data *)arg;

    printf("libevent callback\n");

    read_from_fd(&server, fd);

}

// 初始化,epool
void init_epoll(ServerInfo *server) {
    #if EVENT_TYPE == EVENT_TYPE_EPOLL

    // 创建,epoll, 套接字
    server->epFd = epoll_create(server->clientNum );
    if (server->epFd < 0) {
        perror("epoll_create");
    }

    #endif
}

// 将fd,添加到,epoll,监听集合中
void add_epoll_event(ServerInfo *server, int fd) {
    #if EVENT_TYPE == EVENT_TYPE_EPOLL
    
    // 事件
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;

    int ret = epoll_ctl(server->epFd, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0) {
        perror("epoll_ctl add");
        return ;
    }

    #endif
}

// 移除 epoll
void remove_epoll_event(ServerInfo *server, int fd) {
    #if EVENT_TYPE == EVENT_TYPE_EPOLL

    int ret = epoll_ctl(server->epFd, EPOLL_CTL_DEL, fd, NULL);
    if (ret < 0) {
        perror("epoll_ctl del");
    }

    #endif
}


// 摧毁,all
void destory_all(int sigNum) {
    if (sigNum == SIGUSR1) {
        // 删除链表
        delete_link(server.userLink);

        // 取消线程
        int ret = pthread_cancel(server.acceptTh);
        if (ret) {
            perror("pthread_canncel");
            return;
        }
        ret = pthread_cancel(server.workerTh);
        if (ret) {
            perror("pthread_canncel");
            return;
        }

        destroy_libevent(&server);
    }

}

// 初始化,服务器
void init_server(ServerInfo *server) {
    
    server->port = 8888;
    server->clientNum = 10;

    // 创建,套接字
    server->s_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (server->s_s == -1) {
        perror("socket:");
        return;
    }

    int reuse = 1;
    if (setsockopt(server->s_s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse) ) ) {
        perror("setsockopt");
        return;
    }


    // 初始化,ip
    int localAddrLen = sizeof(server->addr );
    
    memset(&server->addr, 0, localAddrLen );
    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = htonl(INADDR_ANY );
    server->addr.sin_port = htons(server->port );

    // 绑定ip
    bind(server->s_s, (struct sockaddr*)&server->addr, localAddrLen);

    // 监听
    listen(server->s_s, server->clientNum);

}


// 接收,客户端的连接
void *accept_connect(void *arg) {
    ServerInfo *server = (ServerInfo *)arg;
    
    // 地址
    struct sockaddr addr;
    int addrLen = sizeof(struct sockaddr);
    
    int fd;
    while (1) {
        printf("I accept:\n");
        // 接收,连接
        fd = accept(server->s_s, &addr, &addrLen);
        if (fd == -1) {
            perror("accept ");
            continue;
        }
        printf("I accept one:\n");

        // 添加,到登录链表中
        UserInfo userInfo;
        userInfo.fd = fd;
        add_link(server->userLink, userInfo);

        // 添加事件
        add_epoll_event(server, fd);

        add_event_libevent(server, fd);
    }
}

// 初始化,fd
void init_fd_set(ServerInfo *server) {
    
    FD_ZERO(&server->fds);
    
    // printf("0\n");
    UserLink *tmp = server->userLink->next;
    while (tmp) {
        FD_SET(tmp->user.fd, &server->fds);

        tmp = tmp->next;
    }
     
}

// 改变的fd个数
void change_fd_set(ServerInfo *server) {
    UserLink *tmp = server->userLink->next;
    while (tmp) {
        // 有改变的fd
        if (FD_ISSET(tmp->user.fd, &server->fds)) {
            read_from_fd(server, tmp->user.fd);
        }

        tmp = tmp->next;
    }
}

//  从fd,读取,内容
void read_from_fd(ServerInfo *server, int fd) {

    SendMsg msg;
    int ret = read(fd, &msg, sizeof(SendMsg));
    printf("read %d:\n", ret);
    // if (ret == -1) {
    if (ret <= 0) {
        // 退出连接
        handle_quit(server, fd);
        printf("client 异常. 退出\n");
        close(fd); 
        // 断开连接
    } else {
        printf("get request %d: \n", msg.func == ALL_FRIEND_FUNC);

        switch(msg.func) {
            case LOGIN_FUNC:
                handle_login(server, fd, &msg);
            break;
    
            case QUIT_FUNC:
                handle_quit(server, fd);

            break;
            
            case ALL_FRIEND_FUNC: // 获取,所有的登录用户
                // printf("ALL_FRIEND_FUNC\n");
                handle_all_friend(server, fd);
            break;

            case SEND_FUNC: // 发送
                handle_send(server, fd, &msg);
            break;
        
        }
    }
}

// 进行,登录
int handle_login(ServerInfo *server, int fd, SendMsg *msg) {

    LoginInfo login = msg->payload.login;

    int len = sizeof(UserTableArr) / sizeof(UserTableArr[0]);

    printf("一个用户,来验证密码 %s %s\n", login.userName, login.passwd);


    for (int i=0; i < len; i++) {
        // 用户名, 和密码, 进行匹配
        if ( !strcmp(login.userName, UserTableArr[i].userName) && ! strcmp(login.passwd, UserTableArr[i].passwd) ) {
            // 设置登录信息
            UserInfo userInfo;
            userInfo.fd = fd;
            strcpy(userInfo.name, login.userName);

            // printf("%d", server->userLink.user.fd );

            // 登录成功
            login_from_link(server->userLink, userInfo);
            printf("22\n");

            printf("%s login success\n", login.userName);
            send_login_result(server, fd, 1);
            return 1;
        }
    }
    printf("验证,密码失败\n");
    send_login_result(server, fd, 0);

    return -1;

}

// 退出
int handle_quit(ServerInfo *server, int fd) {
    UserInfo userInfo;
    userInfo.fd = fd;
    
    delete_fd_from_link(server->userLink, userInfo);

    remove_epoll_event(server, fd);
    
    // 取消,对fd的监听
    remove_lib_event2(server, fd);
}


// 向客户端,登录的结果
void send_login_result(ServerInfo *server , int fd, int result) {

    // 服务器,发送的信息
    SendServerMsg serverMsg;
    serverMsg.func = LOGIN_FUNC;

    serverMsg.result = result;

    // 发送
    write(fd, &serverMsg, sizeof(SendServerMsg) );
}


// 获取,所有的朋友
void handle_all_friend(ServerInfo *server , int fd) {

    // 服务器,发送的信息
    SendServerMsg serverMsg;
    serverMsg.func = ALL_FRIEND_FUNC;
    // 清空
    memset(serverMsg.payload, 0, sizeof(serverMsg.payload));

    get_all_login_user(server->userLink, serverMsg.payload);

    // 发送
    write(fd, &serverMsg, sizeof(SendServerMsg) );
    printf ("I send: %s\n", serverMsg.payload);
}


// 发送消息
void handle_send(ServerInfo *server, int fd, SendMsg *msg) {
    SendToMsg sendToMsg = msg->payload.send;

    printf("I recv: %s %s\n", sendToMsg.receiver, sendToMsg.message);

    UserLink *toUser = get_user_ByName(server->userLink, sendToMsg.receiver);

    // 没有map,效率,就是差
    UserLink *fromUser = get_user_ByFd(server->userLink, fd);

    // 服务器,发送的信息
    SendServerMsg serverMsg;
    serverMsg.func = SEND_FUNC;

    if (toUser) {
        // 找到
        // 拼接信息
        sprintf(serverMsg.payload, "From %s to %s: %s\n", fromUser->user.name,toUser->user.name, sendToMsg.message);

        write(toUser->user.fd, &serverMsg, sizeof(SendServerMsg) );

    } else {
        sprintf(serverMsg.payload, "not find this user");
   
        write(fd, &serverMsg, sizeof(SendServerMsg) );
    }
    // 发送

}

void *read_from_client_epoll(void *arg) {
    ServerInfo *server = (ServerInfo *)arg;

    struct epoll_event recvEventArr[server->clientNum];

    while (1) {
        int size = epoll_wait(server->epFd, recvEventArr, server->clientNum , -1);

        int i = 0;
        while (i < size) {
            int fd = recvEventArr[i].data.fd;
            
            // 读取,文件描述符的内容
            read_from_fd(server, fd);
            i++;
        }
    }

}

// 进入循环
void *read_from_client_libevent(void *arg) {
    #if EVENT_TYPE == EVENT_TYPE_EVENT2

    ServerInfo *server = (ServerInfo *)arg;

    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 1000000000 / 2; // 每半秒,超时一次

    while (1) {
        printf("libevent start\n");
        event_base_dispatch(server->evBase);
        printf("libevent end\n");

        nanosleep(&timeout, NULL);
    }
    
    #endif
}

// 读取,用户的传入的信息
void *read_from_client(void *arg) {
    ServerInfo *server = (ServerInfo *)arg;
    
    // 超时时间
    struct timeval timeout;

    printf("I select: \n");

    while (1) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000000 / 2; // 每1/2 秒, 检测一次;

        init_fd_set(server);

        int ret = select(server->clientNum+1, &server->fds, NULL, NULL, &timeout);
        if (ret == -1) {
            perror("select 异常");
            continue;
        } else if (ret == 0) {
            // printf("select 超时\n");
            continue;
        } else {
            printf("one request is coming\n");
            change_fd_set(server);
        }
    }
}

int main() {

    init_all(&server);

}


