
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

struct average
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
	struct sockaddr_in *addr;
	struct msghdr hmsg;
	struct iovec iov[1];
	struct info in;
};

void send_packet(void);

struct protoent 	*proto = NULL;
struct average avg = {};
unsigned int alarm_cnt = 0;

int ping(struct sockaddr_in *addr, char *dom_name);

void gg(void)
{
	struct timeval t;
	int time = 0;
	gettimeofday(&t, 0);
	avg.pck_loss = avg.pck_transmited - avg.pck_recv;
	time = (int)((t.tv_sec - avg.time.tv_sec)* 1000 + (t.tv_usec - avg.time.tv_usec)/ 1000 ) ;
	
	printf("\n--- localhost ping statistics ---\n");
	printf("%d packets transmitted, %d reveived, %d  packet loss, time %dms\n", avg.pck_transmited, avg.pck_recv, avg.pck_loss, time);
	exit(0);
}

void intHandler(int dummy) {
	gg();
}

void  ALARMhandler(int sig)
{
//  signal(SIGALRM, SIG_IGN);		  /* ignore this signal	   */
//  printf("Hello\n");
//	  signal(SIGALRM, ALARMhandler);	 /* reinstall the handler	*/
		  send_packet();
	  alarm(1);
//	  printf("Hello1\n");

//	alarm_cnt++;
}

int main(int ac, char **av)
{
	struct sockaddr_in 	addr;

	struct hostent	*hostname = NULL;

	if (ac != 2)
	{
	printf("usage : ft_ping <hostname>\n");
	return (-1);
	}

	proto = getprotobyname("ICMP");

	if ((hostname = gethostbyname(av[1])) == NULL)
	{
	printf("ft_ping: %s: Name or service not known\n", av[1]);
	return (-1);
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = hostname->h_addrtype;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(long*)hostname->h_addr;

	signal(SIGINT, intHandler);

	ping(&addr, av[1]);


	return (0);
}

unsigned short checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
	sum += *buf++;
	if ( len == 1 )
	sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}
char *get_ip_from_header(uint32_t ip)
{
	struct in_addr addr;
	char *ip_str = NULL;
  
	addr.s_addr = ip;
	return (ip_str = inet_ntoa(addr));
}

int display(struct info *in, struct sockaddr_in *addr, char *dom_name)
{
	struct addrinfo *addr_info;
	char a_name[1024] = { 0 };
	char a_name2[1024] = { 0 };
	struct iphdr *ip = (struct iphdr *)in->buf;

//	printf("size : %lu    %s \n", ip->ihl, get_ip_from_header((uint32_t)ip->saddr));
	/*
	 * struct icmphdr
	 * {
	 * u_int8_t type; 			--> message type 
	 * u_int8_t code;			--> type sub-code 
	 * u_int16_t checksum;
	 * union
	 *  {
	 *	   struct
	 *	  {
	 *		u_int16_t id;
	 *		u_int16_t sequence;
	 *	  } echo; 			--> echo datagram 
	 *  u_int32_t gateway; 		--> gateway address 
	 *	  struct
	 *	  {
	 *		u_int16_t_ _unused;
	 *		u_int16_t mtu;
	 *	  } frag; 			  	--> path mtu discovery 
	 * } un;
	 * };
	 */
	struct icmphdr * icmphdr1 = (struct icmphdr *)(in->buf + sizeof(struct iphdr));


	if (getaddrinfo(dom_name, NULL, NULL, &addr_info) != 0)
	{
	printf("error getaddrinfo..\n");
	return (-1);
	}

	if (inet_ntop(AF_INET, &((struct sockaddr_in *)addr_info->ai_addr)->sin_addr, a_name, sizeof(a_name)) == NULL)
	{
	printf("error inet_ntop..\n");
	return -1;
	}

	//printf("aname = %s dom = %s ip->proto = %lu  aname2 = %s\n", a_name, dom_name, ip->protocol,   get_ip_from_header((uint32_t)ip->saddr));
	/*
	 * check if the package is a ECHO REPLY from the host requested.
	 * from ip_icmp.h : 
	 * #define ICMP_ECHOREPLY 0	---> Echo Reply
	 */  
	if (icmphdr1->type == ICMP_ECHOREPLY && (int)(icmphdr1)->un.echo.id == in->id)// && !strncmp(a_name, get_ip_from_header((uint32_t)ip->saddr), strlen(a_name)))
	{
		printf ("yes  iphdr id = %d \n",ip->id);
		float time_f = 0;
		if ((int)(ip->ttl) == 1)
			printf("%lu bytes from %s icmp_seq=%lu Time to live exceeded\n",in->bytes, a_name, ntohs((icmphdr1)->un.echo.sequence));
		else
		{
			time_f = (float)((float)avg.in.time / 1000);// + (float)((float)time % 1000);
			// print proto : int(ip->protocol)
			//	printf ("seq=%d  id=%d ",(int)(icmphdr1)->un.echo.sequence, (int)(icmphdr1)->un.echo.id); 
			printf("%lu bytes from %s icmp_seq=%lu ttl=%d time=%3f ms\n",in->bytes, a_name, ntohs((icmphdr1)->un.echo.sequence), (int)(ip->ttl), time_f);
			avg.pck_recv++;
		}
	avg.send = 0;
	//printf("\n%20s\n",(buf + sizeof(struct iphdr)));
	}
	else
	{
		if (icmphdr1->type == ICMP_TIME_EXCEEDED /* 11 */ && icmphdr1->code ==  ICMP_EXC_TTL/* 0 */)
		{
			printf("%lu bytes from %s icmp_seq=%lu Time to live exceeded\n",in->bytes, a_name, avg.pck_transmited -1 );			
		}
		printf ("noo  iphdr id = %d \n",ip->id);
//		printf("aname : %s    dom_name : %s            idk : %s\n",a_name, dom_name, &((struct sockaddr_in *)addr_info->ai_addr)->sin_addr );
//		printf("no ECHO_REPLY : id  = %d type = %d   code : %d \n", (int)(icmphdr1)->un.echo.id, icmphdr1->type, icmphdr1->code );
		return -1;
	}
  
	return 0;
}

/*void init_icmphdr(struct icmphdr)
  {

  
  hmsg.msg_name = &recv_addr;
  hmsg.msg_namelen = sizeof(recv_addr);
  hmsg.msg_iov = iov;
  hmsg.msg_iovlen = 1;
  hmsg.msg_control = &aux;
  hmsg.msg_controllen = sizeof(aux) ;
  iov[0].iov_base = data;
  iov[0].iov_len = sizeof(data);  
  }
*/
void tvsub( struct timeval *out, struct timeval *in )

{

	if( (out->tv_usec -= in->tv_usec) < 0 )   {
	out->tv_sec--;
	out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

void init_avg(void){
	avg.pck_transmited = 0;
    avg.pck_recv = 0;
	avg.pck_loss = 0;
	avg.min = 0;
	avg.avg = 0;
	avg.max = 0;
	avg.mdev = 0;
	avg.send = 0;
	avg.sock = -1;
}

void send_packet(void)
{
	struct packet pkt;
    struct timeval t0;
	struct timeval t1;
	int i = 0;
	
	bzero(&pkt, sizeof(pkt));

	pkt.hdr.type = ICMP_ECHO;
	pkt.hdr.un.echo.id = avg.in.id;
	for (i = 0; i < sizeof(pkt.msg)-1; i++ )
		pkt.msg[i] = i+'0';
	pkt.msg[i] = 0;
	avg.str = pkt.msg;
//	printf("%%  %s  %%\n", pkt.msg);
	pkt.hdr.un.echo.sequence = htons(avg.pck_transmited + 1);
//		pkt.iphdr.id = 10;
//		pkt.iphdr.ttl = 10;
	pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));

//		printf("%%  %s  %%\n", pkt.msg);
	gettimeofday(&avg.time2, 0);
	if ((sendto(avg.sock, &pkt, sizeof(pkt), 0, (struct sockaddr*)avg.addr, sizeof(*avg.addr))) <= 0 )
	{
		perror("sendto");
		exit(1);
	}
	else
	{
		avg.pck_transmited++;
		avg.send = 1;
	}

}

void recv_packet(void)
{
	if (recvmsg(avg.sock, &avg.hmsg, 0) > 0)
	{
		gettimeofday(&avg.time1, 0);
		//len = sizeof(&recv_addr);
		avg.in.time = 0;
		avg.in.time = avg.time1.tv_usec - avg.time2.tv_usec;

		/*
		 * data are in iov[0].iov_base (data[]) for the iphdr parsing in display. (ttl proto..)
		 */
		avg.in.bytes = avg.hmsg.msg_iov[0].iov_len;
		avg.in.buf = avg.hmsg.msg_iov[0].iov_base; 
//		if (avg.in.buf != avg.str)
//			return;
	}
}

#define BUFFER_MAX_SIZE 1024

int ping(struct sockaddr_in *addr, char *dom_name)
{
	const int val = 255;
	int reuseaddr = 0;
	struct sockaddr_in recv_addr;
	int len_addr = 0;
	struct packet pkt;
	int len = 0;
	int i = 0;
	int cnt = 1;
	int ret = 0;
	char buf[1024] = { 0 };
	char data[64] = { 0 };
	struct timeval t0;
	struct timeval t1;
	struct timeval t3;
	int erecv = 0;
	struct cmsghdr *cmhdr = NULL;
	struct iovec iov[1];
	char aux[1024] = { 0 };
	unsigned char *tos = NULL;
	int size_send = 0;
	struct info in;
	struct msghdr hmsg ;
	init_avg();
	avg.addr = addr;
	avg.dom_name = dom_name;
	struct cmsghdr *cmsg = NULL;
	struct sock_extended_err *sock_err; 
	struct icmphdr * icmphdr1 = NULL;
	
	/*
	// struct msghdr :

	void		 *msg_name		optional address
	socklen_t	 msg_namelen	 size of address
	struct iovec *msg_iov		 scatter/gather array
	int		   msg_iovlen	  members in msg_iov
	void		 *msg_control	 ancillary data, see below
	socklen_t	 msg_controllen  ancillary data buffer len
	int		   msg_flags	   flags on received message
	*/
	hmsg.msg_name = &recv_addr;
	hmsg.msg_namelen = sizeof(recv_addr);
	hmsg.msg_iov = iov;
	hmsg.msg_iovlen = 1;
	hmsg.msg_control = &aux;
	hmsg.msg_controllen = sizeof(aux) ;
	iov[0].iov_base = data;
	iov[0].iov_len = sizeof(data);  
//			struct sockaddr_in *addr, char *dom_name, long time, int id)



	
	avg.sock = socket(AF_INET, SOCK_RAW , proto->p_proto);
   	
	if (avg.sock < 0)
	{
		printf("error getting the socket.\n");
		perror(NULL);
		return (-1);
	}
	if (setsockopt(avg.sock, IPPROTO_IP, IP_TTL , &val, sizeof(val)) != 0)
	{
		printf("TTL error, setsockopt failed.\n");
		return (-1);
	}
	else
		if (setsockopt(avg.sock, SOL_SOCKET, SO_REUSEADDR, &(reuseaddr), sizeof(reuseaddr)) != 0 )
		{
			printf("TTL error, setsockopt failed.\n");
			return (-1);
		}
	
	in.id = getpid();
	avg.in.id = in.id;
	int saved_alarm = 0;
	in.time = 0;	
	avg.in.time = in.time;
	gettimeofday(&(avg.time), 0);
	if (!avg.pck_transmited)
		printf("PING %s %lu %lu %lu bytes of data.\n", dom_name, sizeof(pkt.msg), sizeof(pkt.iphdr), sizeof(pkt.hdr));
	if (1 == 1 )
	{
	signal(SIGALRM, ALARMhandler);
	alarm(1);
	}
	send_packet();

	
	while(1)
	{

//		if (avg.send == 1)
		{
//			recv_packet();
			if (recvmsg(avg.sock, &hmsg, 0) > 0)
			{
				gettimeofday(&avg.time1, 0);
				//len = sizeof(&recv_addr);
				//in.time = 0;
				in.time = avg.time1.tv_usec - avg.time2.tv_usec;

				/*
				 * data are in iov[0].iov_base (data[]) for the iphdr parsing in display. (ttl proto..)
				 */
				in.bytes = hmsg.msg_iov[0].iov_len;
				in.buf = hmsg.msg_iov[0].iov_base;

				icmphdr1 = (struct icmphdr *)(in.buf + sizeof(struct iphdr));
				struct iphdr *ip = (struct iphdr *)in.buf;

				avg.in = in;
//				printf("avg.str == %s , avg.in.buf == %s , okok   in.buf == %s       ---> %d   %d \n", avg.str, avg.in.buf, in.buf, (int)(icmphdr1)->un.echo.id, avg.in.id);
				if (avg.in.id == (int)(icmphdr1)->un.echo.id || !strncmp(get_ip_from_header((uint32_t)ip->saddr), dom_name, 5))
					display(&in, avg.addr, avg.dom_name);
				//          return;
			}
//			if (avg.in.buf != avg.str)
//				display(&avg.in, avg.addr, avg.dom_name);
		}

	}
/*
** 
** if flood is activate, send_packet() here ! 
**

		else if (avg.send == 0  )// && (alarm_cnt != saved_alarm)) 
			erecv = -1;
		if ((erecv != -1 || cnt == 1) && 1==0)
		{
			send_packet();

		}
	}
*/
}

