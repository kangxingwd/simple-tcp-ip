#pragma pack(1)

#define ETH_ADDR_LENGTH 6
#define ETH_ALEN	6
#define PROTO_IP	0x0800
#define PROTO_ARP	0x0806

#define PROTO_UDP	17
#define PROTO_ICMP	1
#define PROTO_IGMP	2

struct ethhdr {
    unsigned char dst_addr[ETH_ADDR_LENGTH];
    unsigned char src_addr[ETH_ADDR_LENGTH];
    unsigned short protocol;
};


struct iphdr {
    unsigned char   version:4,
                    hdrlen:4;
    unsigned char tos;
    unsigned short length;
    unsigned short id;
    unsigned short  flag:3,
                    offset:13;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short checknum;

    unsigned int sip;
    unsigned int dip;
};


struct udphdr {
    unsigned short sport;
    unsigned short dport;
    unsigned short length;
    unsigned short checknum;
};

struct icmphdr{
    unsigned char type;
    unsigned char code;
    unsigned short checknum;
    unsigned int other;
};


struct udppkt{
    struct ethhdr eth;
    struct iphdr ip;
    struct udphdr udp;
    unsigned char data[0];
};

