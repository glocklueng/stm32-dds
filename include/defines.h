#ifndef __DEFINES_H
#define __DEFINES_H

/* network configuration settings */
#define MAC_ADDR0 02
#define MAC_ADDR1 00
#define MAC_ADDR2 00
#define MAC_ADDR3 00
#define MAC_ADDR4 00
#define MAC_ADDR5 00

#define IP_ADDR0 172
#define IP_ADDR1 31
#define IP_ADDR2 10
#define IP_ADDR3 50

#define NETMASK_ADDR0 255
#define NETMASK_ADDR1 255
#define NETMASK_ADDR2 255
#define NETMASK_ADDR3 0

#define GW_ADDR0 172
#define GW_ADDR1 31
#define GW_ADDR2 10
#define GW_ADDR3 1

// chip is soldered in RMII_MODE not in MII_MODE
#define RMII_MODE

#endif /* __DEFINES_H */