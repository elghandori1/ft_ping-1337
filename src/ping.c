#include "../includes/ft_ping.h"
#include <stdint.h>
#include <math.h>
#include <sys/select.h>

#define RECV_BUF_SIZE 65536

static volatile sig_atomic_t g_sigint;

static void handle_stop(int sig)
{
    (void)sig;
    g_sigint = 1;
}

static uint16_t icmp_checksum(const void *buf, size_t len)
{
    const uint16_t *p;
    uint32_t sum;

    p = (const uint16_t *)buf;
    sum = 0;
    while (len > 1)
    {
        sum += *p++;
        len -= 2;
    }
    if (len == 1)
        sum += (uint16_t)(*(const uint8_t *)p);
    while (sum >> 16)
        sum = (sum & 0xffffU) + (sum >> 16);
    return (uint16_t)(~sum & 0xffffU);
}

static void print_icmp_error_verbose(struct icmphdr *icmp, struct in_addr *from,
    uint16_t seq)
{
    char from_str[INET_ADDRSTRLEN];

    if (!inet_ntop(AF_INET, from, from_str, sizeof(from_str)))
        return;
    if (icmp->type == ICMP_TIME_EXCEEDED)
    {
        printf("From %s icmp_seq=%u Time to live exceeded\n", from_str,
            (unsigned int)seq);
        return;
    }
    if (icmp->type == ICMP_DEST_UNREACH)
    {
        printf("From %s icmp_seq=%u Destination Host Unreachable\n", from_str,
            (unsigned int)seq);
    }
}

static int recv_one_icmp(int sock, struct sockaddr_in *peer, t_args *args,
    uint16_t want_id, uint16_t want_seq, struct timeval *sent_at,
    int *got_reply, int *got_related_err, double *rtt_ms, int *reply_ttl)
{
    unsigned char buf[RECV_BUF_SIZE];
    ssize_t n;
    struct sockaddr_in from;
    socklen_t fromlen;
    struct timeval now;
    struct iphdr *ip;
    struct icmphdr *icmp;
    size_t ip_len;
    uint16_t rid;
    uint16_t rseq;

    *got_reply = 0;
    *got_related_err = 0;
    fromlen = sizeof(from);
    n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
    if (n < 0)
        return -1;
    if (n < (ssize_t)sizeof(struct iphdr))
        return 0;
    ip = (struct iphdr *)buf;
    ip_len = (size_t)ip->ihl * 4U;
    if (ip_len < sizeof(struct iphdr) || (size_t)n < ip_len + sizeof(struct icmphdr))
        return 0;
    if (ip->protocol != IPPROTO_ICMP)
        return 0;
    icmp = (struct icmphdr *)(buf + ip_len);
    if (icmp->type == ICMP_ECHOREPLY)
    {
        rid = ntohs(icmp->un.echo.id);
        rseq = ntohs(icmp->un.echo.sequence);
        if (rid != want_id)
            return 0;
        if (from.sin_addr.s_addr != peer->sin_addr.s_addr)
            return 0;
        if (rseq != want_seq)
            return 0;
        gettimeofday(&now, NULL);
        *rtt_ms = (now.tv_sec - sent_at->tv_sec) * 1000.0
            + (now.tv_usec - sent_at->tv_usec) / 1000.0;
        if (*rtt_ms < 0.0)
            *rtt_ms = 0.0;
        *reply_ttl = (int)ip->ttl;
        *got_reply = 1;
        return 1;
    }
    if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_DEST_UNREACH)
    {
        if (args->verbose)
        {
            print_icmp_error_verbose(icmp, &from.sin_addr, want_seq);
            *got_related_err = 1;
        }
        return 0;
    }
    return 0;
}

static void print_stats(const char *target, unsigned long tx, unsigned long rx,
    double min_ms, double max_ms, double sum_ms, double sum_sq_ms,
    struct timeval *start, struct timeval *end)
{
    double avg;
    double stddev;
    double loss;
    long total_ms;

    total_ms = time_diff_ms(start, end);
    if (total_ms < 0)
        total_ms = 0;

    printf("\n--- %s ping statistics ---\n", target);
    if (tx == 0)
    {
        printf("0 packets transmitted, 0 received, 100%% packet loss, time %ldms\n",
            total_ms);
        return;
    }
    loss = 100.0 * (double)(tx - rx) / (double)tx;
    if (loss < 0.0)
        loss = 0.0;
    if (loss > 100.0)
        loss = 100.0;
    printf("%lu packets transmitted, %lu received, %.0f%% packet loss, time %ldms\n",
        tx, rx, loss, total_ms);
    if (rx == 0)
        return;
    avg = sum_ms / (double)rx;
    if (rx > 1)
        stddev = sqrt((sum_sq_ms / (double)rx) - (avg * avg));
    else
        stddev = 0.0;
    if (stddev < 0.0)
        stddev = 0.0;
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
        min_ms, avg, max_ms, stddev);
}

static void print_reply(t_args *args, char *ipstr, int pkt_len, uint16_t seq,
    int reply_ttl, double rtt)
{
    if (args->numeric)
        printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
            pkt_len, ipstr, (unsigned int)seq, reply_ttl, rtt);
    else if (strcmp(args->target, ipstr) != 0)
        printf("%d bytes from %s : icmp_seq=%u ttl=%d time=%.3f ms\n",
            pkt_len, args->target, (unsigned int)seq, reply_ttl, rtt);
    else
        printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
            pkt_len, ipstr, (unsigned int)seq, reply_ttl, rtt);
}

void run_ping(t_args *args, struct sockaddr_in *addr, char *ipstr)
{
    int sock;
    unsigned char *pkt;
    size_t pkt_len;
    struct icmphdr *icmp;
    uint16_t id;
    uint16_t seq;
    unsigned long tx;
    unsigned long rx;
    double min_ms;
    double max_ms;
    double sum_ms;
    double sum_sq_ms;
    struct timeval t_start;
    struct timeval t_end;
    struct sigaction sa;
    struct sigaction old_int;
    struct sigaction old_term;

    pkt_len = sizeof(struct icmphdr) + (size_t)args->size;
    pkt = (unsigned char *)malloc(pkt_len);
    if (!pkt)
    {
        perr_errno("malloc");
        exit(EXIT_FAILURE);
    }
    g_sigint = 0;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, &old_int) == -1)
    {
        perr_errno("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, &old_term) == -1)
    {
        perr_errno("sigaction");
        sigaction(SIGINT, &old_int, NULL);
        exit(EXIT_FAILURE);
    }
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        if (errno == EPERM || errno == EACCES)
            fprintf(stderr, "ft_ping: socket: Operation not permitted (raw ICMP needs root or CAP_NET_RAW)\n");
        else
            perr_errno("socket");
        sigaction(SIGTERM, &old_term, NULL);
        sigaction(SIGINT, &old_int, NULL);
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &args->ttl, sizeof(args->ttl)) == -1)
    {
        perr_errno("setsockopt");
        close(sock);
        sigaction(SIGTERM, &old_term, NULL);
        sigaction(SIGINT, &old_int, NULL);
        exit(EXIT_FAILURE);
    }
    gettimeofday(&t_start, NULL);
    id = (uint16_t)(getpid() & 0xffff);
    seq = 0;
    tx = 0;
    rx = 0;
    min_ms = 1e12;
    max_ms = 0.0;
    sum_ms = 0.0;
    sum_sq_ms = 0.0;
    while (!g_sigint)
    {
        struct timeval sent_at;
        struct timeval now_chk;
        fd_set rfds;
        int rv;
        int wait_ms;
        int got_reply;
        int got_related_err;
        double rtt;
        int reply_ttl;
        int deadline_hit;

        deadline_hit = 0;
        if (args->deadline_sec > 0)
        {
            gettimeofday(&now_chk, NULL);
            if (time_diff_ms(&t_start, &now_chk) >= args->deadline_sec * 1000L)
                break;
        }
        if (args->count > 0 && (int)tx >= args->count)
            break;
        memset(pkt, 0, pkt_len);
        icmp = (struct icmphdr *)pkt;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.id = htons(id);
        icmp->un.echo.sequence = htons(seq);
        memset(pkt + sizeof(struct icmphdr), 0x42, (size_t)args->size);
        icmp->checksum = icmp_checksum(pkt, pkt_len);
        if (sendto(sock, pkt, pkt_len, 0, (struct sockaddr *)addr,
                sizeof(*addr)) < 0)
        {
            fprintf(stderr, "ft_ping: sendto: %s\n", strerror(errno));
            break;
        }
        tx++;
        gettimeofday(&sent_at, NULL);
        wait_ms = args->timeout_ms;
        got_reply = 0;
        got_related_err = 0;
        rtt = 0.0;
        reply_ttl = 0;
        while (wait_ms > 0 && !g_sigint)
        {
            struct timeval tv;
            int slice;

            if (args->deadline_sec > 0)
            {
                gettimeofday(&now_chk, NULL);
                if (time_diff_ms(&t_start, &now_chk) >= args->deadline_sec * 1000L)
                {
                    deadline_hit = 1;
                    break;
                }
            }
            slice = wait_ms;
            if (slice > 500)
                slice = 500;
            tv.tv_sec = slice / 1000;
            tv.tv_usec = (slice % 1000) * 1000;
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);
            rv = select(sock + 1, &rfds, NULL, NULL, &tv);
            if (rv < 0)
            {
                if (errno == EINTR)
                {
                    if (g_sigint)
                        break;
                    continue;
                }
                fprintf(stderr, "ft_ping: select: %s\n", strerror(errno));
                break;
            }
            if (rv == 0)
            {
                wait_ms -= slice;
                continue;
            }
            if (recv_one_icmp(sock, addr, args, id, seq, &sent_at,
                    &got_reply, &got_related_err, &rtt, &reply_ttl) < 0)
            {
                if (errno == EINTR && g_sigint)
                    break;
                continue;
            }
            if (got_reply)
            {
                rx++;
                if (rtt < min_ms)
                    min_ms = rtt;
                if (rtt > max_ms)
                    max_ms = rtt;
                sum_ms += rtt;
                sum_sq_ms += rtt * rtt;
                print_reply(args, ipstr, (int)pkt_len, seq, reply_ttl, rtt);
                break;
            }
            if (got_related_err)
                break;
        }
        if (g_sigint)
            break;
        if (deadline_hit)
            break;
        if (!got_reply && !got_related_err)
            printf("Request timeout for icmp_seq %u\n", (unsigned int)seq);
        if (args->count > 0 && (int)tx >= args->count)
            break;
        seq++;
        if (!g_sigint)
            sleep(1);
    }
    if (rx == 0)
    {
        min_ms = 0.0;
        max_ms = 0.0;
    }
    gettimeofday(&t_end, NULL);
    print_stats(args->target, tx, rx, min_ms, max_ms, sum_ms, sum_sq_ms,
        &t_start, &t_end);
    close(sock);
    free(pkt);
    sigaction(SIGTERM, &old_term, NULL);
    sigaction(SIGINT, &old_int, NULL);
}
