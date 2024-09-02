#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include "defines.h"


static Customer customer;


void set_up_broadcast (int port) {
    struct sockaddr_in bc_address;
    int broadcast = 1, reuse_port = 1;

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr("192.168.164.255");

    bind(sock_fd, (struct sockaddr*)&bc_address, sizeof(bc_address));

    customer.bc_info.addr = bc_address;
    customer.bc_info.fd = sock_fd;
}

void set_up_server () {
    int port;

    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    for (port = LOWERBOUND_PORT; port <= UPPERBOUND_PORT; port++) {
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == 0) {
            break;
        }
    }
    
    customer.tcp_info.addr = address;
    customer.tcp_info.fd = server_fd;
    customer.tcp_info.port = port;

    listen(server_fd, 4);
}

int connect_to_server (int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

int accept_client (int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

int send_message_bc (const char* message) {
    int status;
    
    status = sendto(customer.bc_info.fd, message, strlen (message), 0,(struct sockaddr *)&customer.bc_info.addr, sizeof(customer.bc_info.addr));

    return status;
}


void insert_food (Food* head, Food* food) {
    while (head->next_food != NULL) {
        head = head->next_food;
    }

    head->next_food = food;
    food->next_food = NULL;
}

void read_foods_file () {
    int bytes_count;

    FILE *file = fopen("recipes.json", "r");
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);

    bytes_count = fread(buffer, 1, size, file);

    buffer[size] = '\0';

    fclose(file);

    cJSON *json = cJSON_Parse(buffer);

    cJSON *food = json->child;

    customer.foods = (Food*)malloc (sizeof (Food));
    customer.foods->next_food = NULL;

    while (food) {
        Food* food_item = (Food*)malloc (sizeof (Food));
        food_item->name = (char *)malloc (strlen (food->string) * sizeof (char));
        strcpy (food_item->name, food->string);
        
        insert_food (customer.foods, food_item);

        food = food->next;
    }
    
    cJSON_Delete(food);
    free(buffer);
}

void get_name () {
    write (STDIN, NAME_message, NAME_message_SIZE);
    read (STDOUT, customer.name, sizeof (customer.name));

    customer.name[strcspn (customer.name, "\n")] = 0;
}

void timeout_handler (int signum) {

}

void show_restaurants () {
    int client_fd;
    int bytes_count;
    char buffer[BUFFER_SIZE];

    signal (SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);

    sprintf (buffer, "%s-%d", SHOW_RESTAURANT_REQ, customer.tcp_info.port);

    send_message_bc (buffer);

    alarm (1);
    client_fd = accept_client (customer.tcp_info.fd);
    alarm (0);
    
    while (client_fd > 0) {
        bytes_count = recv (client_fd, buffer, BUFFER_SIZE, 0);
        buffer[bytes_count] = '\n';
        write (STDOUT, buffer, bytes_count + 1);

        close (client_fd);

        client_fd = -1;

        alarm (1);
        client_fd = accept_client (customer.tcp_info.fd);
        alarm (0);
    }
}

void show_foods () {
    char buffer[BUFFER_SIZE];
    Food* head = customer.foods->next_food;

    while (head != NULL) {
        sprintf (buffer, "%s\n", head->name);
        write (STDOUT, buffer, strlen (buffer));

        head = head->next_food;
    }
}

void timeout_handler2 (int signum) {
    write (STDOUT, "timeout\n", 8);
}

void order_food () {
    int port;
    int fd;
    int bytes_count;
    char* buffer[BUFFER_SIZE];
    char* food_name[BUFFER_SIZE];

    signal (SIGALRM, timeout_handler2);
    siginterrupt(SIGALRM, 1);

    write (STDOUT, "Enter food name:\n", 17);

    bytes_count = read (STDIN, food_name, BUFFER_SIZE);

    food_name[bytes_count - 1] = '\0';

    write (STDOUT, "Enter port:\n", 12);

    bytes_count = read (STDIN, buffer, BUFFER_SIZE);

    buffer[bytes_count - 1] = '\0';
    port = atoi (buffer);

    sprintf (buffer, "order-%s-%d", food_name, customer.tcp_info.port);

    fd = connect_to_server (port);

    send (fd, buffer, strlen (buffer), 0);
    bytes_count = -1;

    write (STDOUT, "waiting for restaurant response...\n", 35);

    alarm (120);
    fd = accept_client (customer.tcp_info.fd);
    alarm (0);

    memset (buffer, 0, BUFFER_SIZE);

    bytes_count = recv (fd, buffer, BUFFER_SIZE, 0);

    if (bytes_count > 0) {
        buffer[bytes_count - 1] = '\0';
        
        if (!strcmp (buffer, "accepted")) {
            write (STDOUT, "accepted\n", 9);
        }
        else {
            write (STDOUT, "denied\n", 7);
        }
    }

    close (fd);
}

void exacute_command () {
    int bytes_count;
    char buffer[BUFFER_SIZE];

    bytes_count = read (STDIN, buffer, BUFFER_SIZE);

    buffer[bytes_count - 1] = '\0';
    
    if (!strcmp (buffer, SHOW_MENU_COMMAND)) {
        show_foods ();
    }
    else if (!strcmp (buffer, SHOW_RESTAURANTS_COMMAND)) {
        show_restaurants ();
    }
    else if (!strcmp (buffer, ORDER_FOOD_COMMAND)) {
        order_food ();
    }
}

void handle_same_name (int port) {
    int fd;

    fd = connect_to_server (port);

    send (fd, MACH_NAME_message, 3, 0);
}

void handle_bc (const char* message) {
    int port;
    char* word;

    word = strtok (message, DELIMITER);

    if (!strcmp (word, STATUS_message)) {
        word = strtok (NULL, DELIMITER);

        write (STDOUT, word, strlen (word));
    }
    else if (!strcmp (word, NAME_CHECK)) {
        word = strtok (NULL, DELIMITER);
        
        if (!strcmp (word, customer.name)) {
            word = strtok (NULL, DELIMITER);
            port = atoi (word);
            
            if (port != customer.tcp_info.port) {
                handle_same_name (port);
            }
        }
    }
}

void run () {
    int fd_max;
    int bytes_count;
    int i;
    fd_set master;
    fd_set working;
    char buffer[BUFFER_SIZE];

    FD_ZERO (&master);

    fd_max = customer.tcp_info.fd;

    FD_SET (STDIN, &master);
    FD_SET (customer.bc_info.fd, &master);
    FD_SET (customer.tcp_info.fd, &master);

    while (1) {
        working = master;

        select (fd_max + 1, &working, NULL, NULL, NULL);

        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET (i, &working)) {
                if (i == STDIN) {
                    exacute_command ();
                }
                else if (i == customer.bc_info.fd) {
                    bytes_count = recv (i, buffer, BUFFER_SIZE, 0);
                    buffer[bytes_count] = '\0';
                    
                    handle_bc (buffer);
                }
            }
        }
    }
}

void check_name () {
    int fd;
    char buffer[BUFFER_SIZE];

    signal (SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);

    fd = -1;

    while (1) {
        sprintf (buffer, "name-%s-%d", customer.name, customer.tcp_info.port);
        send_message_bc (buffer);
        
        alarm (1);
        fd = accept_client (customer.tcp_info.fd);
        alarm (0);

        if (fd > 0) {
            close (fd);
            write (STDOUT, GET_NAME_AGAIN_message, GET_NAME_AGAIN_message_SIZE);
            get_name ();
        }
        else {
            break;
        }
    }
}

void write_logs () {
    int fd;
    char buffer[BUFFER_SIZE];

    fd = open ("logs.txt", O_APPEND);

    sprintf (buffer, "customer %s logged in\n", customer.name);
    write (fd, buffer, strlen (buffer));

    close (fd);
}

void start_app () {
    get_name ();
    read_foods_file ();
    set_up_broadcast (customer.bc_info.port);
    set_up_server ();

    check_name ();

    write_logs ();
}

int main (int argc, char* argv[]) {
    if (argc < 2) {
        write (1, "error\n", 6);
    }

    customer.bc_info.port = atoi (argv[1]);
    
    start_app ();
    run ();

    return 0;
}