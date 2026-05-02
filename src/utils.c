#include "../includes/ft_ping.h"

void perr_exit(const char *msg)
{
    if (msg)
        fprintf(stderr, "ft_ping: %s\n", msg);
    exit(EXIT_FAILURE);
}

void perr_errno(const char *msg)
{
    if (msg)
    {
        if (errno != 0)
            fprintf(stderr, "ft_ping: %s: %s\n", msg, strerror(errno));
        else
            fprintf(stderr, "ft_ping: %s\n", msg);
    }
    else if (errno != 0)
        fprintf(stderr, "ft_ping: %s\n", strerror(errno));
}

long time_diff_ms(struct timeval *start, struct timeval *end)
{
    if (!start || !end) return -1;
    return (end->tv_sec - start->tv_sec) * 1000L +
           (end->tv_usec - start->tv_usec) / 1000L;
}