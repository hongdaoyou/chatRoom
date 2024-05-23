
// 登录信息
typedef struct {
    char userName[20];
    char passwd[20];
} LoginInfo;

// 发送信息
typedef struct {
    char receiver[20];
    char message[100];
} SendToMsg;


typedef struct {
    int func; // 功能

    union {
        // 登录信息
        LoginInfo login;

        // 发送信息
        SendToMsg send;

    } payload;
} SendMsg;


// 定义,消息的功能符号
typedef enum {
    LOGIN_FUNC,
    QUIT_FUNC,
    SEND_FUNC,
    ALL_FRIEND_FUNC,
} SendMsgFunc;


// 服务器,发送的消息
typedef struct {
    int func; // 功能

    int result;
    char payload[1024];
} SendServerMsg;


