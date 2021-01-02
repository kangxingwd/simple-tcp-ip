#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/poll.h>
#include <arpa/inet.h>

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>

#include "protocol.h"
#include "utils.h"

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

            unsigned char *cur_ptr = stream;

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
                struct arppkt * arp_req = (struct arppkt *)stream;

                unsigned char eth_ip[128] = {0};
                unsigned char eth_mac[ETH_ADDR_LENGTH];
                if (get_eth_ip("ens33", eth_ip) != 0 || get_eth_mac("ens33", eth_mac) != 0) {
                    printf("get_eth_ip or get_eth_macfailed!");
                    return -1;
                }

                printf("rp_req->arp.hw_type)[0] = %02X %02X\n" ,((char*)(&arp_req->arp.hw_type))[0], ((char*)(&arp_req->arp.hw_type))[1]);

                if (arp_req->arp.op == htons(1) && arp_req->arp.dst_ip_addr == inet_addr(eth_ip)) {
                    printf("response the apr, ip = %s\n", eth_ip);

                    struct arppkt arp_rsp;
                    memcpy(&arp_rsp, arp_req, sizeof(struct arppkt));

                    // eth 
                    memcpy(arp_rsp.eth.src_addr, eth_mac, ETH_ADDR_LENGTH);
                    memcpy(arp_rsp.eth.dst_addr, arp_req->eth.src_addr, ETH_ADDR_LENGTH);

                    // arp
                    arp_rsp.arp.op = htons(2);

                    memcpy(arp_rsp.arp.src_hw_addr, eth_mac, ETH_ADDR_LENGTH);
                    arp_rsp.arp.src_ip_addr = arp_req->arp.dst_ip_addr;

                    memcpy(arp_rsp.arp.dst_hw_addr, arp_req->arp.src_hw_addr, ETH_ADDR_LENGTH);
                    arp_rsp.arp.dst_ip_addr = arp_req->arp.src_ip_addr;

                    nm_inject(nmr, &arp_rsp, sizeof(struct arppkt));
                }
                break;
            
            default:
                break;
            }
        }
    }

    return 0;
}