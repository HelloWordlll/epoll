/*************************************************************************
	> File Name: sever.c
	> Author: 
	> Mail: 
	> Created Time: Thu 07 Mar 2024 08:03:26 PM CST
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>

typedef struct task{
    int fd;
    struct task* next;
}task;

typedef struct task_pool{
    task* head;
    task* tail;
    pthread_mutex_t lock;
    pthread_cond_t havetask;
}task_pool;

task_pool* task_init(){
    task_pool *tp = (task_pool *)malloc(sizeof(task_pool));
    tp->head = NULL;
    tp->tail = NULL;
    pthread_mutex_init(&tp->lock, NULL);
    pthread_cond_init(&tp->havetask, NULL);

    return tp;
}

void task_push(task_pool *tp, int fd){
    pthread_mutex_lock(&tp->lock);

    task *t = (task *)malloc(sizeof(task));
    t->fd = fd;
    t->next = NULL;

    if(!tp->tail){
        tp->head = tp->tail = t;
    }else{
        tp->tail->next = t;
        tp->tail = t;
    }

    pthread_cond_broadcast(&tp->havetask);
    pthread_mutex_unlock(&tp->lock);
}

task task_pop(task_pool *tp){
    pthread_mutex_lock(&tp->lock);
    
    while(tp->head == NULL){
        pthread_cond_wait(&tp->havetask, &tp->lock);
    }

    task tep, *t;
    t = tp->head;
    tep = *t;
    tp->head = tp->head->next;

    if(!tp->head){
        tp->tail = NULL;
    }

    free(t);
    pthread_mutex_unlock(&tp->lock);

    return tep;
}

void task_free(task_pool* tp){
    pthread_mutex_lock(&tp->lock);
    task *p = tp->head, *k;

    while(p){
        k = p;
        p = p->next;
        free(k);
    }
    tp->head = NULL;

    pthread_mutex_unlock(&tp->lock);
    pthread_mutex_destroy(&tp->lock);
    pthread_cond_destroy(&tp->havetask);
    free(tp);
    return ;
}

void* thr(void* arg){                                                                    
    pthread_detach(pthread_self());//自己给自己收尸                                                      
    while(1){                                                                            
        task_pool *tp = (task_pool *)arg;                                                
        task p = task_pop(tp);//从队列中获取一个任务                                                           
        int connfd = p.fd;//记录文件描述符                                                               
        char buf[125];//设置缓冲区                                                                   
        int n;                                                                           
        printf("get : %d\n", connfd);                                                    
        if(1){//对任务的操作                                                                           
            n = read(connfd, buf, 124);                                                  
                                                                                         
            printf("read : %s\n", buf);                                                  
            for(int i = 0; i < n; i++){                                                  
                buf[i] = toupper(buf[i]);                                                
            }                                                                            
            printf("out : %s\n", buf);                                                   
            write(connfd, buf, n);                                                       
            if(!strncmp(buf, "QUIT", 4)){                                                
                close(connfd);//关闭文件描述符                                                           
            }                                                                            
        }                                                                                
    }                                                                                    
    return NULL;                                                                         
} 

int main(){
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t addr_len = sizeof(client_address);
    task_pool* tp = task_init();
    pthread_t threadid[4];

    struct epoll_event ev, clev, event[125];
    int epfd = epoll_create(125);
    if(epfd == -1){
        perror("epoll create failed");
    }

    for(int i = 0; i < 4; i++){
        pthread_create(&threadid[i], NULL, thr, (void *)tp);
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        perror("socket create failed\n");
        exit(1);
    }

    ev.data.fd = server_socket;
    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &ev);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("绑定失败\n");
        exit(1);
    }

    if(listen(server_socket, 10) < 0){
        perror("listen failed\n");
        exit(1);
    }
    printf("waiting connections...\n");
    while(1){
        int n = epoll_wait(epfd, event, 125, -1);
        for(int i = 0; i < n; i++){
            if(event[i].data.fd == server_socket){
                client_socket = accept
                        (server_socket, (struct sockaddr*)&client_address, &addr_len);
                if(client_socket < 0){
                    perror("connect failed...\n");
                    exit(1);
                }
                
                clev.data.fd = client_socket;
                clev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &clev);
                printf("connect success\n");
            }else if(event[i].events & EPOLLIN) {
                int clfd = event[i].data.fd;
                if(clfd < 3) continue;
                task_push(tp, clfd);
            }
        }
    }

    task_free(tp);
    return 0;
}
