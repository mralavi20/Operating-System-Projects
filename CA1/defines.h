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

#define STDIN 0
#define STDOUT 1
#define OPEN 1
#define CLOSED 0
#define BUFFER_SIZE 1024
#define NAME_message "Enter your name: "
#define NAME_message_SIZE 17
#define BREAK_COMMAND "break"
#define START_COMMAND "start working"
#define SHOW_INGREDIENTS_COMMAND "show ingredients"
#define INGREDIENTS_message "ingredients/amount\n"
#define INGREDIENTS_message_SIZE 19
#define SHOW_RECIPES_COMMAND "show recipes"
#define SHOW_RESTAURANT_REQ "info"
#define LOWERBOUND_PORT 1024
#define UPPERBOUND_PORT 65535
#define NAME_CHECK "name"
#define MACH_NAME_message "yes"
#define MACH_NAME_SIZE 3
#define GET_NAME_AGAIN_message "Name is not unique. Try again\n"
#define GET_NAME_AGAIN_message_SIZE 30
#define SHOW_MENU_COMMAND "show menu"
#define DELIMITER "-"
#define STATUS_message "status"
#define SHOW_RESTAURANTS_COMMAND "show restaurants"
#define ORDER_FOOD_COMMAND "order food"
#define ORDER_FOOD_REQ "order"
#define SHOW_REQS_COMMAND "show requests list"
#define ACCEPT_COMMAND "accept"
#define DENY_COMMAND "deny"


typedef struct BC_info {
    int fd;
    int port;
    struct sockaddr_in addr;
} BC_info;

typedef struct TCP_info {
    int fd;
    int port;
    struct sockaddr_in addr;
} TCP_info;

typedef struct Restaurant {
    int status;
    char name[BUFFER_SIZE];
    struct BC_info bc_info;
    struct TCP_info tcp_info;
    struct Food* foods;
    struct Supply* supplies;
    struct Req* reqs;
} Restaurant;

typedef struct Supply {
    int amount;
    char *name;
    struct Supply* next_supply;
    struct Supply* perv_supply;
} Supply;

typedef struct Food {
    char *name;
    struct Supply* supplies;
    struct Food* next_food;
    struct Food* perv_food;
} Food;

typedef struct Customer {
    char name[BUFFER_SIZE];
    struct BC_info bc_info;
    struct TCP_info tcp_info;
    struct Food* foods;
} Customer;

typedef struct Req {
    char* food_name;
    int port;
    struct Req* next_req;
} Req;