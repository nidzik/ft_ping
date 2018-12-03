#include "ft_ping.h"

t_avg avg;

int main(int ac, char **av)
{
	struct sockaddr_in 	addr;
	struct hostent	*hostname = NULL;

	if (ac != 2)
	{
		check_args(av);
		if (!avg.options)
		{
			printf("usage : ft_ping [-fmtci] <hostname>\n");
			return (-1);
		}
	}
	avg.proto = getprotobyname("ICMP");
	while (*av++)
	{
		if (*av && (hostname = gethostbyname(*av)) == NULL)
		{
			if (avg.options & F_VERB)
				printf("ft_ping: %s: Name or service not known\n", *av);
			if (*av == NULL || *(av + 1) == NULL )
				return (-1);
	}
		else if (hostname && strlen(hostname->h_name) > 6)
		break;
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = hostname->h_addrtype;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(long*)hostname->h_addr;
	signal(SIGINT, int_handler);
	ping(&addr, *av);
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

void tvsub( struct timeval *out, struct timeval *in )

{

	if( (out->tv_usec -= in->tv_usec) < 0 )   {
	out->tv_sec--;
	out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

double      timevaldiff(struct timeval *tv_1, struct timeval *tv_2)
{
	return ((double)(((tv_2->tv_sec - tv_1->tv_sec) * 1000000) + (tv_2->tv_usec - tv_1->tv_usec)) / 1000);
}

int display(struct info *in, struct sockaddr_in *addr, char *dom_name)
{
	struct addrinfo *addr_info;
	char a_name[1024] = { 0 };
	struct iphdr *ip = (struct iphdr *)in->buf;
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
	if (!(avg.options & F_FLOOD))
	{
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
	}
	(void)addr;
	/*
	 * check if the package is a ECHO REPLY from the host requested.
	 * from ip_icmp.h : 
	 * #define ICMP_ECHOREPLY 0	---> Echo Reply
	 */  
	if (icmphdr1->type == ICMP_ECHOREPLY && (int)(icmphdr1)->un.echo.id == in->id)
	{
		float time_f = 0;
		if ((int)(ip->ttl) == 1)
			printf("%d bytes from %s icmp_seq=%d Time to live exceeded\n",in->bytes, a_name, ntohs((icmphdr1)->un.echo.sequence));
		else
		{
			time_f = (float)((float)avg.in.time / 1000);
			
			if (!(avg.options & F_FLOOD) && !(avg.options & F_QUIET))
				printf("%d bytes from %s icmp_seq=%d ttl=%d time=%.3f ms\n",in->bytes, a_name, ntohs((icmphdr1)->un.echo.sequence), ((int)(ip->ttl)- (64 - avg.ttl))  , time_f);
			double timedouble = timevaldiff(&avg.time2, &avg.time1);
			avg.rtt_sum += timedouble;
			if (timedouble > avg.rtt_max)
				avg.rtt_max = timedouble;
			if (avg.rtt_min == 0)
				avg.rtt_min = timedouble;
			else if (timedouble < avg.rtt_min)
				avg.rtt_min = timedouble;

			avg.pck_recv++;
			if (avg.options & F_COUNT)
			if (avg.pck_recv >= avg.count)
				gg();
		}
	avg.send = 0;
	}
	else
	{
		if (icmphdr1->type == ICMP_TIME_EXCEEDED /* 11 */ && icmphdr1->code ==  ICMP_EXC_TTL/* 0 */)
			printf("%d bytes from %s icmp_seq=%d Time to live exceeded\n",in->bytes, a_name, avg.pck_transmited);
		else if (avg.options & F_VERB)
			printf("%d bytes from %s icmp_seq=%d icmp_type=%d icmp_code=%d\n",in->bytes, a_name, avg.pck_transmited, icmphdr1->type, icmphdr1->code);
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

void init_avg(void)
{
	avg.pck_transmited = 0;
    avg.pck_recv = 0;
	avg.pck_loss = 0;
	avg.min = 0;
	avg.avg = 0;
	avg.max = 0;
	avg.mdev = 0;
	avg.send = 0;
	avg.sock = -1;
	avg.rtt_min = 0;
	avg.rtt_max = 0;
	avg.rtt_sum = 0;
	avg.addr_info = NULL;
	if(!avg.options || !(avg.options & F_TTL))
		avg.ttl = D_TTL;
		}

void send_packet(void)
{
	struct packet pkt;
	int i = 0;
	
	bzero(&pkt, sizeof(pkt));

	pkt.hdr.type = ICMP_ECHO;
	pkt.hdr.un.echo.id = avg.in.id;
	for (i = 0; i < (int)sizeof(pkt.msg)-1; i++ )
		pkt.msg[i] = i+'0';
	pkt.msg[i] = 0;
	avg.str = pkt.msg;
	pkt.hdr.un.echo.sequence = htons(avg.pck_transmited + 1);
	pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));

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
	struct icmphdr * icmphdr1 = NULL;
	struct iphdr *ip = NULL; 
	
	if (recvmsg(avg.sock, &(avg.hmsg), 0) > 0)
	{
		gettimeofday(&avg.time1, 0);
		avg.in.time = 0;
		avg.in.time = avg.time1.tv_usec - avg.time2.tv_usec;

		/*
		 * data are in iov[0].iov_base (data[]) for the iphdr parsing in display. (ttl proto..)
		 */
		avg.in.bytes = avg.hmsg.msg_iov[0].iov_len;
		avg.in.buf = avg.hmsg.msg_iov[0].iov_base;
		ip = (struct iphdr *)avg.in.buf;
		icmphdr1 = (struct icmphdr *)(avg.in.buf +sizeof(struct iphdr));

		if (avg.in.id == (int)(icmphdr1)->un.echo.id || !strncmp(get_ip_from_header((uint32_t)ip->saddr), avg.dom_name, 7) || (!strncmp(avg.dom_name, "localhost", 9) && avg.in.id == (int)(icmphdr1)->un.echo.id))
			display(&avg.in, avg.addr, avg.dom_name);
	}
}

#define BUFFER_MAX_SIZE 1024

int ping(struct sockaddr_in *addr, char *dom_name)
{
	struct sockaddr_in recv_addr;
	struct packet pkt;
	char data[64] = { 0 };
	char aux[1024] = { 0 };

	init_avg();
	avg.addr = addr;
	avg.dom_name = dom_name;

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
	avg.hmsg.msg_name = &recv_addr;
	avg.hmsg.msg_namelen = sizeof(recv_addr);
	avg.hmsg.msg_iov = avg.iov;
	avg.hmsg.msg_iovlen = 1;
	avg.hmsg.msg_control = &aux;
	avg.hmsg.msg_controllen = sizeof(aux) ;
	avg.iov[0].iov_base = data;
	avg.iov[0].iov_len = sizeof(data);  
	avg.sock = socket(AF_INET, SOCK_RAW , avg.proto->p_proto);

	init_socket();
	if (!avg.pck_transmited)
		printf("PING %s %lu(%lu) bytes of data.\n", avg.dom_name, sizeof(pkt.msg) + sizeof(pkt.iphdr),sizeof(pkt.msg) + sizeof(pkt.iphdr) +  sizeof(pkt.hdr) + 20 );
	if (!(avg.options & F_FLOOD ))
	{
		signal(SIGALRM, ALARMhandler);
		alarm(1);
	}
	send_packet();

	
	while(1)
	{
		recv_packet();
		if (avg.options & F_FLOOD && avg.send == 0)
			send_packet();
	}
}

