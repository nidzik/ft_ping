#include <sys/prctl.h>
#include <sys/capability.h>

#include "ft_ping.h"

void init_socket(void)
{
	int val = 0;
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
	if (avg.options & F_TTL)
		val = avg.ttl;
	else
		val = D_TTL;
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
	if (setsockopt(avg.sock, SOL_SOCKET, SO_REUSEADDR, &(reuseaddr), sizeof(reuseaddr)) != 0 )
	{
		printf("TTL error, setsockopt failed.\n");
		exit(-1);
	}
#ifdef F_MARK
	if (avg.options & F_MARK)
	{
		int ret;
		ret = setsockopt(avg.sock, SOL_SOCKET, SO_MARK, &avg.mark, sizeof(avg.mark));
		if (ret == -1)
			printf("Warning: Failed to set mark: %d", avg.mark);
	}
#endif
	avg.in.id = getpid();
	avg.in.time = 0;
	gettimeofday(&(avg.time), 0);

	return ;
}
