#ifndef __USER_LINK__
#define __USER_LINK__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 用户的信息
typedef struct  {
    char name[10]; // 姓名
    int loginStatus; // 是否登录
    int fd; // 文件描述符
    
} UserInfo;


// 用户链表
typedef struct userLink {
    UserInfo user;

    struct userLink *next;

} UserLink;




// 向链表中, 添加用户信息
void add_link(UserLink * link, UserInfo userInfo) {

    UserLink * tmp = link;
    while (tmp->next) {
        tmp = tmp->next;
    }
    tmp->next = (UserLink *)malloc(sizeof(UserLink) );
    
    tmp->next->user = userInfo;
    tmp->next->next = NULL;
}

// 登录,成功
int login_from_link(UserLink * link, UserInfo userInfo) {
    UserLink * tmp = link->next;
    
    while (tmp) {
        if ( tmp->user.fd == userInfo.fd) {
            // 登录成功
            strcpy(tmp->user.name, userInfo.name);
            tmp->user.loginStatus = 1; // 登录成功
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

// 从链表中, 删除,文件描述符 (主动退出)
int delete_fd_from_link(UserLink * link, UserInfo userInfo) {

    UserLink * tmp = link;
    // if (tmp->next == NULL) {
    //     printf("链表头, 为空\n");
    // } else {
    //     printf("链表头, 不为空\n");
    // }
    // printf("fd1: %d\n", tmp->next->user.fd);

    while (tmp->next) {
        // printf("fd: %d\n", tmp->next->user.fd);
        // 找到,删除
        if ( tmp->next->user.fd == userInfo.fd) {
            tmp->next = tmp->next->next;

            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

// 从链表中, 删除用户 (踢人)
int delete_name_from_link(UserLink * link, UserInfo userInfo) {

    UserLink * tmp = link;
    while (tmp->next) {
        // 找到,删除
        if (! strcmp(tmp->next->user.name, userInfo.name)) {
            tmp->next = tmp->next->next;

            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

int loop_link(UserLink *link) {
    UserLink *tmp = link->next; // 从链表的第二个,进行查找

    while (tmp) {
        printf("%s ", tmp->user.name);
        tmp = tmp->next;
    }
    
    printf("\n");
}

// 获取,所有的登录用户
void get_all_login_user(UserLink *link, char *buf) {
    UserLink *tmp = link->next; // 从链表的第二个,进行查找
    
    char bufTmp[30] = {'\0'};
    while (tmp) {
        // printf("name: %s\n", tmp->user.name);
        sprintf(bufTmp, "%s ", tmp->user.name); // 会全部覆盖
        
        strcat(buf, bufTmp);
        tmp = tmp->next;
    }
}


// 获取,登录用户的id
UserLink *get_user_ByName(UserLink *link, char *name) {
    UserLink *tmp = link->next; // 从链表的第二个,进行查找

    while (tmp) {
        if (! strcmp( tmp->user.name, name)) { // 找到用户
            return tmp;
        }
        tmp = tmp->next;
    }
    
    return NULL;
}

// 获取,登录用户的id
UserLink *get_user_ByFd(UserLink *link, int fd) {
    UserLink *tmp = link->next; // 从链表的第二个,进行查找

    while (tmp) {
        if (tmp->user.fd == fd) { // 找到用户
            return tmp;
        }
        tmp = tmp->next;
    }
    
    return NULL;
}

// 链表的初始化
void init_link(UserLink **link) {
    
    // 初始化,链表头
    *link = (UserLink *)malloc(sizeof(UserLink) );
    memset(*link, 0 , sizeof(*link) );
}

// 删除链表
void delete_link(UserLink *link) {
    UserLink *tmp = link;

    UserLink *tmp2;
    while (tmp) {
        tmp2 = tmp->next; // 保存下一个
        free(tmp); // 删除,节点

        tmp = tmp2; // 指向,下一个
    }
}
// int main() {

//     UserLink userLink; // 第一个链表头,不存储内容

//     UserInfo userInfo1;
//     sprintf(userInfo1.name, "%s", "hdy1");
    
//     add_link(&userLink, userInfo1);


//     // 添加,用户
//     UserInfo userInfo2;
//     sprintf(userInfo2.name, "%s", "hdy2");
//     add_link(&userLink, userInfo2);

//     loop_link(&userLink);

//     delete_name_from_link(&userLink, userInfo2);
//     // delete_name_from_link(&userLink, userInfo1);

//     loop_link(&userLink);

// }


#endif
