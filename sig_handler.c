#include "ft_ping.h"

void gg(void)
{
	struct timeval t;
	int time = 0;
	double rtt_avg = 0;

	avg.pck_loss = 0;
	gettimeofday(&t, 0);
	avg.pck_loss = 100 - ((avg.pck_recv * 100)/ avg.pck_transmited );
	if (avg.rtt_sum != 0 &&  avg.pck_recv != 0)
		rtt_avg = avg.rtt_sum / avg.pck_recv;
	time = (int)((t.tv_sec - avg.time.tv_sec)* 1000 + (t.tv_usec - avg.time.tv_usec)/ 1000 ) ;

	printf("\n--- %s ping statistics ---\n", avg.dom_name);
	printf("%d packets transmitted, %d reveived,", avg.pck_transmited, avg.pck_recv);
	if (avg.pck_transmited - avg.pck_recv > 0 )
		printf(" +%derrors,", avg.pck_transmited - avg.pck_recv);
	printf(" %d%% packet loss, time %dms\n",  avg.pck_loss, time);
	if (avg.pck_loss != 100)
		printf("rtt min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", avg.rtt_min, rtt_avg, avg.rtt_max, avg.rtt_max - avg.rtt_min);
	exit(0);
}

void int_handler(int dummy) 
{
	(void)dummy;
	if (avg.send == 1 && !(avg.options & F_FLOOD))
	recv_packet();
	gg();
}

void  ALARMhandler(int sig)
{
	(void)sig;
	send_packet();
	alarm(1);
}
