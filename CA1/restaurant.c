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


static Restaurant res;


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

    res.bc_info.addr = bc_address;
    res.bc_info.fd = sock_fd;
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
    
    res.tcp_info.addr = address;
    res.tcp_info.fd = server_fd;
    res.tcp_info.port = port;

    listen(server_fd, 4);
}

int connect_to_server (int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
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

void receive_broadcast () {
    int bytes_count;
    int port;
    int fd;
    char buffer[BUFFER_SIZE];
    char* word;

    bytes_count = recv (res.bc_info.fd, buffer, BUFFER_SIZE, 0);
    buffer[bytes_count - 1] = '\0';

    word = strtok (buffer, "-");

    if (!strcmp (word, SHOW_RESTAURANT_REQ)) {
        word = strtok (NULL, "-");
        port = atoi (word);
        fd = connect_to_server (port);
        sprintf (buffer, "%s %d\0", res.name, res.tcp_info.port);
        send (fd, buffer, strlen (buffer), 0);
        close (fd);
    }
    else if (!strcmp (word, NAME_CHECK)) {
        word = strtok (NULL, ":");

        if (!strcmp (word, res.name)) {
            word = strtok (NULL, ":");
            port = atoi (word);

            fd = connect_to_server (port);

            send (fd, MACH_NAME_message, MACH_NAME_SIZE, 0);
        }
    }
}

int send_message_bc (const char* message) {
    int status;

    status = sendto(res.bc_info.fd, message, strlen (message), 0,(struct sockaddr *)&res.bc_info.addr, sizeof(res.bc_info.addr));

    return status;
}


void insert_food (Food* head, Food* food) {
    while (head->next_food != NULL) {
        head = head->next_food;
    }

    head->next_food = food;
    food->next_food = NULL;
}

void insert_supply (Supply* head, Supply* supply) {
    while (head->next_supply != NULL) {
        head = head->next_supply;
    }

    head->next_supply = supply;
    supply->next_supply = NULL;
}

int find_supply (Supply* head, const char* name) {
    while (head != NULL) {
        if (!strcmp (head->name, name)) {
            return 1;
        }

        head = head->next_supply;
    }

    return 0;
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

    res.foods = (Food*)malloc (sizeof (Food));
    res.foods->next_food = NULL;

    res.supplies = (Supply*)malloc (sizeof (Supply));
    res.supplies->next_supply = NULL;

    while (food) {
        Food* food_item = (Food*)malloc (sizeof (Food));
        food_item->name = (char *)malloc (strlen (food->string) * sizeof (char));
        strcpy (food_item->name, food->string);

        food_item->supplies = (Supply*)malloc (sizeof (Supply));

        food_item->supplies->next_supply = NULL;

        cJSON *supply = food->child;
        
        while (supply) {
            if (find_supply (res.supplies->next_supply, supply->string) == 0) {
                Supply* supply_item = (Supply*)malloc (sizeof (Supply));
                supply_item->name = (char *)malloc (strlen (supply->string) * sizeof (char));
                strcpy (supply_item->name, supply->string);
                supply_item->amount = 0;
                
                insert_supply (res.supplies, supply_item);
            }

            Supply* supply_item = (Supply*)malloc (sizeof (Supply));
            supply_item->name = (char *)malloc (strlen (supply->string) * sizeof (char));
            strcpy (supply_item->name, supply->string);
            supply_item->amount = supply->valueint;

            insert_supply (food_item->supplies, supply_item);

            supply = supply->next;

        }
        
        insert_food (res.foods, food_item);

        food = food->next;
    }
    
    cJSON_Delete(food);
    free(buffer);
}

void close_restaurant () {
    char buffer[BUFFER_SIZE];

    sprintf (buffer, "status-%s is closed\n", res.name);
    send_message_bc (buffer);
    res.status = CLOSED;

    write_logs (3);
}

void open_restaurant () {
    char buffer[BUFFER_SIZE];

    sprintf (buffer, "status-%s is open\n", res.name);
    send_message_bc (buffer);
    res.status = OPEN;

    write_logs (2);
}

void show_supplies () {
    int i;
    char buffer[BUFFER_SIZE];
    Supply* head = res.supplies->next_supply;

    write (STDOUT, INGREDIENTS_message, INGREDIENTS_message_SIZE);

    while (head != NULL) {
        sprintf (buffer, "%s %d\n", head->name, head->amount);
        write (STDOUT, buffer, strlen (buffer));

        head = head->next_supply;
    }
}

void show_recipes () {
    char buffer[BUFFER_SIZE];
    Food* food_head;
    Supply* supply_head;
    int i;

    food_head = res.foods->next_food;
    i = 1;

    while (food_head != NULL) {
        sprintf (buffer, "%d- %s:\n", i, food_head->name);
        write (STDOUT, buffer, strlen (buffer));

        supply_head = food_head->supplies->next_supply;

        while (supply_head != NULL) {
            sprintf (buffer, "  %s : %d\n", supply_head->name, supply_head->amount);
            write (STDOUT, buffer, strlen (buffer));

            supply_head = supply_head->next_supply;
        }

        i = i + 1;
        food_head = food_head->next_food;
    }
}

void show_reqs () {
    char buffer[BUFFER_SIZE];
    Req* head;

    head = res.reqs->next_req;

    if (head == NULL) {
        write (STDOUT, "empty\n", 6);
    }

    while (head != NULL) {
        sprintf (buffer, "%s %d\n", head->food_name, head->port);
        write (STDOUT, buffer, strlen (buffer));

        head = head->next_req;
    }
}

void remove_req (int port) {
    Req* req = res.reqs;
    Req* found_req;

    while (req->next_req != NULL) {
        if (req->next_req->port == port) {
            found_req = req->next_req;
            req->next_req = req->next_req->next_req;
            free (found_req);
            break;
        }
    }
}

void accept_req () {
    int port;
    int fd;
    int bytes_count;
    char buffer[BUFFER_SIZE];
    char b[BUFFER_SIZE];

    write (STDOUT, "Enter port:\n", 12);
    bytes_count = read (STDIN, buffer, BUFFER_SIZE);

    buffer[bytes_count - 1] = '\0';

    port = atoi (buffer);

    fd = connect_to_server (port);
    
    send (fd, "accepted", 8, 0);
    close (fd);

    remove_req (port);
}

void deny_req () {
    int port;
    int fd;
    int bytes_count;
    char buffer[BUFFER_SIZE];

    write (STDOUT, "Enter port:\n", 12);
    bytes_count = read (STDIN, buffer, BUFFER_SIZE);

    buffer[bytes_count - 1] = '\0';

    port = atoi (buffer);

    fd = connect_to_server (port);
    send (fd, "denied", 8, 0);
    close (fd);

    remove_req (port);
}

void exacute_command () {
    int bytes_count;
    char buffer[BUFFER_SIZE];

    bytes_count = read (STDIN, buffer, BUFFER_SIZE);

    buffer[bytes_count - 1] = '\0';
    
    if (!strcmp (buffer, BREAK_COMMAND) && res.status == OPEN) {
        close_restaurant ();
    }
    else if (!strcmp (buffer, START_COMMAND)) {
        open_restaurant ();
    }
    else if (!strcmp (buffer, SHOW_INGREDIENTS_COMMAND) && res.status == OPEN) {
        show_supplies ();
    }
    else if (!strcmp (buffer, SHOW_RECIPES_COMMAND) && res.status == OPEN) {
        show_recipes ();
    }
    else if (!strcmp (buffer, SHOW_REQS_COMMAND) && res.status == OPEN) {
        show_reqs ();
    }
    else if (!strcmp (buffer, ACCEPT_COMMAND) && res.status == OPEN) {
        accept_req ();
    }
    else if (!strcmp (buffer, DENY_COMMAND) && res.status == OPEN) {
        deny_req ();
    }
}

void handle_same_name (int port) {
    int fd;

    fd = connect_to_server (port);

    send (fd, MACH_NAME_message, 3, 0);
}

void handle_bc (const char* message) {
    int port;
    int fd;
    char* word;
    char buffer[BUFFER_SIZE];

    word = strtok (message, DELIMITER);

    if (!strcmp (word, NAME_CHECK)) {
        word = strtok (NULL, DELIMITER);
        
        if (!strcmp (word, res.name)) {
            word = strtok (NULL, DELIMITER);
            port = atoi (word);
            
            if (port != res.tcp_info.port) {
                handle_same_name (port);
            }
        }
    }
    else if (!strcmp (word, SHOW_RESTAURANT_REQ) && res.status == OPEN) {
        word = strtok (NULL, DELIMITER);
        port = atoi (word);

        sprintf (buffer, "%s %d", res.name, res.tcp_info.port);
        
        fd = connect_to_server (port);
        send (fd, buffer, strlen (buffer), 0);
    }
}

void insert_req (Req* head, Req* req) {
    while (head->next_req != NULL) {
        head = head->next_req;
    }

    head->next_req = req;
    req->next_req = NULL;
}

void handle_tcp (const char* message) {
    int port;
    char* word;
    char buffer[BUFFER_SIZE];

    word = strtok (message, DELIMITER);

    if (!strcmp (word, ORDER_FOOD_REQ)) {
        word = strtok (NULL, DELIMITER);
        sprintf (buffer, "%s", word);

        word = strtok (NULL, DELIMITER);
        port = atoi (word);

        Req* req = (Req *)malloc (sizeof (Req));
        req->food_name = (char *)malloc (strlen (buffer) * sizeof (char));
        strcpy (req->food_name, buffer);
        req->port = port;

        insert_req (res.reqs, req);

        write (STDOUT, "new request\n", 12);
    }
}

void run () {
    int fd_max;
    int new_socket;
    int bytes_count;
    int i;
    char buffer[BUFFER_SIZE];
    fd_set master;
    fd_set working;

    FD_ZERO (&master);

    fd_max = res.tcp_info.fd;

    FD_SET (STDIN, &master);
    FD_SET (res.bc_info.fd, &master);
    FD_SET (res.tcp_info.fd, &master);

    while (1) {
        working = master;

        select (fd_max + 1, &working, NULL, NULL, NULL);

        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET (i, &working)) {
                if (i == STDIN) {
                    exacute_command ();
                }
                else if (i == res.tcp_info.fd) {
                    new_socket = accept_client (res.tcp_info.fd);
                    FD_SET (new_socket, &master);

                    if (fd_max < new_socket) {
                        fd_max = new_socket;
                    }
                }
                else if (i == res.bc_info.fd) {
                    bytes_count = recv (i, buffer, BUFFER_SIZE, 0);
                    buffer[bytes_count] = '\0';
                    
                    handle_bc (buffer);
                }
                else {
                    bytes_count = recv (i, buffer, BUFFER_SIZE, 0);

                    if (bytes_count == 0) {
                        close (i);
                        FD_CLR (i, &master);
                    }
                    else {
                        buffer[bytes_count] = '\0';

                        handle_tcp (buffer);
                    }
                }
            }
        }
    }
}

void get_name () {
    write (STDOUT, NAME_message, NAME_message_SIZE);
    read (0, res.name, sizeof (res.name));

    res.name[strcspn (res.name, "\n")] = 0;
}

void timeout_handler (int signum) {

}

void check_name () {
    int fd;
    char buffer[BUFFER_SIZE];

    signal (SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);

    fd = -1;

    while (1) {
        sprintf (buffer, "name-%s-%d", res.name, res.tcp_info.port);
        send_message_bc (buffer);
        
        alarm (1);
        fd = accept_client (res.tcp_info.fd);
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

void write_logs (int status) {
    int fd;
    char buffer[BUFFER_SIZE];

    fd = open ("logs.txt", O_APPEND);

    if (status == 1) {
        sprintf (buffer, "restaurant %s logged in\n", res.name);
        write (fd, buffer, strlen (buffer));
    }
    else if (status == 2) {
        sprintf (buffer, "restaurant %s is open\n", res.name);
        write (fd, buffer, strlen (buffer));
    }
    else if (status == 3) {
        sprintf (buffer, "restaurant %s is closed\n", res.name);
        write (fd, buffer, strlen (buffer));
    }

    close (fd);
}

void start_app () {
    char buffer[BUFFER_SIZE];

    read_foods_file ();
    
    get_name ();

    set_up_broadcast (res.bc_info.port);
    set_up_server ();

    check_name ();

    write_logs (1);

    open_restaurant ();

    res.reqs = (Req *)malloc (sizeof (Req));
    res.reqs->next_req = NULL;
}

int main (int argc, char* argv[]) {
    if (argc < 2) {
        write (1, "error\n", 6);
    }

    res.bc_info.port = atoi (argv[1]);
    res.status = OPEN;

    start_app ();
    run ();

    return 0;
}