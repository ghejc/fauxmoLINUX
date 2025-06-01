#include <string.h>   //strncpy
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>   //ifreq
#include <unistd.h>   //close
#include <arpa/inet.h> //inet_ntoa
 
char *get_mac_addr(const char *iface = "eth0")
{
    int fd;
    struct ifreq ifr;
     
    fd = socket(AF_INET, SOCK_DGRAM, 0);
 
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
 
    ioctl(fd, SIOCGIFHWADDR, &ifr);
 
    close(fd);

    char mac[19]; /* one extra for terminating '\0'; 
    You may want to make it a little larger
    still, just to be sure. */

    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
    (unsigned char)ifr.ifr_hwaddr.sa_data[0],
    (unsigned char)ifr.ifr_hwaddr.sa_data[1],
    (unsigned char)ifr.ifr_hwaddr.sa_data[2],
    (unsigned char)ifr.ifr_hwaddr.sa_data[3],
    (unsigned char)ifr.ifr_hwaddr.sa_data[4],
    (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
     
    return strdup(mac);
}

char *get_ip_addr(const char *iface = "eth0")
{
    int fd;
    struct ifreq ifr;
     
    fd = socket(AF_INET, SOCK_DGRAM, 0);
 
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
 
    ioctl(fd, SIOCGIFADDR, &ifr);
 
    close(fd);

    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}
