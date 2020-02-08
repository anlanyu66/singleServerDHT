//
// Created by Anlan Yu on 2/8/20.
//
/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include "messageType.h"
#define LOCK_NUM 100


template <typename T>
std::map<int,T> hash_table;
std::timed_mutex lock[LOCK_NUM];
void dostuff(int); /* function prototype */
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

template <typename T>
void get(int k, T* value, int* get_status){
    *get_status = 1;
//    lock[k%LOCK_NUM].lock();
    auto it = hash_table<T>.find(k);
    if(it == hash_table<T>.end()) {
        *get_status = 0;
    }
    else {
        memcpy(value, &(hash_table<T>[k]), sizeof(T));
    }
//    lock[k%LOCK_NUM].unlock();
}

template <typename T>
void put(int k, T* value, int* put_status){
    *put_status = 1;
//    lock[k%LOCK_NUM].lock();
    auto it = hash_table<T>.find(k);
    if(it == hash_table<T>.end()){
        memcpy(&(hash_table<T>[k]), value, sizeof(T));
    }else {
        *put_status = 0;
    }
//    lock[k%LOCK_NUM].unlock();
}

template <typename T>
void Server_SendRespond(int sock, RequestInfo<T>* message){
    int n_s;
    T value;
    int get_status;
    int put_status;
    ResponseInfo<T> response;

    int k = message->hash_key;
    response.operation_type = message->operation_type;
    response.hash_key = message->hash_key;
//    if(lock[k%LOCK_NUM].try_lock()){
    if(message->operation_type == PUT){

        put(k, &message->hash_value, &put_status);
        response.status = put_status;

        // put_status = 1 success
        if(put_status){
            memcpy(&response.hash_value, &message->hash_value, sizeof(T));
        }
    }else{
        get(k, &value, &get_status);
        response.status = get_status;

        // get_status = 1 success
        if(get_status){
            memcpy(& response.hash_value, &value, sizeof(T));
        }
    }
//    }else{
//        response.status = 2; // if didn't get lock, then status is 2, resend
//    }
    n_s = send(sock, &response, sizeof(response), 0);
    if (n_s < 0) error("ERROR writing to socket");
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid, n;
    char buffer[256];
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 45454;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,3);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    while(1){

        if (newsockfd < 0)
            error("ERROR on accept");

        bzero(buffer,256);
        RequestInfo<int> request;
        printf("////////////////////////////////////////////\n");
        n = recv(newsockfd, (void*)&buffer, sizeof(buffer),0);
        if (n < 0) error("ERROR reading from socket");

        memcpy((void*)&request,buffer,sizeof(request));
        printf("size of operation_type is %ld, hash_key is %d\n", request.operation_type, request.hash_key);

        Server_SendRespond(newsockfd, &request);

    }

    close(sockfd);
    close(newsockfd);

    return 0;

}

