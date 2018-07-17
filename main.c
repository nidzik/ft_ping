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
struct packet
{
  struct icmphdr hdr;
  struct iphdr iphdr;
  char msg[64-(sizeof(struct icmphdr) + sizeof(struct iphdr))];
};

struct protoent 	*proto = NULL;

unsigned int alarm_cnt = 0;

int ping(struct sockaddr_in *addr, char *dom_name);

void  ALARMhandler(int sig)
{
//  signal(SIGALRM, SIG_IGN);          /* ignore this signal       */
//  printf("Hello\n");
  //  signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
  //  alarm(1);
  alarm_cnt++;
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

int display(int bytes, char *buf, struct sockaddr_in *addr, char *dom_name, long time, int id)
{
  struct addrinfo *addr_info;
  char a_name[1024] = { 0 };
    char a_name2[1024] = { 0 };
  struct iphdr *ip = (struct iphdr *)buf;

  //printf("size : %lu\n", ip->ihl);
  /*
   * struct icmphdr
   * {
   * u_int8_t type; 			--> message type 
   * u_int8_t code;			--> type sub-code 
   * u_int16_t checksum;
   * union
   *  {
   *       struct
   *	  {
   *	    u_int16_tid;
   *	    u_int16_tsequence;
   *	  } echo; 			--> echo datagram 
   *  u_int32_tgateway; 		--> gateway address 
   *      struct
   *	  {
   *	    u_int16_t__unused;
   *	    u_int16_tmtu;
   *	  } frag; 		      	--> path mtu discovery 
   * } un;
   * };
   */
  struct icmphdr * icmphdr1 = (struct icmphdr *)(buf + sizeof(struct iphdr));
  

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
   * #define ICMP_ECHOREPLY 0    ---> Echo Reply
   */  
  if (icmphdr1->type == ICMP_ECHOREPLY && (int)(icmphdr1)->un.echo.id == id)// && !strncmp(a_name, get_ip_from_header((uint32_t)ip->saddr), strlen(a_name)))
    {
            printf ("seq=%d  id=%d ",(int)(icmphdr1)->un.echo.sequence, (int)(icmphdr1)->un.echo.id);
      printf("%lu bytes from  %s icmp_seq=%lu ttl=%d proto=%d  time=%lu\n",bytes, a_name, (icmphdr1)->un.echo.sequence, (int)(ip->ttl),(int)(ip->protocol), time);
      
      //printf("\n%20s\n",(buf + sizeof(struct iphdr)));
    }
  else
    {
      printf("no ECHO_REPLY : code  = %d \n", (int)(icmphdr1)->un.echo.id);
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



int ping(struct sockaddr_in *addr, char *dom_name)
{
	int sock = 0;
	const int val = 255;
	struct sockaddr_in recv_addr;
	int len_addr = 0;
	struct packet pkt;
	int len = 0;
	int pid = 0;
	int i = 0;
	int cnt = 1;
	int ret = 0;
	char buf[1024] = { 0 };
	char data[64] = { 0 };
	struct timeval t0;
	struct timeval t1;
	int erecv = 0;
	struct msghdr hmsg;
	struct cmsghdr *cmhdr = NULL;
	struct iovec iov[1];
	char aux[1024] = { 0 };
	unsigned char *tos = NULL;
	int size_send = 0;
/*
// struct msghdr :

void         *msg_name        optional address
socklen_t     msg_namelen     size of address
struct iovec *msg_iov         scatter/gather array
int           msg_iovlen      members in msg_iov
void         *msg_control     ancillary data, see below
socklen_t     msg_controllen  ancillary data buffer len
int           msg_flags       flags on received message
*/
	hmsg.msg_name = &recv_addr;
	hmsg.msg_namelen = sizeof(recv_addr);
	hmsg.msg_iov = iov;
	hmsg.msg_iovlen = 1;
	hmsg.msg_control = &aux;
	hmsg.msg_controllen = sizeof(aux) ;
	iov[0].iov_base = data;
	iov[0].iov_len = sizeof(data);  

  sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sock < 0)
    {
      printf("error getting the socket.\n");
      perror(NULL);
      return (-1);
    }
  if (setsockopt(sock, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
      printf("TTL error, setsockopt failed.\n");
      return (-1);
    }
  pid = getpid();
  int saved_alarm = 0;
  signal(SIGALRM, ALARMhandler);
  //    alarm(1);
  //  while(1);
 
  while (1)
    {

      len = sizeof(&recv_addr);
      if (ret = recvmsg(sock, &hmsg, MSG_DONTWAIT) > 0 && cnt != 1)
	{
	  //  write(1,hmsg.msg_iov[0].iov_base + (sizeof (struct iphdr) + sizeof(struct icmphdr)),64);
	  
	  gettimeofday(&t1, 0);
	  //	  test_alarm = alarm(1);
	  //printf("alarm = %lu \n" , test_alarm);
	  long time =  t1.tv_usec-t0.tv_usec ;

	  /*
	   * data are in iov[0].iov_base (data[]) for the iphdr parsing in display. (ttl proto..)
	   */
	  printf("Display : \n");
	  erecv = display(hmsg.msg_iov[0].iov_len, hmsg.msg_iov[0].iov_base, addr, dom_name, time, pid);

	  if (erecv != -1)
	    {
	      //	      if (cnt > 155)
		{
	      alarm(1);
	      while (1)
		{
		  if (alarm_cnt != saved_alarm)
		    break;
		}

	      saved_alarm++;
		}
	    }
	}
      else
	{
	  	  printf("no Display : \n");
	  printf("ft_ping: socket: Permission denied, attempting raw socket...\n");
	  perror(NULL);
	  erecv = -1;
      alarm(1);
      while (1)
	{
	  if (alarm_cnt != saved_alarm)
	    break;
	}
      saved_alarm++;

	}

      
      bzero(&pkt, sizeof(pkt));
      pkt.hdr.type = ICMP_ECHO;
      pkt.hdr.un.echo.id = pid;
      printf("pid : %d && sqved : %d && al_cnt : %d \n", pid, saved_alarm, alarm_cnt);
      for (i = 0; i < sizeof(pkt.msg)-1; i++ )
	pkt.msg[i] = i+'0';
      pkt.msg[i] = 0;
      pkt.hdr.un.echo.sequence = cnt++;
      pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));
      //      test_alarm = alarm(0);

      gettimeofday(&t0, 0);
      //      printf("send : icmp=%lu  iphdr=%lu\n",sizeof(struct icmphdr), sizeof(struct iphdr));
      if ((size_send = sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr*)addr, sizeof(*addr))) <= 0 )
	perror("sendto");
      else
	printf("package send : \n");

      //printf("sizepkt : %d  size data : %lu\n",size_send, sizeof(pkt.msg));
      if (pkt.hdr.un.echo.sequence == 0)
	printf("PING %s %lu bytes of data.\n", dom_name, sizeof(pkt.msg));
     
    }

}
