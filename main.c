/*
 * Author   : Rajagopalaswamy M 
 * E-Mail   : mrajagopalaswamy@gmail.com 
 */

#include "shareit.h"

const char *discovery_msg = "Discovery message !";
char fshare_name[512];

void server(char *file_name)
{
    if(file_name == NULL || strlen(file_name) <= 0) {
        printf("Invalid filename\n");
        exit(0);
    }
    strncpy(fshare_name, file_name, sizeof(fshare_name));
    pthread_t disc_thread, fshare_thread;
    pthread_create(&disc_thread, 0 , handle_discovery_message, NULL);
    pthread_create(&fshare_thread, 0 , handle_file_share, NULL);
    pthread_join(disc_thread, NULL);
    pthread_join(fshare_thread, NULL);
}

void client()
{
    int server_num = 0;
    send_discovery_message();
    print_list();
    printf("Enter the choice to get file from : ");
    scanf("%d", &server_num);
    recv_file_from_server(server_num);
    free_server_list();
}

void show_help()
{
    printf("To send a file \t: shareit -s <file>\n");
    printf("To receive from\t: shareit -r\n");
}

int main(int argc, char *argv[])
{
    // int choice = 0;
    // void (*start_app[2])(void) = {server, client};
    if(argc <= 1 ) {
        show_help();
        exit(0);
    };
    if(argc == 2 && strncmp(argv[1], "-s", 3) == 0) {
        show_help();
        exit(0);
    }
    if(argc == 2 && strncmp(argv[1], "-r", 3) == 0) client();
    if(argc > 2 && strncmp(argv[1], "-s", 3) == 0) server(argv[2]);
    // printf("Choose mode : \n");
    // printf("\t[1] Send\n");
    // printf("\t[2] Receive\n");
    //
    // scanf("%d", &choice);
    // start_app[choice-1]();
}
