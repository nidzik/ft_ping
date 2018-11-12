#include "ft_ping.h"

void init_socket(void)
{

	struct sockaddr_in recv_addr;
	char aux[1024] = { 0 };
	char data[64] = { 0 };
	const int val = 255;
	int reuseaddr = 0;
/*
    // struct msghdr :

    void         *msg_name      optional address
    socklen_t    msg_namelen     size of address
    struct iovec *msg_iov        scatter/gather array
    int        msg_iovlen     members in msg_iov
    void         *msg_control    ancillary data, see below
    socklen_t    msg_controllen  ancillary data buffer len
    int        msg_flags       flags on received message
*/
/*	
	avg.hmsg.msg_name = &recv_addr;
	avg.hmsg.msg_namelen = sizeof(recv_addr);
	avg.hmsg.msg_iov = avg.iov;
	avg.hmsg.msg_iovlen = 1;
	avg.hmsg.msg_control = &aux;
	avg.hmsg.msg_controllen = sizeof(aux) ;
	avg.iov[0].iov_base = data;
	avg.iov[0].iov_len = sizeof(data);

	avg.sock = socket(AF_INET, SOCK_RAW , avg.proto->p_proto);
*/		
	if (avg.sock < 0)
	{
		printf("error getting the socket.\n");
		perror(NULL);
		exit(-1);
	}
	if (setsockopt(avg.sock, IPPROTO_IP, IP_TTL , &val, sizeof(val)) != 0)
	{
		printf("TTL error, setsockopt failed.\n");
		exit(-1);
	}
//	else
		if (setsockopt(avg.sock, SOL_SOCKET, SO_REUSEADDR, &(reuseaddr), sizeof(reuseaddr)) != 0 )
		{
			printf("TTL error, setsockopt failed.\n");
			exit(-1);
		}
	avg.in.id = getpid();
	avg.in.time = 0;
	gettimeofday(&(avg.time), 0);

	return ;
}
