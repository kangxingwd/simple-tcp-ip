
int print_mac(unsigned char*mac);
int get_eth_ip(char *eth_name, unsigned char *ip);
int get_eth_mac(char *eth_name, unsigned char* mac);
int print_local_ip();
unsigned short cksum(unsigned short *ck_data, int len);
unsigned short in_cksum(unsigned short *addr, int len);