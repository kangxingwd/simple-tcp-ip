#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "protocol.h"

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

/*
     1、将校验和字段置为0。
     2、将每两个字节（16位）相加（二进制求和）直到最后得出结果，若出现最后还剩一个字节继续与前面结果相加。
     3、(溢出)将高16位与低16位相加，直到高16位为0为止。
     4、将最后的结果（二进制）取反。
*/
unsigned short cksum(unsigned short *ck_data, int len)
{
     unsigned short* data = ck_data;
     long sum = 0;
     while(len > 1) {
          sum += *(data++);
          if (sum & 0x80000000)
               sum = (sum & 0xFFFF) + (sum >> 16);
          len -= 2;
     }

     if (len)
          sum += (unsigned short) *data;
     
     while (sum >> 16) {
          sum = (sum & 0xFFFF) + (sum >> 16);
     }

     return ~sum;
}

unsigned short in_cksum(unsigned short *addr, int len)
{
	register int nleft = len;
	register unsigned short *w = addr;
	register int sum = 0;
	unsigned short answer = 0;

	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);	
	sum += (sum >> 16);			
	answer = ~sum;
	
	return (answer);

}
