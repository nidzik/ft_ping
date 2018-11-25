#ifndef FT_PING_H
#define FT_PING_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <sys/capability.h>

#define F_FLOOD     0x001
#define F_MARK      0x002
#define F_TTL		0x004
#define F_VERB		0x008
#define F_COUNT		0x010
#define F_QUIET		0x020

#define D_COUNT		10
#define D_MARK		42
#define D_TTL		64

struct packet
{
	struct icmphdr hdr;                    //8
	struct iphdr iphdr;                    //20
	char msg[64-(sizeof(struct icmphdr) + sizeof(struct iphdr) )]; //36
};

struct info
{
	int bytes;
	char *buf;
	long time;
	int id;
};

typedef struct average
{
	int pck_transmited;
	int pck_recv;
	int pck_loss;
	struct timeval time;
	struct timeval time1;
	struct timeval time2;
	float min;
	float avg;
	float max;
	float mdev;
	int send;
	int sock;
	char *str;
	char *dom_name;
	int options;
	int mark;
	int count;
	int ttl;
	double rtt_min;
	double rtt_max;
	double rtt_sum;
	struct addrinfo *addr_info;
	struct protoent *proto;
	struct sockaddr_in *addr;
	struct msghdr hmsg;
	struct iovec iov[1];
	struct info in;
} t_avg;

extern t_avg avg;

int ping(struct sockaddr_in *addr, char *dom_name);
void init_socket(void);
void send_packet(void);
void recv_packet(void);
void gg(void);
void check_args(char **args);

void ALARMhandler(int sig);
void int_handler(int dummy);
//extern int modify_capability(cap_value_t, cap_flag_value_t);
/*static inline int enable_capability_admin(void) { return modify_capability(CAP_NET_ADMIN, CAP_SET);   }
static inline int disable_capability_admin(void){ return modify_capability(CAP_NET_ADMIN, CAP_CLEAR); }

static inline int enable_capability_raw(void){ return modify_capability(CAP_NET_RAW,   CAP_SET);   }
static inline int disable_capability_raw(void){ return modify_capability(CAP_NET_RAW,   CAP_CLEAR); }
static inline int enable_capability_admin(void){ return modify_capability(CAP_NET_ADMIN, CAP_SET);   }
static inline int disable_capability_admin(void){ return modify_capability(CAP_NET_ADMIN, CAP_CLEAR); }*/
#else
#endif
