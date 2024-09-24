/*
 * This is a file that implements the operation on TCP sockets that are used by
 * all of the programs used in this assignment.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */

#include "connection.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


//Establish a TCP connection with server
int tcp_connect(char *hostname, int port)
{
    int sock_fd;
    struct sockaddr_in server_addr;

    //Creating a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == 0)
    {
        // Print an error message if the socket creation failed.
        perror("Socket creation failed");
        return -1;
       // exit(EXIT_FAILURE);
    }

    // Speicifies the server's IP address and port number.
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);
    server_addr.sin_port = htons(port);

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
         // Print an error message if the connection failed.
        perror("Connecting failed");
        return -1;
       // exit(EXIT_FAILURE);
    }
    //retunerer socket deskriptor
    return sock_fd;
}

//The function reads up to n bytes from the socket sock and stores them in memory at buffer. 
//And handles error
int tcp_read(int sock, char *buffer, int n)
{
    int bytesRead = read(sock, buffer, n);
    
    // If read returns -1, an error occurred, else 1 -> succesful
    if (bytesRead < 0){   
         perror("Read failed");

    }else if(bytesRead == 0){
        perror("socket has been closed by the other side");
    }
    
    return bytesRead;
}

int tcp_write(int sock, char *buffer, int bytes)
{
    int bytesWritten = write(sock, buffer, bytes);
    
    // If write returns -1, an error occurred
    if (bytesWritten < 0)
    {
        perror("Write failed");
    }
    
    return bytesWritten;
}

/*Write to the given TCP socket sock in a loop until an error occurs.*/
int tcp_write_loop(int sock, char *buffer, int bytes)
{
    int total_written = 0;
    int written;

    while (total_written < bytes)
    {
        written = tcp_write(sock, buffer + total_written, bytes - total_written);
        if (written <= 0)
            return -1;
        total_written += written;
    }

    return total_written;
}

//close TCP socket
void tcp_close(int sock)
{
    close(sock);
}


int tcp_create_and_listen(int port)
{
    int server_sock;
    struct sockaddr_in server_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0); // create socket to listen
    if (server_sock == 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET; // specializes the socket
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // bind to listen
    {
        perror("Binding failed");
        return -1;
    }

    if (listen(server_sock, 3) < 0) 
    {
        perror("Listening failed");
        return -1;
    }

    return server_sock;
}

//// This function accepts a new client connection request
int tcp_accept(int server_sock)
{
    int client_sock;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0)
    {
        perror("Accepting failed");
        return -1;
    }
    return client_sock;
}

// wait for clients
int tcp_wait(fd_set *waiting_set, int wait_end){
    return select(wait_end + 1, waiting_set, NULL, NULL, NULL);
}

// wait for given seconds
int tcp_wait_timeout(fd_set *waiting_set, int wait_end, int seconds){
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    return select(wait_end + 1, waiting_set, NULL, NULL, &timeout);
}

