#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/poll.h>
#include <arpa/inet.h>

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>

#include "protocol.h"

int main()
{
    printf("udp protocol test start: \n");

    struct ethhdr *eth;
    struct nm_desc *nmr = nm_open("netmap:ens33", NULL, 0, NULL);
    if (nmr == NULL) {
        printf("nm_open error!\n");
        return -1;
    }

    struct pollfd pfd = {0};
    pfd.fd = nmr->fd;
    pfd.events = POLLIN;

    while(1) {

        int ret = poll(&pfd, 1, -1);
        if (ret < 0) continue;

        if (pfd.revents & POLLIN) {
            struct nm_pkthdr h;
            unsigned char * stream = nm_nextpkt(nmr, &h);

            struct ethhdr *eth = (struct ethhdr *)stream;
            printf("eth protocol: %u\n", ntohs(eth->protocol));

            switch (ntohs(eth->protocol)) {
            case PROTO_IP:
                printf("[PROTO_IP] \n");

                struct iphdr *ip_hdr = (struct iphdr*)(stream + (unsigned char)sizeof(struct ethhdr));
                if (ip_hdr->protocol == PROTO_UDP) {
                    printf("[PROTO_IP][PROTO_UDP] \n");

                    struct udphdr * udp_hdr = (struct udphdr*)(stream + sizeof(struct ethhdr) + (unsigned char)sizeof(struct iphdr));
                    
                    printf("udp sport: %u\n", ntohs(udp_hdr->sport));
                    printf("udp dport: %u\n", ntohs(udp_hdr->dport));
                    printf("udp len: %u\n", ntohs(udp_hdr->length));
                    printf("udp checknum: %u\n", ntohs(udp_hdr->checknum));

                    unsigned char *udp_data = (unsigned char*)udp_hdr + sizeof(struct udphdr);
                    unsigned short udp_len =  ntohs(udp_hdr->length) - (unsigned short)(sizeof(struct udphdr));
                    udp_data[udp_len] = '\0';

                    printf("udp data: [%s]\n", udp_data);

                } else if (ip_hdr->protocol == PROTO_ICMP) {
                    printf("[PROTO_IP][PROTO_ICMP] \n");
                    
                } else if (ip_hdr->protocol == PROTO_IGMP) {
                    printf("[PROTO_IP][PROTO_IGMP] \n");

                } else {
                    printf("[PROTO_IP] ip_hdr protocol is %u\n", ip_hdr->protocol);
                }

                break;
            
            case PROTO_ARP:
                printf("[PROTO_ARP] \n");
                break;
            
            default:
                break;
            }

        }

    }

    return 0;
}