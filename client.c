/*
 * Author   : Rajagopalaswamy M 
 * E-Mail   : mrajagopalaswamy@gmail.com 
 */

#include "shareit.h"

struct available_server *head = NULL;

/*
 * Function name    : free_server_list
 * Description      : free the allocated memory for available server list
 * Aruments         : None
 * Return           : None
 */

void free_server_list()
{
    struct available_server *temp = head, *next;
    while(temp != NULL) {
        next = temp->next;
        free(temp);
        temp = next;
    }
}

/*
 * Function name    : get_target_server_ip
 * Description      : Fills the IP address of user chosen server
 * Aruments         : Address of buffer to fill, chosen server index
 * Return           : None
 */
void get_target_server_ip(char *server_ip, int server_num)
{
    struct available_server *temp = head;
    int counter = 0;
    while(temp != NULL) {
        counter++;
        if(counter != server_num) {
            temp = temp->next;
            continue;
        }
        strncpy(server_ip, temp->serv.ip, 32);
    }
}

/*
 * Function name    : recv_file
 * Description      : Receives the file metadata, creates file, writes the data shared from server.
 * Aruments         : socket file descriptor of connected server
 * Return           : None
 */
void recv_file(int sock_fd)
{
    int bytes_received = 0;
    char buf[1024] = {0};
    struct file_info file;
    int total_recv_bytes = 0;

    recv(sock_fd, &file, sizeof(file), 0);

    int fd = open(file.name, O_WRONLY | O_CREAT | O_TRUNC, file.perm & 0777);
    if(fd == -1) {
        printf("opening this file %s\n", file.name);
        perror("open failed ");
        return;
    }

    while((bytes_received = recv(sock_fd, buf, sizeof(buf), 0)) > 0) {
        write(fd, buf, bytes_received);

        total_recv_bytes += bytes_received;
        float percent = (total_recv_bytes * 100.0) / file.size;
        printf("\r%s\t\t\t\t%.2f%%", file.name, percent);
        fflush(stdout);

        if(total_recv_bytes == file.size)
            break;
    }
    printf("\n");
    close(fd);
}

/*
 * Function name    : recv_file_from_server
 * Description      : creates socket to the chosen server to receive file
 * Aruments         : User chosen server index
 * Return           : None
 */
void recv_file_from_server(int server_num) 
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    
    char server_ip[32] = {0};
    get_target_server_ip(server_ip, server_num);
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(SHAREIT_MAIN_PORT);
    addr.sin_family = AF_INET;

    printf("Sending connect to ipaddr : %s\n", server_ip);
    if( connect(sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("Connect failed");
        close(sock_fd);
        return;
    }
    recv_file(sock_fd);
    close(sock_fd);
}

/*
 * Function name    : print_list
 * Description      : Prints the available shareIT servers in LAN
 * Aruments         : None
 * Return           : None
 */
void print_list()
{
    struct available_server *temp = head;
    int i = 1;
    printf("\n\nBelow is the all available servers in network : \n");
    while(temp != NULL) {
        printf("%d) %s - %s\n", i, temp->serv.ip, temp->serv.hostname);
        temp = temp->next;
        i++;
    }
}

/*
 * Function name    : insert_server_entry
 * Description      : Inserts the discovery response from server to the client's available server list
 * Aruments         : Discovery response received over socket from servers
 * Return           : None
 */
void insert_server_entry(struct discovery_response resp)
{
    struct available_server *temp = NULL;
    temp = malloc(sizeof(struct available_server));

    strncpy(temp->serv.hostname, resp.hostname, HOST_NAME_MAX);
    strncpy(temp->serv.ip, resp.ip, 32);
    temp->next = NULL;

    if(head == NULL) {
        head = temp;
    }
    else {
        struct available_server *prev = head;
        while(prev->next != NULL)
            prev = prev->next;
        prev->next = temp;
    }
    printf("Added %s in available server list\n", temp->serv.ip);
}

/*
 * Function name    : send_discovery_message
 * Description      : It sends shareIt discovery message, and captures from all the shareIt servers 
 *                    in LAN, uses port 6776
 * Aruments         : None
 * Return           : None
 */
void send_discovery_message()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    char nw_broadaddr[32];
    int bytes = 0;
    int total_recv_bytes = 0;
    struct discovery_response resp;
    struct sockaddr_in serv_addr;

    if(sock_fd == -1) {
        perror("Socket init error: ");
        exit(FAILURE);
    }

    get_new_broadcast_addr(nw_broadaddr);
    if(strlen(nw_broadaddr) <= 0 ) 
        goto cleanup_exit;

    printf("My Broadcast address =  %s\n", nw_broadaddr);

    serv_addr.sin_addr.s_addr = inet_addr(nw_broadaddr);
    serv_addr.sin_port = htons(SHAREIT_DISCOVERY_PORT);
    serv_addr.sin_family = AF_INET;

    printf("Sending Discovery broadcast message\n");
    int broadcastEnable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt(SO_BROADCAST) failed");
        goto cleanup_exit;
    }
    if(sendto(sock_fd, discovery_msg, strlen(discovery_msg), 0, (struct sockaddr*)&serv_addr, 
              sizeof(serv_addr)) < 0) {
        perror("Sendto failed: ");
        goto cleanup_exit;
    }

    printf("Waiting for server to send Discovery response \n");
    struct timeval tv = { .tv_sec = DISCOVERY_TIMEOUT};
    if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ) {
        perror("setsockopt(SO_RCVTIMEO) failed");
        goto cleanup_exit;
    }

    long start = time(NULL);
    long now = start;

    while((now - start) < DISCOVERY_TIMEOUT) {
        memset(&resp, 0, sizeof(resp));
        socklen_t addr_len = sizeof(serv_addr);
        bytes = recvfrom(sock_fd, &resp, sizeof(resp), 0, (struct sockaddr*)&serv_addr, &addr_len);
        printf("bytes received = %d\n", bytes);
        
        total_recv_bytes += bytes;
        if(bytes  < 0 && total_recv_bytes <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            perror("revfrom timeout");
            goto cleanup_exit;
        }
        if(bytes > 0){
            printf("From server : %s - %s\n", resp.ip, resp.hostname);
            insert_server_entry(resp);
        }
        now = time(NULL);
    }
    printf("That's it timeout \n\n");
    close(sock_fd);
    return;
cleanup_exit:
    printf("Failed at %s()\n", __func__);
    close(sock_fd);
    exit(FAILURE);
}
