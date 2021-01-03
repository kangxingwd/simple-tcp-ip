#pragma pack(1)

#define ETH_ADDR_LENGTH 6
#define ETH_ALEN	6
#define PROTO_IP	0x0800
#define PROTO_ARP	0x0806

#define PROTO_UDP	17
#define PROTO_ICMP	1
#define PROTO_IGMP	2

#define ETH_HDR_LEN 14
#define IP_HDR_LEN 20
#define ICMP_HDR_LEN 8

struct ethhdr {
    unsigned char dst_addr[ETH_ADDR_LENGTH];
    unsigned char src_addr[ETH_ADDR_LENGTH];
    unsigned short protocol;
};
//14

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
//20

struct udphdr {
    unsigned short sport;
    unsigned short dport;
    unsigned short length;
    unsigned short checknum;
};

// struct icmphdr {
//     unsigned char type;
//     unsigned char code;
//     unsigned short checknum;
//     unsigned int other;
// };


struct udppkt {
    struct ethhdr eth;
    struct iphdr ip;
    struct udphdr udp;
    unsigned char data[0];
};

struct arphdr {
    unsigned short hw_type;
    unsigned short pro_type;
    unsigned char hw_len;
    unsigned char pro_len;
    unsigned short op;

    unsigned char src_hw_addr[ETH_ADDR_LENGTH];
    unsigned int src_ip_addr;

    unsigned char dst_hw_addr[ETH_ADDR_LENGTH];
    unsigned int dst_ip_addr;
};

struct arppkt {
    struct ethhdr eth;
    struct arphdr arp;
};

struct icmphdr {
    unsigned char icmp_type;
    unsigned char icmp_code;
    unsigned short icmp_cksum;
    unsigned short icmp_ident;
    unsigned short icmp_sep_num;
};

struct icmppkt {
    struct ethhdr eth;          //14
    struct iphdr ip;            //20
    struct icmphdr icmp;        //8
    unsigned char data[0];
};