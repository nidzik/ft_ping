#include "ft_ping.h"


//#include <ctype.h>
//int avg.options;
//#define F_FLOOD  	0x001 
//#define F_MARK 		0x002

//int mark;
//#define D_MARK      42
static int strisnumber(const char *str)
{
	if (!str || !strlen(str))
		return (0);
	while (*str != '\0')
		if (!isdigit(*str++))
			return (0);
	return (1);
}

void check_args(char **args)
{
	int i = 0;
	int j = 0;
	int ac_check = 0;
	
	while (args[++j])
	{
		i = 1;
		if (args[j] && args[j][0] && args[j][0] == '-')
		{
			while (args[j][i])
			{
				switch (args[j][i])
				{
				case ('f'):
				{
					avg.options |= F_FLOOD;
					printf("okok F \n");
					break;
				}
				case ('m'):
				{
					ac_check++;
					if (args[j + ac_check] && strisnumber(args[j + ac_check]) )
						avg.mark = atoi(args[j + ac_check]);
					else
						avg.mark = D_MARK;
					avg.options |= F_MARK;
					break;
				}
				case ('v'):
				{
					avg.options |= F_VERB;
					printf("okok F \n");
					break;
				}
				case('c'):
				{
					ac_check++;
					if (args[j + ac_check] && strisnumber(args[j + ac_check]) )
						avg.count = atoi(args[j + ac_check]);
					else
						avg.count = D_COUNT;
					avg.options |= F_COUNT;
				}
				case('t'):
				{
					ac_check++;
					if (args[j + ac_check] && strisnumber(args[j + ac_check]) )
						avg.ttl = atoi(args[j + ac_check]);
					else
						avg.ttl = D_TTL;
					avg.options |= F_TTL;
				}
				default :
					printf("def\n");
					break;
					}
				i++;
			}
		}
		
	}
/*	if (avg.options & F_FLOOD & F_MARK)
		printf("avg.options & F_FLOOD & F_MARK\n");
	if (avg.options & (F_FLOOD | F_MARK))
		printf("avg.options & F_FLOOD | F_MARK\n");
	if (avg.options & F_MARK)
		printf("avg.options & F_MARK\n");
	if (avg.options & F_FLOOD)
		printf("avg.options & F_FLOOD\n");
	if (avg.options)
		printf("avg.options \n");
	if (!avg.options)
		printf("!avg.options \n");
	else
		printf("ko\n");
*/	
}
