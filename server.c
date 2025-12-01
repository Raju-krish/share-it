/*
 * Author   : Rajagopalaswamy M 
 * E-Mail   : mrajagopalaswamy@gmail.com 
 */

#include "shareit.h"
#include <fcntl.h>
#include <libgen.h>
#include <string.h>

struct discovery_response resp;
char *failure_disc_msg = "Hey buddy, you are not sending discovery message";


/*
 * Function name    : send_metadata
 * Description      : Sends metadata of file to client
 * Aruments         : Client's socket file descriptor
 * Return           : None
 */
struct file_info send_metadata(int sock_fd, char *base)
{
    struct stat st;
    struct file_info file;

    stat(base, &st);
    strncpy(file.name, base, sizeof(file.name));
    file.size = st.st_size;
    file.perm = st.st_mode;
    file.type = st.st_mode & S_IFMT;
    
    if(file.type != S_IFDIR) {
        unsigned char md5[34] = {0};
        md5_file(file.name, md5);
        md5_to_hex(md5, file.checksum);
    }
    send(sock_fd, &file, sizeof(file), 0);
    return file;
}

void send_file_content(int sock_fd, char *client_ip, char *file_name, struct file_info file)
{
    int bytes_read = 0;
    int bytes_sent = 0;
    int total_sent_bytes = 0;
    char buf[1024] = {0};
    FILE *fp = fopen(file_name, "rb");
    if(fp == NULL) {
        perror("fopen failed: ");
        return;
    }

    while ((bytes_read = fread(buf,1, sizeof(buf), fp)) > 0) {
        bytes_sent = send(sock_fd, buf, bytes_read, 0);
        total_sent_bytes += bytes_sent;
        float percent = (total_sent_bytes * 100.0) / file.size;
        printf("\r%s to %s\t\t\t\t%.2f%%", file.name, client_ip, percent);
        fflush(stdout);
        if(bytes_sent < 0) {
            perror("Send failed:  ");
        }
    }

    printf("\nDone Sending !\n");
    fclose(fp);
}

void send_dir(int sock_fd, char *client_ip, char *path)
{
    DIR *p_dir = opendir(path);
    struct dirent *p_entry;
    char rel_path[512] = {0};
    while((p_entry = readdir(p_dir)) != NULL) {
        if(strncmp(p_entry->d_name, ".", 2) == 0 || strncmp(p_entry->d_name, "..", 3) == 0) {
            continue;
        }
        snprintf(rel_path, sizeof(rel_path), "%s/%s", path, p_entry->d_name);
        struct file_info file = send_metadata(sock_fd, rel_path);
        if(file.type == S_IFDIR) 
            send_dir(sock_fd, client_ip, rel_path);
        else 
            send_file_content(sock_fd, client_ip, rel_path, file);
    }

}
/*
 * Function name    : send_file
 * Description      : Send file to client
 * Aruments         : Client's socket file descriptor
 * Return           : None
 */
void* send_file(void *arg)
{
    struct thread_arg *data = (struct thread_arg*)arg;
    char base[512] = {0};
    snprintf(base, sizeof(base), "%s", basename(fshare_name));

    struct file_info file =  send_metadata(data->sock_fd, base);
    if(file.type == S_IFDIR) { // It's a directory
        printf("Its, a  directory\n");
        send_dir(data->sock_fd, data->client_ip, fshare_name);
    } 
    else {
        send_file_content(data->sock_fd, data->client_ip, fshare_name, file);
    }
    send(data->sock_fd, "STOP_TRANS", strlen("STOP_TRANS"), 0);
    close(data->sock_fd);
    free(data);
    return NULL;
}

/*
 * Function name    : handle_file_share
 * Description      : Send the file over TCP connection for the clients conneting on port 6777
 * Aruments         : None
 * Return           : None
 */
void* handle_file_share(void *arg) 
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(sock_fd == -1)
        perror("Socket init error: ");

    struct sockaddr_in addr, client_addr;
    char client_ip[IP_ADDR_LEN] = {0};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SHAREIT_MAIN_PORT);
    addr.sin_family = AF_INET;

    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        goto cleanup_exit;
    }
    if(bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        goto cleanup_exit;
    }
    if(listen(sock_fd, 3) < 0) {
        perror("Listen fails");
        goto cleanup_exit;
    }

    while(1) {
        socklen_t client_addr_len = sizeof(client_addr);
        int new_sock_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        struct thread_arg *arg = malloc(sizeof(struct thread_arg));
        pthread_t tid;

        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        arg->sock_fd = new_sock_fd;
        strncpy(arg->client_ip, client_ip, IP_ADDR_LEN);
        printf("\nReceived file request from : %s\n", client_ip);
        pthread_create(&tid, NULL, send_file, (void*)arg);
        pthread_detach(tid);
    }
    return NULL;
cleanup_exit:
    close(sock_fd);
    exit(0);
}

/*
 * Function name    : handle_discovery_message
 * Description      : 
 * Aruments         : Address of buffer to fill, chosen server index
 * Return           : None
 */
void* handle_discovery_message(void *arg)
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    char buf[1024] = {0};

    if(sock_fd == -1) {
        perror("Socket init error: ");
        exit(FAILURE);
    }
    gethostname(resp.hostname, sizeof(resp.hostname));
    get_iface_ipaddr(resp.ip);

    struct sockaddr_in addr, client_addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SHAREIT_DISCOVERY_PORT);
    addr.sin_family = AF_INET;

    if(bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Socket bind failed: ");
        goto cleanup_exit;
    }

    while(1) {
        socklen_t len = sizeof(client_addr);
        memset(buf, 0, sizeof(buf));
        printf("Waiting for client's Discovery message: \n");
        recvfrom(sock_fd, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &len);

        if(strncmp(buf, discovery_msg, strlen(discovery_msg)) == 0) {
            printf("Its discovery message\n");
            if(sendto(sock_fd, &resp, sizeof(resp), 0, (struct sockaddr*)&client_addr, len) < 0)
                perror("send_to failed: ");
        } 
        else 
            sendto(sock_fd, failure_disc_msg, strlen(failure_disc_msg), 0, (struct sockaddr*)&client_addr, len);
    }

cleanup_exit:
    close(sock_fd);
    exit(FAILURE);
}
