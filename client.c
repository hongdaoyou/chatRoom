#include "common.h"
#include "client.h"

ClientInfo client;

void init_all(ClientInfo *client) {
    init_page(client );

    init_client(client);

    pthread_create(&client->readTh, NULL, read_from_server, (void *)client);


    pthread_create(&client->uiTh, NULL, ui_page, (void *)client);

    // 等待线程的结束
    pthread_join(client->readTh, NULL);
    pthread_join(client->uiTh, NULL);

}

void init_page(ClientInfo *client) {
    client->pageNum = LOGIN_FUNC; // 设置为,登录页面

    client->loginStatus = -1; // 未登录
}

// 初始化,客户端
void init_client(ClientInfo *client) {
    client->port = 8888;
    // client->ip = "localhost";

    // 创建,套接字
    client->c_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (client->c_s == -1) {
        perror("socket:");
        return;
    }

    client->addr.sin_family = AF_INET;
    client->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client->addr.sin_port = htons(client->port);

    // 连接,远程客户
    int ret = connect(client->c_s, (struct sockaddr*)&client->addr, sizeof(struct sockaddr));
    
    if (ret == -1) {
        perror("connect 失败");
        exit(-1);;
    } else {
        // printf("连接成功");
    }
}

//  从fd,读取,内容
void *read_from_server(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;

    // 服务器,发送的信息
    SendServerMsg serverMsg;

    while (1) {
        int ret = read(client->c_s, &serverMsg, sizeof(SendServerMsg));
        // printf("read: %d\n", ret);
        if (ret <= 0) {
            printf ("服务器,异常. 那么,我退出了\n");
            exit(-1);;
            // 服务器,关闭
        } else {
            switch(serverMsg.func ) {
                case LOGIN_FUNC: // 登录的结果
                    if (serverMsg.result == 1) {
                        client->pageNum = SEND_FUNC; // 进行,发送的页面
                        client->loginStatus = 1;
                    } else {
                        client->loginStatus = 0; // 登录失败
                    }

                break;

                case SEND_FUNC: case ALL_FRIEND_FUNC: // 接收到,其它信息.直接显示
                    printf("Recv: %s\n", serverMsg.payload);

                break;

                default:
                    printf("读取到,异常数据\n");
                break;;
            }

        }
    }

}

// ui界面的读取
void *ui_page(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    
    while (1) {
        switch (client->pageNum) {
            case LOGIN_FUNC: // 登录
                login_page(client);
            break;

            case SEND_FUNC: // 聊天页面
                chat_page(client);
            
            break;

            default:
                printf("页面异常\n");
                exit(-1);
            break;
        }

    }
}

// 聊天的首界面
void chat_page(ClientInfo *client) {
    system("clear");

    printf("        ChatRoom_Hdy\n");
    printf("     1) Display friends\n");
    printf("     2) Chat one friends\n");
    printf("     3) Exit\n\n");

    printf("please input number: ");
    int num;
    scanf("%d", &num);

    switch (num) {
        case 1: // 显示在线用户
            all_friend_send(client);
            break;

        case 2: // 聊天
            chat_one_friend_send(client);
            break;
    
        case 3: // 退出
            quit_send(client);
            break;

        default:
            printf("input error\n");
            break;
    }
}

// 登录页面
void login_page(ClientInfo *client) {
    system("clear");

    printf("        ChatRoom_Hdy\n");
    printf("............................\n");
    printf("     1)Login\n");
    printf("     2)exit\n");
    printf("............................\n\n");
    
    printf("please input number: ");

    int num;
    scanf("%d", &num);

    switch (num) {
        case 1: // 登录
            login_send(client);
            break;

        case 2: // 退出
            quit_send(client);
            exit(-1);;
            break;

        default:
            printf("input error\n");
            break;
    }

}

// 获取,所有朋友的请求
void all_friend_send(ClientInfo *client) {
    // 发送的信息
    SendMsg sendMsg;
    sendMsg.func = ALL_FRIEND_FUNC;

    write(client->c_s, &sendMsg, sizeof(SendMsg));
    getchar();
    getchar();
}

// 和朋友,聊天
void chat_one_friend_send(ClientInfo *client) {

    // 发送的信息
    SendMsg sendMsg;
    sendMsg.func = SEND_FUNC;

    SendToMsg sendToMsg;
    memset(sendToMsg.message, 0, sizeof(sendToMsg.message));

    // 获取发送的name,用户
    printf("input friend Name: ");
    scanf("%s", sendToMsg.receiver);

    getchar(); // 获取,回车符

    printf("input message: ");
    
    fgets( sendToMsg.message, sizeof(sendToMsg.message), stdin);
    sendMsg.payload.send = sendToMsg;

    // printf("I get: %s\n", sendToMsg.message);

    write(client->c_s, &sendMsg, sizeof(SendMsg));

    getchar();
}


// 登录发送
void login_send(ClientInfo *client) {

    LoginInfo loginInfo;
    loginInfo.userName,

    printf("input userName: ");
    scanf("%s", loginInfo.userName);

    printf("input passwd: ");
    scanf("%s", loginInfo.passwd);

    // 发送的信息
    SendMsg sendMsg;
    sendMsg.func = LOGIN_FUNC;

    sendMsg.payload.login = loginInfo;

    write(client->c_s, &sendMsg, sizeof(SendMsg));
    printf("已经向服务器,发送信息\n");

    
    struct timespec ts;
    ts.tv_sec = 0;
    // ts.tv_nsec= 9000000000000;
    ts.tv_nsec= 500000000; // 900 000 000
    nanosleep(&ts, NULL);

    if (client->loginStatus == 1) {
        printf("登录成功\n");
    } else {
        printf("登录失败\n");
    }
    getchar();
    getchar();
}

// 退出
void quit_send(ClientInfo *client) {

    // 发送的信息
    SendMsg sendMsg;
    sendMsg.func = QUIT_FUNC;

    write(client->c_s, &sendMsg, sizeof(SendMsg));

    exit(-1);;
}


int main() {

    // 初始化
    init_all(&client);


}