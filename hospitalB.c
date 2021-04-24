/*
 * Hospital B
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// function that implements Dijkstra's single source shortest path algorithm for a graph represented using adjacency matrix representation
// reference: https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
float * spt(float graph[100][100], int src) {
    static float dist[100]; // output
    bool sptSet[100];
    for (int i = 0; i < 100; i++) {
        dist[i] = FLT_MAX, sptSet[i] = false;
    }
    dist[src] = 0;
    // find shortest path for all vertices
    for (int count = 0; count < 99; count++) {
        float min = FLT_MAX;
        int u;
        for (int v = 0; v < 100; v++) {
            if (sptSet[v] == false && dist[v] <= min) {
                min = dist[v], u = v;
            }
        }
        sptSet[u] = true;
        for (int v = 0; v < 100; v++) {
            if (!sptSet[v] && graph[u][v] && dist[u] != FLT_MAX && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
    return dist;
}

int main(int argc, char *argv[]){
    printf("Hospital B is up and running using UDP on port 31040\n");
    // read in map
    float map[100][100]; // adjacency matrix
    int reindex[100]; // for location# reindexing
    float *distance; // spt array pointer
    for (int i = 0; i < 100; i++) {
        reindex[i] = -1;
    }
    FILE *mapptr;
    mapptr = fopen("./map.txt", "r");
    while(!feof(mapptr)) {
        // i,j: reindexed location#
        int i;
        int j;
        // a,b: original location#
        int a;
        int b;
        float d; // distance
        fscanf (mapptr, "%d", &a);
        fscanf (mapptr, "%d", &b);
        fscanf (mapptr, "%f", &d);
        for (int k = 0; k < 100; k++) {
            if (reindex[k] == a) {
                i = k;
                break;
            }
            else if (reindex[k] == -1) {
                reindex[k] = a;
                i = k;
                break;
            }
        }
        for (int k = 0; k < 100; k++) {
            if (reindex[k] == b) {
                j = k;
                break;
            }
            else if (reindex[k] == -1) {
                reindex[k] = b;
                j = k;
                break;
            }
        }
        map[i][j] = d;
        map[j][i] = d;
    }
    int orig_loc = atoi(argv[1]); // hospital B location#
    int rein_loc; // reindexed B location#
    for (int k = 0; k < 100; k++) {
        if (reindex[k] == orig_loc) {
            rein_loc = k;
            break;
        }
    }
    distance = spt(map, rein_loc);

    // initialize capacity and occupancy
    int capacity = atoi(argv[2]);
    int occupancy = atoi(argv[3]);
    int info[2];
    info[0] = capacity;
    info[1] = occupancy;
    printf("Hospital B has total capacity ​%d​ and initial occupancy ​%d\n", capacity, occupancy);

    //create UDP socket
    int b_socket;
    b_socket = socket(PF_INET, SOCK_DGRAM, 0);

    // define address for UDP socket
    struct sockaddr_in b_address;
    b_address.sin_family = AF_INET;
    b_address.sin_port = htons(31040);
    b_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind UDP socket to address
    bind(b_socket, (struct sockaddr*) &b_address, sizeof(b_address));

    // send initial info to scheduler
    struct sockaddr_in scheduler_address; // address of scheduler
    scheduler_address.sin_family = AF_INET;
    scheduler_address.sin_port = htons(33040);
    scheduler_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(scheduler_address);
    sendto(b_socket, &info, sizeof(info), 0, (struct sockaddr*) &scheduler_address, len);

    // main loop
    while (1) {
        float score = 0; // final score
        float dist; // shortest distance
        int patient_loc;
        int rein_pat_loc = -1; // reindexed  patient location#
        int update; // to dertermine whether it needs to update availability
        int temp;
        recvfrom(b_socket, &temp, sizeof(temp), 0, (struct sockaddr*) &scheduler_address, &len);
        patient_loc = ntohl(temp);
        printf("Hospital B has received input from client at location %d\n", patient_loc);
        for (int k = 0; k < 100; k++) {
            if (reindex[k] == patient_loc) {
                rein_pat_loc = k;
                break;
            }
        }
        if (rein_pat_loc == -1) {
            printf("Hospital B does not have the location %d​ in map\n", patient_loc);
        }
        float availability = ((float)capacity - (float)occupancy) / (float)capacity;
        dist = distance[rein_pat_loc];
        if (availability < 0 || availability >1) {
            printf("Hospital B has capacity = ​%d​, occupation = ​%d, availability = None\n", capacity, occupancy);
        }
        else {
            printf("Hospital B has capacity = ​%d​, occupation = ​%d, availability = %f\n", capacity, occupancy, availability);
        }
        sendto(b_socket, &dist, sizeof(dist), 0, (struct sockaddr*) &scheduler_address, len);
        if (rein_pat_loc == -1) {
            sendto(b_socket, &score, sizeof(score), 0, (struct sockaddr*) &scheduler_address, len);
            printf("Hospital B has sent \"location not found\" to the Scheduler\n");
        }
        else if (rein_pat_loc == rein_loc) {
            printf("Hospital B has found the shortest path to client, distance = ​%f\n", dist);
            printf("Hospital B has the score = ​None\n");
            sendto(b_socket, &score, sizeof(score), 0, (struct sockaddr*) &scheduler_address, len);
            printf("Hospital B has sent score = ​%f and distance = ​%f​ to the Scheduler\n", score, dist);
        }
        else {
            if (availability < 0 || availability >1) {
                printf("Hospital B has found the shortest path to client, distance = ​%f\n", dist);
                printf("Hospital B has the score = ​None\n");
                sendto(b_socket, &score, sizeof(score), 0, (struct sockaddr*) &scheduler_address, len);
                printf("Hospital B has sent score = ​%f and distance = ​%f​ to the Scheduler\n", score, dist);
            }
            else {
                score = 1.0 / (dist * (1.1 - availability)); // score calc formula
                printf("Hospital B has found the shortest path to client, distance = ​%f\n", dist);
                printf("Hospital B has the score = %f\n", score);
                sendto(b_socket, &score, sizeof(score), 0, (struct sockaddr*) &scheduler_address, len);
                printf("Hospital B has sent score = ​%f and distance = ​%f​ to the Scheduler\n", score, dist);
            }
        }
        recvfrom(b_socket, &temp, sizeof(temp), 0, (struct sockaddr*) &scheduler_address, &len);
        update = ntohs(temp);
        if (update == 2) {
            occupancy++;
            availability = ((float)capacity - (float)occupancy) / (float)capacity;
            printf("Hospital B has been assigned to a client, occupation is updated to %d​, availability is updated to ​%f\n", occupancy, availability);
        }
    }

    // close the socket
    close(b_socket);

    return 0;
}