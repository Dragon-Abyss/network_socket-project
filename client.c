/*
 * Client
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    printf("The client is up and running\n");
    int loc = atoi(argv[1]); // client location#
    int result; // to store assigned hospital

    // create TCP socket
    int client_socket;
    client_socket = socket(PF_INET, SOCK_STREAM, 0);

    // define server address at scheduler
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(34040);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connect to server at scheduler
    connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    // send location# to scheduler
    int temp = htonl(loc);
    send(client_socket, &temp, sizeof(temp), 0);
    printf("The client has sent query to Scheduler using TCP: client location %d\n", loc);

    // receive assigned hospital location# from scheduler
    recv(client_socket, &result, sizeof(result), 0);
    if (result == -1) {
        printf("The client has received results from the Scheduler: assigned to Hospital None\n");
    }
    if (result == 0) {
        printf("The client has received results from the Scheduler: assigned to Hospital A\n");
    }
    if (result == 1) {
        printf("The client has received results from the Scheduler: assigned to Hospital B\n");
    }
    if (result == 2) {
        printf("The client has received results from the Scheduler: assigned to Hospital C\n");
    }

    // close the socket
    close(client_socket);

    return 0;
}