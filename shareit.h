/*
 * Author   : Rajagopalaswamy M 
 * E-Mail   : mrajagopalaswamy@gmail.com 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h> // For basename
#include "md5.h"

#define SHAREIT_DISCOVERY_PORT 6776
#define SHAREIT_MAIN_PORT 6777
#define TOTAL_TIMEOUT_SEC 5
#define DISCOVERY_TIMEOUT 2
#define FAILURE -1
#define SUCCESS 0
#define IP_ADDR_LEN 32

struct discovery_response {
    char hostname[HOST_NAME_MAX];
    char ip[IP_ADDR_LEN];
};

struct available_server {
    struct discovery_response serv;
    struct available_server *next;
};

struct file_info {
    char name[512];
    size_t size;
    int type;
    mode_t perm;
    char checksum[34];
};

struct thread_arg {
    int sock_fd;
    char client_ip[IP_ADDR_LEN];
};

extern const char *discovery_msg;
extern char fshare_name[512];
void server();
void send_discovery_message();
void* handle_discovery_message(void *arg);
void get_new_broadcast_addr(char *nw_broadaddr);
void get_iface_ipaddr(char *nw_ipaddr);
void* handle_file_share(void *arg);
void print_list();
void recv_file_from_server(int server_num);
void free_server_list();
void md5_to_hex(const unsigned char *md5, char *out);
int md5_file(const char *path, unsigned char *md5_out);
