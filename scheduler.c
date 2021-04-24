/*
 * The scheduler
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

int main(int argc, char *argv[]) {
    printf("The Scheduler is up and running\n");
    // create UDP socket for the hospitals
    int hospital_socket;
    hospital_socket = socket(PF_INET, SOCK_DGRAM, 0);

    // define address for UDP socket
    struct sockaddr_in hospital_address;
    hospital_address.sin_family = AF_INET;
    hospital_address.sin_port = htons(33040);
    hospital_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind UDP socket to its address
    bind(hospital_socket, (struct sockaddr*) &hospital_address, sizeof(hospital_address));

    // define addresses for the hospitals
    struct sockaddr_in a_address;
    a_address.sin_family = AF_INET;
    a_address.sin_port = htons(30040);
    a_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t a_len = sizeof(a_address);

    struct sockaddr_in b_address;
    b_address.sin_family = AF_INET;
    b_address.sin_port = htons(31040);
    b_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t b_len = sizeof(b_address);

    struct sockaddr_in c_address;
    c_address.sin_family = AF_INET;
    c_address.sin_port = htons(32040);
    c_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t c_len = sizeof(c_address);

    // receive initial info from the hospitals
    struct sockaddr_in temp_address; // address used to identify each hospital
    int init_temp[2];
    int init_a[2];
    int init_b[2];
    int init_c[2];
    for (int i = 0; i < 3; i++) {
        socklen_t fromlen = sizeof(temp_address);
        recvfrom(hospital_socket, &init_temp, sizeof(init_temp), 0, (struct sockaddr*) &temp_address, &fromlen);
        if (temp_address.sin_port == htons(30040)) {
            init_a[0] = init_temp[0];
            init_a[1] = init_temp[1];
            printf("The Scheduler has received information from Hospital A: total capacity is ​%d and initial occupancy is %d\n", init_a[0], init_a[1]);
        }
        else if (temp_address.sin_port == htons(31040)) {
            init_b[0] = init_temp[0];
            init_b[1] = init_temp[1];
            printf("The Scheduler has received information from Hospital B: total capacity is ​%d and initial occupancy is %d\n", init_b[0], init_b[1]);
        }
        else if (temp_address.sin_port == htons(32040)) {
            init_c[0] = init_temp[0];
            init_c[1] = init_temp[1];
            printf("The Scheduler has received information from Hospital C: total capacity is ​%d and initial occupancy is %d\n", init_c[0], init_c[1]);
        }
    }
 
    // create TCP socket for the client
    int patient_socket;
    patient_socket = socket(PF_INET, SOCK_STREAM, 0);

    // define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(34040);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind TCP socket to address
    bind(patient_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(patient_socket, 5);

    // main loop
    while (1) {
        int child_socket = accept(patient_socket, NULL, NULL);
        int patient_loc; // to store patient location#
        // score for each hospital;
        float score_a = -1;
        float score_b = -1;
        float score_c = -1;
        // distance for each hospital
        float dist_a = -1;
        float dist_b = -1;
        float dist_c = -1;
        recv(child_socket, &patient_loc, sizeof(patient_loc), 0);
        printf("The Scheduler has received client at location %d​ from the client using TCP over port 34040\n", patient_loc);
        
        if (init_a[0] > init_a[1]) {
            sendto(hospital_socket, &patient_loc, sizeof(patient_loc), 0, (struct sockaddr*) &a_address, a_len);
            printf("The Scheduler has sent client location to Hospital A using UDP over port 33040\n");
            recvfrom(hospital_socket, &dist_a, sizeof(dist_a), 0, (struct sockaddr*) &a_address, &a_len);
            recvfrom(hospital_socket, &score_a, sizeof(score_a), 0, (struct sockaddr*) &a_address, &a_len);
            printf("The Scheduler has received map information from Hospital A, the score = ​%f​ and the distance = %f\n", score_a, dist_a);
        }
        if (init_b[0] > init_b[1]) {
            sendto(hospital_socket, &patient_loc, sizeof(patient_loc), 0, (struct sockaddr*) &b_address, b_len);
            printf("The Scheduler has sent client location to Hospital A using UDP over port 33040\n");
            recvfrom(hospital_socket, &dist_b, sizeof(dist_b), 0, (struct sockaddr*) &b_address, &b_len);
            recvfrom(hospital_socket, &score_b, sizeof(score_b), 0, (struct sockaddr*) &b_address, &b_len);
            printf("The Scheduler has received map information from Hospital B, the score = ​%f​ and the distance = %f\n", score_b, dist_b);
        }
        if (init_c[0] > init_c[1]) {
            sendto(hospital_socket, &patient_loc, sizeof(patient_loc), 0, (struct sockaddr*) &c_address, c_len);
            printf("The Scheduler has sent client location to Hospital A using UDP over port 33040\n");
            recvfrom(hospital_socket, &dist_c, sizeof(dist_c), 0, (struct sockaddr*) &c_address, &c_len);
            recvfrom(hospital_socket, &score_c, sizeof(score_c), 0, (struct sockaddr*) &c_address, &c_len);
            printf("The Scheduler has received map information from Hospital C, the score = ​%f​ and the distance = %f\n", score_c, dist_c);
        }

        // reply to client
        int reply = -1; // -1 for None, 0 for A, 1 for B, 2 for C
        if (dist_a == 0 || dist_b == 0 || dist_c == 0) {
            send(child_socket, &reply, sizeof(reply), 0);
        }
        else {
            if (score_a == 0 && score_b == 0 && score_c == 0) {
                send(child_socket, &reply, sizeof(reply), 0);
            }
            // three non-tie cases
            else if (score_a != 0 && score_a > score_b && score_a > score_c) {
                reply = 0;
                send(child_socket, &reply, sizeof(reply), 0);
            }
            else if (score_b != 0 && score_b > score_a && score_b > score_c) {
                reply = 1;
                send(child_socket, &reply, sizeof(reply), 0);
            }
            else if (score_c != 0 && score_c > score_a && score_c > score_b) {
                reply = 2;
                send(child_socket, &reply, sizeof(reply), 0);
            }
            // four tie cases
            else if (score_a != 0 && score_a == score_b && score_a > score_c) {
                if (dist_a < dist_b) {
                    reply = 0;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
                else {
                    reply = 1;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
            }
            else if (score_b != 0 && score_b == score_c && score_b > score_a) {
                if (dist_b < dist_c) {
                    reply = 1;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
                else {
                    reply = 2;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
            }
            else if (score_c != 0 && score_c == score_a && score_c > score_b) {
                if (dist_c < dist_a) {
                    reply = 2;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
                else {
                    reply = 0;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
            }
            else {
                if (dist_a < dist_b && dist_a < dist_c) {
                    reply = 0;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
                else if (dist_b < dist_a && dist_b < dist_c) {
                    reply = 1;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
                else {
                    reply = 2;
                    send(child_socket, &reply, sizeof(reply), 0);
                }
            }
        }
        if (reply == -1) {
            printf("The Scheduler has assigned Hospital ​None to the client\n");
            printf("The Scheduler has sent the result to client using TCP over port 34040\n");
        }
        if (reply == 0) {
            printf("The Scheduler has assigned Hospital A to the client\n");
            printf("The Scheduler has sent the result to client using TCP over port 34040\n");
        }
        if (reply == 1) {
            printf("The Scheduler has assigned Hospital B to the client\n");
            printf("The Scheduler has sent the result to client using TCP over port 34040\n");
        }
        if (reply == 2) {
            printf("The Scheduler has assigned Hospital C to the client\n");
            printf("The Scheduler has sent the result to client using TCP over port 34040\n");
        }
        close(child_socket);

        // reply to hospitals
        int update; // 2 for update, 1 for no update
        int temp;
        if (reply == -1) {
            if (score_a != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &a_address, a_len);
            }
            if (score_b != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &b_address, b_len);
            }
            if (score_c != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &c_address, c_len);
            }
        }
        if (reply == 0) {
            if (score_a != -1) {
                temp = 2;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &a_address, a_len);
                init_a[1]++;
                printf("The Scheduler has sent the result to Hospital ​A using UDP over port 33040\n");
            }
            if (score_b != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &b_address, b_len);
            }
            if (score_c != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &c_address, c_len);
            }
        }
        if (reply == 1) {
            if (score_a != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &a_address, a_len);
            }
            if (score_b != -1) {
                temp = 2;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &b_address, b_len);
                init_b[1]++;
                printf("The Scheduler has sent the result to Hospital ​B using UDP over port 33040\n");
            }
            if (score_c != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &c_address, c_len);
            }
        }
        if (reply == 2) {
            if (score_a != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &a_address, a_len);
            }
            if (score_b != -1) {
                temp = 1;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &b_address, b_len);
            }
            if (score_c != -1) {
                temp = 2;
                update = htons(temp);
                sendto(hospital_socket, &update, sizeof(update), 0, (struct sockaddr*) &c_address, c_len);
                init_c[1]++;
                printf("The Scheduler has sent the result to Hospital ​C using UDP over port 33040\n");
            }
        }
    }

    // close the sockets
    close(hospital_socket);
    close(patient_socket);

    return 0;
}