//
//  server.c
//  server
//
//  Created by 고상원 on 2020/06/10.
//  Copyright © 2020 고상원. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024
#define NAME_SIZE 15

void *send_msg(void *arg);
void *recv_msg(void *arg);

char name[NAME_SIZE] = "[DEFUALT]";
char msg[BUFF_SIZE];

int main(int argc, const char * argv[]) {
    int sock;
    
    struct sockaddr_in server_addr;
    pthread_t snd_thread;
    pthread_t rcv_thread;
    void *thread_return;
    if(argc != 4){
        printf("Usage: %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }
    
    sprintf(name, "[%s]", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    if(-1 == connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        printf("connect() 실행 에러\n");
        exit(1);
    }
    
    pthread_create(&snd_thread, NULL, sendmsg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recvmsg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}


void *send_msg(void *arg){
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE + BUFF_SIZE];
    while(1){
        fgets(msg, BUFF_SIZE, stdin);
        if(!strcmp(msg, "q\n")||!strcmp(msg, "Q\n")){
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void *recv_msg(void *arg){
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE + BUFF_SIZE];
    int str_len;
    while(1){
        str_len = read(sock, name_msg, NAME_SIZE+BUFF_SIZE-1);
        if(str_len == -1)
            return (void*) -1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}
