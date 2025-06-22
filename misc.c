/*
 * Author   : Rajagopalaswamy M 
 * E-Mail   : mrajagopalaswamy@gmail.com 
 */

#include "shareit.h"

/*
 * Function name    : get_iface_ipaddr
 * Description      : Fills the IP-address of wifi interface
 * Aruments         : Address of buffer to fill the IP
 * Return           : None
 */
void get_iface_ipaddr(char *nw_ipaddr)
{
    struct ifaddrs *ifaddr = NULL, *if_ptr = NULL;

    getifaddrs(&ifaddr);
    for(if_ptr = ifaddr; if_ptr != NULL; if_ptr = if_ptr->ifa_next) {
        if(strncmp(if_ptr->ifa_name, "wl", 2) != 0)
            continue;
        if (!(if_ptr->ifa_flags & IFF_UP) || !if_ptr->ifa_addr || if_ptr->ifa_addr->sa_family != AF_INET)
            continue;

        if(if_ptr->ifa_addr != NULL) {
            struct sockaddr_in *addr = (struct sockaddr_in *)if_ptr->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, nw_ipaddr, IP_ADDR_LEN);
        }
    }
    freeifaddrs(ifaddr);
}

/*
 * Function name    : get_new_broadcast_addr
 * Description      : Fills the broadcast ipaddress of wifi interface
 * Aruments         : Address of buffer to fill the broadcast IP
 * Return           : None
 */
void get_new_broadcast_addr(char *nw_broadaddr)
{
    struct ifaddrs *ifaddr = NULL, *if_ptr = NULL;
    getifaddrs(&ifaddr);
    for(if_ptr = ifaddr; if_ptr != NULL; if_ptr = if_ptr->ifa_next) {
        if(strncmp(if_ptr->ifa_name, "wl", 2) != 0)
            continue;
        if (!(if_ptr->ifa_flags & IFF_UP) || (if_ptr->ifa_flags & IFF_LOOPBACK) || \
            !if_ptr->ifa_addr || if_ptr->ifa_addr->sa_family != AF_INET)
            continue;
        if(if_ptr->ifa_flags & IFF_BROADCAST && if_ptr->ifa_broadaddr) {
            printf("ifname : %s\n", if_ptr->ifa_name);
            struct sockaddr_in *addr = (struct sockaddr_in *) if_ptr->ifa_broadaddr;
            inet_ntop(AF_INET, &addr->sin_addr, nw_broadaddr, IP_ADDR_LEN);
        }
    }
    freeifaddrs(ifaddr);
}
