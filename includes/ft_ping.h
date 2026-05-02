#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <stdbool.h>
# include <errno.h>
# include <signal.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <limits.h>

/* Defaults */
# define DEFAULT_TTL        64
# define DEFAULT_TIMEOUT    1000    /* ms */
# define PACKET_SIZE        56      /* Standard ping data size */

typedef struct s_args
{
    char    *target;        /* [MANDATORY] exact string from argv */
    int     verbose;        /* Verbosity level (0, 1, 2, ...) */
    
    int     count;          /* -c: packets to send (0 = unlimited) */
    int     ttl;            /* -t: 1-255 */
    int     timeout_ms;     /* -W: per-packet timeout in ms */
    int     deadline_sec;   /* -w: total runtime limit */
    bool    numeric;        /* -n: skip reverse DNS lookup */
    int     packet_size;    /* -s: data size in bytes */
} t_args;

/* args.c */
int     parse_args(int argc, char **argv, t_args *args);
void    print_usage(void);

/* utils.c */
void    perr_exit(const char *msg);
void    perr_errno(const char *msg);
long    time_diff_ms(struct timeval *start, struct timeval *end);

#endif