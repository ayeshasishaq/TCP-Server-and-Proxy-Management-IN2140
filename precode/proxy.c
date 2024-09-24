/*
 * This is the main program for the proxy, which receives connections for sending and receiving clients
 * both in binary and XML format. Many clients can be connected at the same time. The proxy implements
 * an event loop.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */


#include "xmlfile.h"
#include "connection.h"
#include "record.h"
#include "recordToFormat.h"
#include "recordFromFormat.h"

#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define MAX_CLIENTS 200
#define BUFFER_SIZE 20480 

/* This struct should contain the information that you want
 * keep for one connected client.
 */

struct Client
{ 
    //fd, type, id og next
    int fd;
    char type;
    int id;
    struct Client *next; // bruker lenkeliste til aa haandtere klientene
};

fd_set fd_liste; // making this fd list to be used in remove client
int num_clients = 0;

typedef struct Client Client;

Client *client_list = NULL; 


void usage( char* cmd )
{
    fprintf( stderr, "Usage: %s <port>\n"
                     "       This is the proxy server. It takes as imput the port where it accepts connections\n"
                     "       from \"xmlSender\", \"binSender\" and \"anyReceiver\" applications.\n"
                     "       <port> - a 16-bit integer in host byte order identifying the proxy server's port\n"
                     "\n",
                     cmd );
    exit( -1 );
}

/*
 * This function is called when a new connection is noticed on the server
 * socket.
 * The proxy accepts a new connection and creates the relevant data structures.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */

//denne funksjonen legger til et nytt klient som "hodet" til lenketlista


void add_client_to_list(Client *client)
{
    //setting the next in client as the next "head"
    client->next = client_list;
    client_list = client;
}

int handleNewClient(int server_sock) {
    int ret;
    //Accepting new connection with TCP server
    int client_fd = tcp_accept(server_sock);
    if (client_fd < 0) { // if no client -> error
        perror("Failed to accept new client"); //failed
        return client_fd;
    }

    //Allocating memory for the new client
    Client *newClient = (Client *)calloc(1,sizeof(Client)); // endrer til calloc
    if (newClient == NULL) {
        perror("Failed to allocate memory for new client");
        return client_fd; // burde vsere -1
    }
    newClient->fd = client_fd;
    
    ret = tcp_read(client_fd, &newClient->type, 1); 
    if(ret < 0){
        perror("Return for tcp_read: type");
        return ret;
    }

    ret = tcp_read(client_fd, &newClient->id, 1); // T
    if(ret < 0){
        perror("Return for tcp_read: type");
        return ret;
    }

    newClient->next = NULL;

    // Call add_client_to_list to add the new client to the list as head of the linked list
    add_client_to_list(newClient);
    num_clients++; 
    return client_fd;
}





/*
 * This function is called when a connection is broken by one of the connecting
 * clients. Data structures are clean up and resources that are no longer needed
 * are released.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
//fiks me previous
void removeClient(Client *client) {

    Client *current = client_list->next;
    Client *previous = client_list;

    // checks if list is empty
    if (client_list == NULL) {
        perror("List is empty.\n");
        return;
    }
    
    //checks if the client doesnt exist
    if(client == NULL){
        perror("Client not found.\n");
        return;
    }
    
    //checks if the head of the client is clint, if so put it to the next pointer of head
    if (client_list == client) {
        client_list = client->next;
    } else {
    //  // Going through the whole client list to find the client before the one to remove and setting it to th ecurrents next pointer ( skipping the client)
        while (current != NULL && current != client) {
            previous = current;
            current = current->next;
        }
    // If the client is not found as the last node will be NULL
        if (current == NULL) {
            perror("Client not found.\n");
            return;
        }
    // Set the next pointer of the current node to the client node after the one to remove
        previous->next = current->next;
    }

    FD_CLR(client->fd, &fd_liste);
    tcp_close(client->fd);
    free(client);
    num_clients--; // remove client global variable
    printf("%s\n","Client removed.");
}

/*
 * This function is called when the proxy received enough data from a sending
 * client to create a Record. The 'dest' field of the Record determines the
 * client to which the proxy should send this Record.
 *
 * If no such client is connected to the proxy, the Record is discarded without
 * error. Resources are released as appropriate.
 *
 * If such a client is connected, this functions find the correct socket for
 * sending to that client, and determines if the Record must be converted to
 * XML format or to binary format for sendig to that client.
 *
 * It does then send the converted messages.
 * Finally, this function deletes the Record before returning.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void forwardMessage( Record* msg )
{

    char* buffer;
    int buffer_size = 0;
    
    if (msg == NULL) {
        printf("Message is NULL, nothing to forward.\n");
        return;
    }

    printf("\nReceived message with ID: %d\n", msg->id);

    Client* current = client_list;
    // finding the client with the ID that matches the destination ID in the message.
    while(current != NULL && current->id != msg->dest){
      // printf("%u", msg->id);
        current = current->next;
    }

    //   // If the client is not found- >error
    if(current == NULL){
        printf("Client with id %d not found. Message not sent.\n", msg->dest);
        return;
    }

    //Convert the record from record to client type
    if(current->type == 'X'){
         printf("%s","Type in forward - X: OK");
         buffer = recordToXML(msg, &buffer_size);
    }else if(current->type == 'B'){
          printf("%s","Type in forward - B: OK");
         buffer = recordToBinary(msg, &buffer_size);
    }else{
        perror("Invalid client type");
        return;
    }

    //If conversion fails
    if(buffer == NULL){
        perror("Failed to convert record to client format");
        return;
    }
   // if the conversion is succesful: Send converted record to the client
    if(tcp_write_loop(current->fd, buffer, buffer_size) < 0){
        perror("Failed to send record to client");
    }

    printf("Found client with matching ID: %c. Forwarding message...\n", current->id);
    printf("Message forwarded to client\n");
   //clean-up
    free(buffer);
    deleteRecord(msg);
}

/*
 * This function is called whenever activity is noticed on a connected socket,
 * and that socket is associated with a client. This can be sending client
 * or a receiving client.
 *
 * The calling function finds the Client structure for the socket where acticity
 * has occurred and calls this function.
 *
 * If this function receives data that completes a record, it creates an internal
 * Record data structure on the heap and calls forwardMessage() with this Record.
 *
 * If this function notices that a client has disconnected, it calls removeClient()
 * to release the resources associated with it.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */

void handleClient(Client* client){
    char buffer[BUFFER_SIZE+1];
    int read_bytes = tcp_read(client->fd, buffer, sizeof(buffer)-1); // reading data
    int bytesread;
    int currPos = 0;

    // error-handling
    if(read_bytes < 0){
        perror("Read failed");
        removeClient(client);
        return;
    }
    else if(read_bytes == 0){
        // Client has disconnected
        removeClient(client);
        return;
    }

    // Only null terminate  XML
    if (client->type == 'X') {
        buffer[read_bytes] = '\0';
    }

    // Keep parsing records while there is data
    while (read_bytes > 0) {
        Record* record = NULL;
        if(client->type == 'X'){
            record = XMLtoRecord(&buffer[currPos], read_bytes, &bytesread);
        }else if(client->type == 'B'){
            record = BinaryToRecord(&buffer[currPos], read_bytes, &bytesread);
        }else{
            perror("Invalid client type: neither X or B");
            return;
        }

        // Forward the record to the destination client.
        if(record != NULL) {
            forwardMessage(record);
           // deleteRecord(record); //free the record
        } else {
            printf("\nRecord is NULL, message not forwarded.\n");
            return 0;
        }
        // update position in buffer
        currPos += bytesread;
        read_bytes -= bytesread;
    }

    printf("\nReceived data from client - END\n");
}




int main( int argc, char* argv[] ){
    int port;
    int server_sock;

    if( argc != 2 ){
        usage( argv[0] );
    }

    port = atoi( argv[1] );

    server_sock = tcp_create_and_listen( port );
    if( server_sock < 0 ) exit( -1 );

    /* add your initialization code */
    
    FD_ZERO(&fd_liste); // sending the adress to fd 
    FD_SET(server_sock, &fd_liste); // metjods for setting fd in the position 
    int max_fd = server_sock; // Keep track of the max fd

    /*
     * The following part is the event loop of the proxy. It waits for new connections,
     * new data arriving on existing connection, and events that indicate that a client
     * has disconnected.
     *
     * This function uses handleNewClient() when activity is seen on the server socket
     * and handleClient() when activity is seen on the socket of an existing connection.
     *
     * The loops ends when no clients are connected any more.
     */


    do{
        /* fill in your code */
        fd_set current_fds = fd_liste; // copy of fd

        if (tcp_wait_timeout(&current_fds, max_fd, 60) < 0) // proxy waits 60 sec, can change
        {
            perror("Error on select");
            exit(-1);
        }

        // traverse through fd-s
        for (int i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &current_fds)) //ready to read or write if in here
            {
                if (i == server_sock) // sjekker for connection
                {   // new fd
                    int new_client = handleNewClient(server_sock);
                    if (new_client < 0){
                        return -1;
                    }
                    // add new fd to set
                    FD_SET(new_client, &fd_liste);
                    if (new_client > max_fd){
                        max_fd = new_client;
                    }

                }else{
                    // check if data is received
                // Find the client that match with this fd
                    Client* current = client_list;
                    while(current != NULL && current->fd != i){
                        current = current->next; // find the mathcing one
                    }
                    
                    if(current != NULL){ // if client found then handleclient
                        handleClient(current);
                    }

                }
            }
        }

    }
    while( num_clients > 0); // when all clientes has diconnected

    /* add your cleanup code */

    tcp_close( server_sock ); // close socket

    return 0;
}

