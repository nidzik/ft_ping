#include "ft_ping.h"

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

void int_handler(int dummy) 
{
	(void)dummy;
	gg();
}

void  ALARMhandler(int sig)
{
	send_packet();
	alarm(1);
}
