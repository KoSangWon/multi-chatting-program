//
//  client.c
//  client
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
#define MAX_CLIENT 100

void *handle_client(void *arg);
void send_msg(char *msg, int len);

int client_cnt = 0;
int client_socks[MAX_CLIENT];
pthread_mutex_t mutx;

int main(int argc, const char * argv[]) {
    int client_sock = 0;
    int server_sock = 0;
    unsigned int client_addr_size;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    pthread_t t_id;
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    pthread_mutex_init(&mutx, NULL);
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(-1 == bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr))){
        printf("bind() 실행 에러\n");
        exit(1);
    }
    
    if(-1 == listen(server_sock, 5)){
        printf("listen() 실행 실패");
        exit(1);
    }
    
    while(1){
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        
        pthread_mutex_lock(&mutx);
        client_socks[client_cnt++] = client_sock;
        pthread_mutex_unlock(&mutx);
        
        pthread_create(&t_id, NULL, handle_client, (void*)&client_sock);
        pthread_detach(t_id);
        printf("Connected client IP: %s \n", inet_ntoa(client_addr.sin_addr));
    }
    close(server_sock);
    return 0;
}



void *handle_client(void *arg){
    int client_sock = *((int*)arg);
    int str_len = 0;
    int i = 0;
    char msg[BUFF_SIZE];
    
    while((str_len = read(client_sock, msg, sizeof(msg))) != 0)
        send_msg(msg, str_len);
    
    pthread_mutex_lock(&mutx);
    for(i = 0; i < client_cnt; i++){
        if(client_sock == client_socks[i]){
            while(i++ < client_cnt-1)
                client_socks[i] = client_socks[i+1];
            break;
        }
    }
    client_cnt--;
    pthread_mutex_unlock(&mutx);
    close(client_sock);
    return NULL;
}

void send_msg(char *msg, int len){
    int i;
    pthread_mutex_lock(&mutx);
    for(i = 0; i < client_cnt; i++)
        write(client_socks[i], msg, len);
    pthread_mutex_unlock(&mutx);
}
