#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>       // Gia to AFINET
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define  DEFAULT_PROTOCOL  0
#define NUMBER_OF_ORDERS 10

int main(int argc, char *argv[])
{
    int serverFd; 
    int port; 
    int n;
    char buffer[100];

    int orderClient;
    struct sockaddr_in serverINETAddress;
    struct hostent *server;

    if (argc < 3) {
      fprintf(stderr,"--> usage %s hostname port\n", argv[0]);
      exit(0);
    }

    port = atoi(argv[2]);
    
    serverFd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL); 
    if (serverFd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

   
    bzero((char *)&serverINETAddress, sizeof(serverINETAddress)); 
    serverINETAddress.sin_family = AF_INET; 
    bcopy((char *)server->h_addr, (char *)&serverINETAddress.sin_addr.s_addr, server->h_length);
    serverINETAddress.sin_port = htons(port); 




    if (connect(serverFd, (struct sockaddr *)&serverINETAddress, sizeof(serverINETAddress)) < 0){
       perror("ERROR connecting");
       exit(1);
    }


    for (int i = 0; i < NUMBER_OF_ORDERS; i++)
    {
        srand(clock());
        orderClient=rand() % 20;
        
        n = write(serverFd, &orderClient, sizeof(orderClient));
        if (n < 0){
            perror("ERROR writing to socket");
            exit(1);
        }



        bzero(buffer,100);
        n = read(serverFd, buffer, 100);

        if (n < 0){
            perror("ERROR reading from socket");
            exit(1);
        }

        printf("%s\n", buffer);

        sleep(1);

    }
    return 0;
}
