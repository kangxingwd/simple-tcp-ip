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
                    struct icmppkt *icmp_req = (struct icmppkt *)stream;

                    if (icmp_req->icmp.icmp_type == 8) {
                        unsigned short ip_len = ntohs(icmp_req->ip.length);
                        unsigned int data_len = (unsigned int)ip_len - IP_HDR_LEN -ICMP_HDR_LEN;
                        struct icmppkt *icmp_rsp = (struct icmppkt *)malloc(sizeof(struct icmppkt) + data_len);
                        memcpy(icmp_rsp, icmp_req, sizeof(struct icmppkt) + data_len);

                        // eth
                        memcpy(icmp_rsp->eth.src_addr, icmp_req->eth.dst_addr, ETH_ADDR_LENGTH);
                        memcpy(icmp_rsp->eth.dst_addr, icmp_req->eth.src_addr, ETH_ADDR_LENGTH);

                        // ip
                        icmp_rsp->ip.id = 0xfa4b;
                        icmp_rsp->ip.sip = icmp_req->ip.dip;
                        icmp_rsp->ip.dip = icmp_req->ip.sip;

                        // 为了计算一份数据报的IP检验和，首先把检验和字段置为0。然后，对首部中每个16位进行二进制反码求和(整个首部看成是由一串16位的字组成)，
                        // 结果存在检验和字段中。当接收端收到一份IP数据报后，同样对首部中每个16 位进行二进制反码的求和。由于接收方在计算过程中包含了发送方存在首部中的检验和，
                        // 因此，如果首部在传输过程中没有发生任何差错，那么接收方计算的结果应该为全1
                        icmp_rsp->ip.checknum = 0x0;
                        icmp_rsp->ip.checknum = cksum((unsigned short *)&(icmp_rsp->ip), IP_HDR_LEN);

                        // icmp
                        icmp_rsp->icmp.icmp_type = 0x0;
                        icmp_rsp->icmp.icmp_code = 0x0;
                        icmp_rsp->icmp.icmp_cksum = 0x0;
                        // icmp 的校验和包括数据
                        icmp_rsp->icmp.icmp_cksum = cksum((unsigned short *)&(icmp_rsp->icmp), ICMP_HDR_LEN + data_len);

                        nm_inject(nmr, icmp_rsp, sizeof(struct icmppkt) + data_len);
                        free(icmp_rsp);
                    }

                    printf("icmp end\n");
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
                    printf("arp end\n");
                }
                break;
            
            default:
                break;
            }
        }
    }

    return 0;
}