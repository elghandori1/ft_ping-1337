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

int DNS_LookUp(const char *target, struct sockaddr_in *addr, char *ipstr, size_t ipstr_len)
{
    struct addrinfo hints;
     struct addrinfo *res = NULL;
    int ret;

    if (!target || !addr || !ipstr || ipstr_len == 0)
        return -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;

    ret = getaddrinfo(target, NULL, &hints, &res);
    if (ret != 0 || res == NULL)
    {
        fprintf(stderr, "ft_ping: unknown host\n");
        if (res) freeaddrinfo(res);
        return -1;
    }

    if (res->ai_addrlen < sizeof(struct sockaddr_in))
    {
        freeaddrinfo(res);
        fprintf(stderr, "ft_ping: unknown host\n");
        return -1;
    }
    memcpy(addr, res->ai_addr, sizeof(struct sockaddr_in));
    if (inet_ntop(AF_INET, &addr->sin_addr, ipstr, ipstr_len) == NULL)
    {
        freeaddrinfo(res);
        perr_errno("inet_ntop");
        return -1;
    }
    freeaddrinfo(res);
    return 0;
}