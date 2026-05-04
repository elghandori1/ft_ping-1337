#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>
# include <signal.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <netdb.h>
#include <math.h>

# define DEFAULT_TTL        64
# define DEFAULT_TIMEOUT_MS 1000
# define PACKET_SIZE        56

typedef struct s_args
{
    char    *target;
    int     verbose;
} t_args;

int     parse_args(int argc, char **argv, t_args *args);
void    print_usage(void);

void    perr_errno(const char *msg);
long    time_diff_ms(struct timeval *start, struct timeval *end);
int     DNS_LookUp(const char *target, struct sockaddr_in *addr, char *ipstr,
            size_t ipstr_len);

void    run_ping(t_args *args, struct sockaddr_in *addr, char *ipstr);

#endif