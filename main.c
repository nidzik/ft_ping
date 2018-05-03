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

struct packet
{
  struct icmphdr hdr;
  char msg[64-sizeof(struct icmphdr)];
};

struct protoent 	*proto = NULL;

int ping(struct sockaddr_in *addr, char *dom_name);
  
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

int display(int bytes, char *buf, struct sockaddr_in *addr, char *dom_name, long time)
{
  struct addrinfo *addr_info;
  char a_name[1024] = { 0 };
  struct iphdr *ip = (struct iphdr *)buf;

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
  printf("%lu bytes from  %s ttl=%d time=%lu\n", sizeof(struct packet), a_name, (int)(ip->ttl), time);
  return 0;
}

int ping(struct sockaddr_in *addr, char *dom_name)
{
	int sock;
	const int val = 255;
	struct sockaddr_in recv_addr;
	int len_addr = 0;
	struct packet pkt;
	int len;
	int pid = 0;
	int i = 0;
	int cnt = 0;
	int ret = 0;
	char buf[1024] = { 0 };
	struct timeval t0;
	struct timeval t1;
	int erecv = 0;
	struct msghdr *hmsg;
	struct iovec iov[1];
	char aux[1024] = { 0 };
	hmsg.msg_name = &recv_addr;
	hmsg.msg_len = sizeof(recv_addr);
	hmsg.msg_iov = iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = &aux;
	mhdr.msg_controllen = sizeof(aux) ;
	
	
  sock = socket(AF_INET, SOCK_RAW, proto->p_proto);
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
  while (1)
    {
      len = sizeof(&recv_addr);
      if (ret = recvmsg(sock, hmsg, MSG_DONTWAIT) > 0)
	{
	  //	   gettimeofday(&t0, 0);
	  //	   sleep(1);
	  gettimeofday(&t1, 0);
	  long time =  t1.tv_usec-t0.tv_usec;
	  
	  //	  printf("%lu got one...\n", elapsed);
	
	  display(ret, buf, addr, dom_name, time);
	  erecv = 0;
	}
      else
	{
	printf("ft_ping: socket: Permission denied, attempting raw socket...\n");
	perror(NULL);
	erecv = -1;
	}
      //      if (erecv != -1)
      sleep(1);

      bzero(&pkt, sizeof(pkt));
      pkt.hdr.type = ICMP_ECHO;
      pkt.hdr.un.echo.id = pid;
      for (i = 0; i < sizeof(pkt.msg)-1; i++ )
	pkt.msg[i] = i+'0';
      pkt.msg[i] = 0;
      pkt.hdr.un.echo.sequence = cnt++;
      pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));
      gettimeofday(&t0, 0);
      if ( sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
	perror("sendto");

	//	      sleep(1);
    }
}
