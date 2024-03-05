#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>       // Gia to AFINET 
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


#define NUMBER_OF_PRODUCTS 20
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_ORDERS 10
#define MAX_CHARS_IN_BUFFER 100
#define DEFAULT_PROTOCOL 0

struct product {
    char* description;
    double price;
    int item_count;
    int requests;
    int sold;
    int sold_out;
};

struct eshop_analytics {
    int total_orders;
    int total_successful_orders;
    int total_unsuccessful_orders;
    double total_gain;
};

char* products[] = { "laptop", "mouse", "keyboard", "charger", "screen", "figure", "key_chain",
                     "T-shirt", "hat", "ring", "bag", "wallet", "mask", "poster", "manga",
                     "console", "smartphone", "mug", "lamp", "candle" };

struct product catalog[NUMBER_OF_PRODUCTS];
struct eshop_analytics analytics;

int eshop_pipes[NUMBER_OF_CUSTOMERS][2];
int customers_pipes[NUMBER_OF_CUSTOMERS][2];

void init_catalog();
void init_eshop_analytics();
double random_price();
void statistics();
void customer(int ith_custome);

int main(int argc, char* argv[]) {
    init_catalog();
    init_eshop_analytics();

	int serverFd;   
    int clientFd;   
    socklen_t clientLen;
    struct sockaddr_in serverINETAddress, clientINETAddress; 

    int n;
    int orderClient;
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        if (pipe(customers_pipes[i]) < 0) {
            perror("Error initializing customers pipes!");
            exit(EXIT_FAILURE);
        }

        if (pipe(eshop_pipes[i]) < 0) {
            perror("Error initializing eshop's pipe!");
            exit(EXIT_FAILURE);
        }
    }

    //pid_t pids[NUMBER_OF_CUSTOMERS];
    pid_t pids;

	
    serverFd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL); /* Δημιουργία */
    if (serverFd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }


    int port=4000; 

    serverINETAddress.sin_family = AF_INET; 
    serverINETAddress.sin_addr.s_addr = htonl(INADDR_ANY); 
    serverINETAddress.sin_port = htons(port); 

    
    if (bind(serverFd, (struct sockaddr *)&serverINETAddress, sizeof(serverINETAddress)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    listen(serverFd, 5); 

    while(1)
    {
        clientLen = sizeof( clientINETAddress );

		
        clientFd = accept(serverFd, (struct sockaddr *) &clientINETAddress, &clientLen);
        if (clientFd < 0) {
          perror("ERROR on accept");
          exit(1);
        }
        pids = fork();

	    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
			if (pids < 0) {
				perror("Error creating the customers!");
				exit(EXIT_FAILURE);
			} else if (pids == 0) {
				
				close(serverFd);
				customer(i);
				char buffer[MAX_CHARS_IN_BUFFER];

				for (int i = 0; i < NUMBER_OF_ORDERS; i++) {
					


					n = read( clientFd, &orderClient,  sizeof(orderClient));
					if (n < 0)
					{
						perror("ERROR reading from socket");
						exit(1);
					}


					close(customers_pipes[i][0]);
					write(customers_pipes[i][1], &orderClient, sizeof(orderClient));

					sleep(1);

					close(eshop_pipes[i][1]);
					read(eshop_pipes[i][0], buffer, sizeof(buffer));
					n = write(clientFd, buffer, strlen(buffer)+1);
					 
					if (n < 0)
					{
					   perror("ERROR writing to socket");
					   exit(1);
					}
					printf("Customer: Message from eshop is --> %s", buffer);
				}
			             close(clientFd);
                         exit(EXIT_SUCCESS);


			}
		}


		close(clientFd);
		// Kentrikh Epikoinwnia
		int order;
		char buffer[MAX_CHARS_IN_BUFFER];
		for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
			for (int j = 0; j < NUMBER_OF_ORDERS; j++) {
				close(customers_pipes[i][1]);
				read(customers_pipes[i][0], &order, sizeof(order));

				printf("Eshop: Received Order with number #%d\n", order);
				catalog[order].requests++;

				close(eshop_pipes[i][0]);
				if (catalog[order].item_count > 0) {
					snprintf(buffer, sizeof(buffer), "Found Order #%d with total cost %.2f euros!\n\n",
													  order, catalog[order].price);
					catalog[order].item_count--;
					catalog[order].sold++;
					analytics.total_gain += catalog[order].price;
					analytics.total_successful_orders++;

					write(eshop_pipes[i][1], buffer, sizeof(buffer));
				} else {
					snprintf(buffer, sizeof(buffer), "Your Order #%d Is Not Found!\n\n", order);

					catalog[order].sold_out++;
					analytics.total_unsuccessful_orders++;

					write(eshop_pipes[i][1], buffer, sizeof(buffer));
				}
			}
			sleep(1);
		}

    }
		// Gia thn emfanish twn statistikwn
		int old_fd = dup(STDOUT_FILENO);
		int file = open("eshop_statistics.txt", O_RDWR|O_CREAT, 0644);
		if (file < 0) {
			perror("Error creating the file!");
		}

		dup2(file, STDOUT_FILENO);
		statistics();
		dup2(old_fd, STDOUT_FILENO);

    //return 0;
}

// Synarthsh gia ton pinaka catalog.me ola ta pedia
void init_catalog() {
    for (int i = 0; i < NUMBER_OF_PRODUCTS; i++) {
        catalog[i].description = products[i];
        catalog[i].price = random_price();
        catalog[i].item_count = 2;
        catalog[i].requests = 0;
        catalog[i].sold = 0;
        catalog[i].sold_out = 0;
    }
}

// Synarthsh gia metrhtes kai athroisth
void init_eshop_analytics() {
    analytics.total_orders = 0;
    analytics.total_successful_orders = 0;
    analytics.total_unsuccessful_orders = 0;
    analytics.total_gain = 0;
}

// Synarthsh poy paragei enan tyxaio arithmo apo to 0 ews to 50
double random_price() {
    srand(clock());

    return (double) rand() / (double) (RAND_MAX / 50);
}


// Synarthsh gia ta statistika
void statistics() {
    printf("\nProducts Information\n");
    printf("--------------------\n");
    for (int i = 0; i < NUMBER_OF_PRODUCTS; i++) {
        printf("Description: %s\n", catalog[i].description);
        printf("Requests: %d\n", catalog[i].requests);
        printf("Sold: %d\n", catalog[i].sold);
        printf("Sold Out: %d\n\n", catalog[i].sold_out);
    }

    printf("\nEshop Statistics\n");
    printf("----------------\n");
    printf("Total Orders: %d\n", analytics.total_successful_orders +
                                 analytics.total_unsuccessful_orders);
    printf("Total Successful Orders: %d\n", analytics.total_successful_orders);
    printf("Total Unsuccessful Orders: %d\n", analytics.total_unsuccessful_orders);
    printf("Total Gain: %.2f\n", analytics.total_gain);
}

// Synarthsh oson afora ton pelati
void customer(int ith_customer) {
    char buffer[MAX_CHARS_IN_BUFFER];

    for (int i = 0; i < NUMBER_OF_ORDERS; i++) {
        srand(clock());
        int order = rand() % 20;
        
        close(customers_pipes[ith_customer][0]);
        write(customers_pipes[ith_customer][1], &order, sizeof(order));

        sleep(1);
        
        close(eshop_pipes[ith_customer][1]);
        read(eshop_pipes[ith_customer][0], buffer, sizeof(buffer));

        printf("Customer: Message from eshop is --> %s", buffer);
    }
}