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
void gg(void);

void ALARMhandler(int sig);
void int_handler(int dummy);

#endif
