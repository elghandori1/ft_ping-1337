#include "../includes/ft_ping.h"

static void handle_sigint(int sig)
{
    (void)sig;
    exit(0);
}

int main(int argc, char **argv)
{
    t_args args;

    if (parse_args(argc, argv, &args) != 0)
        return EXIT_FAILURE;

    if (signal(SIGINT, handle_sigint) == SIG_ERR)
        perr_errno("signal");

    printf("PING %s", args.target);
    if (args.packet_size > 0)
        printf(" (%d data bytes)\n", args.packet_size);
    else
        printf("\n");

    if (args.verbose)
    {
        printf("Verbose mode enabled (level %d)\n", args.verbose);
        printf("Target: %s\n", args.target);
        printf("Packet size: %d bytes\n", args.packet_size);
        printf("TTL: %d\n", args.ttl);
        printf("Timeout: %d ms\n", args.timeout_ms);
        if (args.count > 0)
            printf("Count: %d packets\n", args.count);
        if (args.deadline_sec > 0)
            printf("Deadline: %d seconds\n", args.deadline_sec);
        if (args.numeric)
            printf("Numeric output only\n");
    }

    return EXIT_SUCCESS;
}