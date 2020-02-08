//
// Created by Anlan Yu on 2/1/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdatomic.h>
#include <time.h>
#include "messageType.h"
#define KEY_RANGE 100000
#define PUT_PERCENT 0.4
#define MAX_ITER 100000

atomic_int put_success;

atomic_int get_success;

atomic_int put_fail;

atomic_int get_fail;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int findnode(int key){
    return key%3;
}

template <typename T>
int Create_request(RequestInfo<T>* info){

    int hash_key = rand()%KEY_RANGE;
    info->hash_key = hash_key;
    int n = findnode(hash_key);
//    int node = findnode(hash_key);
    printf("hash key generated: %d\n", hash_key);

    //fulfill request->operation_type;
    if(rand()%100 < PUT_PERCENT*100) {
        OperationType operation_type = PUT;
        info->operation_type = operation_type;
        printf("hash put\n");
    }else {
        OperationType operation_type = GET;
        info->operation_type = operation_type;
        printf("hash get\n");
    }

    if(info->operation_type == PUT) {
        T value = (T) rand();
        info->hash_value = value;
        printf("hash value: %d\n", value);
    }else {
        info->hash_value = NULL;
    }
    return n;
}

template <typename T>
void Receive_analyse(ResponseInfo<T>* respond){
    if (respond->operation_type == PUT){
        if (respond->status) {
            put_success++;
        }else {
            put_fail++;
        }
    } else {
        if (respond->status){
            get_success++;
        }else {
            get_fail++;
        }
    }

//    if (respond->status == 2){
//
//    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int sockfd, portno, n, node;
    int iter = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    RequestInfo<int> info;
    ResponseInfo<int> respond;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = 45454;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    while(iter < MAX_ITER){
        sleep(1);

        node = Create_request(&info);

        printf("/////////////////////////iter %d ///////////////////////////\n", iter);
        printf("operation_type is %ld, hash_key is %d\n",info.operation_type,info.hash_key);
        n = write(sockfd, &info, sizeof(info));
        if (n < 0)
            error("ERROR writing to socket");

        bzero(buffer,256);
        n = recv(sockfd, (void*)&buffer, sizeof(buffer),0);
        if (n < 0)
            error("ERROR reading from socket");
        memcpy((void*)&respond,buffer,sizeof(respond));
        printf("received optype is: %ld, hash_key is %d, status is: %d\n", respond.operation_type, respond.hash_key, respond.status);
        Receive_analyse(&respond);
        printf("get success: %d\n", get_success);
        printf("put success: %d\n", put_success);
        printf("get fail: %d\n", get_fail);
        printf("put fail: %d\n", put_fail);
        iter ++;
    }

    close(sockfd);
    return 0;
}
