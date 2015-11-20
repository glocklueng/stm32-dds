include ../../Makefile.inc

# select directory containing the platform specific files
PORTDIR=port/STM32F4x7/Standalone
STM32_DIR=../stm32f4
STM32_INCLUDES=-I../../include -I$(STM32_DIR)/include
STM32_INCLUDES+=-I$(STM32_DIR)/include/core -I$(STM32_DIR)/include/peripherals

# this library doesn't require the std c library
CFLAGS+=-ffreestanding -nostdlib

CPPFLAGS+=-Isrc/include -Isrc/include/ipv4 -I$(PORTDIR)/include -I.
CPPFLAGS+=$(STM32_INCLUDES)

LIB=lwip
SRCS=$(PORTDIR)/ethernetif.c \
     src/core/def.c \
     src/core/dhcp.c \
     src/core/dns.c \
     src/core/init.c \
     src/core/ipv4/autoip.c \
     src/core/ipv4/icmp.c \
     src/core/ipv4/igmp.c \
     src/core/ipv4/inet.c \
     src/core/ipv4/inet_chksum.c \
     src/core/ipv4/ip.c \
     src/core/ipv4/ip_addr.c \
     src/core/ipv4/ip_frag.c \
     src/core/mem.c \
     src/core/memp.c \
     src/core/netif.c \
     src/core/pbuf.c \
     src/core/raw.c \
     src/core/snmp/asn1_dec.c \
     src/core/snmp/asn1_enc.c \
     src/core/snmp/mib2.c \
     src/core/snmp/mib_structs.c \
     src/core/snmp/msg_in.c \
     src/core/snmp/msg_out.c \
     src/core/stats.c \
     src/core/sys.c \
     src/core/tcp.c \
     src/core/tcp_in.c \
     src/core/tcp_out.c \
     src/core/timers.c \
     src/core/udp.c \
     src/netif/etharp.c \
     src/netif/ethernetif.c \
     src/netif/ppp/auth.c \
     src/netif/ppp/chap.c \
     src/netif/ppp/chpms.c \
     src/netif/ppp/fsm.c \
     src/netif/ppp/ipcp.c \
     src/netif/ppp/lcp.c \
     src/netif/ppp/magic.c \
     src/netif/ppp/md5.c \
     src/netif/ppp/pap.c \
     src/netif/ppp/ppp.c \
     src/netif/ppp/ppp_oe.c \
     src/netif/ppp/randm.c \
     src/netif/ppp/vj.c \
     src/netif/slipif.c
OBJS=$(SRCS:.c=.o)

all: lib$(LIB).a

lib$(LIB).a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) lib$(LIB).a
