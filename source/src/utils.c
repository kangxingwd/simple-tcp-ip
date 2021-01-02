#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

int print_mac(unsigned char *mac)
{
     if (!mac)
          return -1;
     printf("mac is: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
     return 0;
}

int get_eth_ip(char *eth_name, unsigned char *ip)
{

     int sock_get_ip;
     struct sockaddr_in *sin;
     struct ifreq ifr_ip;

     if ((sock_get_ip = socket(AF_INET, SOCK_STREAM, 0)) == -1)
     {
          printf("socket create failse ... get_eth_ip!/n");
          return -1;
     }

     memset(&ifr_ip, 0, sizeof(ifr_ip));
     strncpy(ifr_ip.ifr_name, eth_name, sizeof(ifr_ip.ifr_name) - 1);

     if (ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0)
     {
          return -1;
     }
     sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;

     if (ip)
          strcpy(ip, inet_ntoa(sin->sin_addr));

     close(sock_get_ip);
     return 0;
}

int get_eth_mac(char *eth_name, unsigned char *mac)
{

     int sock;
     struct sockaddr_in *sin;
     struct ifreq ifr;

     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
     {
          printf("socket create failse ... get_eth_mac!/n");
          return -1;
     }

     memset(&ifr, 0, sizeof(ifr));
     strncpy(ifr.ifr_name, eth_name, sizeof(ifr.ifr_name) - 1);

     if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
     {
          close(sock);
          return -1;
     }

     if (mac)
          memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

     printf("req eth_name = %s\n", eth_name);
     print_mac((unsigned char*)(ifr.ifr_hwaddr.sa_data));

     close(sock);
     return 0;
}

int print_local_ip()
{
     struct ifaddrs *ifAddrStruct = NULL;
     void *tmpAddrPtr = NULL;

     getifaddrs(&ifAddrStruct);

     while (ifAddrStruct != NULL)
     {
          if (ifAddrStruct->ifa_addr->sa_family == AF_INET)
          { // check it is IP4
               // is a valid IP4 Address
               tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
               char addressBuffer[INET_ADDRSTRLEN];
               inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
               printf("%s IP Address %s/n", ifAddrStruct->ifa_name, addressBuffer);
          }
          else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6)
          { // check it is IP6
               // is a valid IP6 Address
               tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
               char addressBuffer[INET6_ADDRSTRLEN];
               inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
               printf("%s IP Address %s/n", ifAddrStruct->ifa_name, addressBuffer);
          }
          ifAddrStruct = ifAddrStruct->ifa_next;
     }
     return 0;
}