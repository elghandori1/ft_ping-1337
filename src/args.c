#include "../includes/ft_ping.h"

static void init_args(t_args *args)
{
    memset(args, 0, sizeof(t_args));
    args->ttl = DEFAULT_TTL;
    args->timeout_ms = DEFAULT_TIMEOUT;
    args->deadline_sec = -1;
    args->target = NULL;
    args->verbose = 0;
    args->count = 0;
    args->numeric = false;
    args->packet_size = PACKET_SIZE;
}

static bool parse_positive_int(const char *str, int *out)
{
    if (!str || !*str)
        return false;
    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || val < 0 || val > INT_MAX)
        return false;
    *out = (int)val;
    return true;
}

void print_usage(void)
{
    printf("Usage: ft_ping [-c count] [-v] [-t ttl] [-n] [-W timeout] [-w deadline] [-?] destination\n");
    printf("\n");
    printf("  -v              Verbose mode (show packet errors)\n");
    printf("  -?              Show this help message\n");
    printf("\n");
    printf("  -c <count>      Stop after <count> packets (default: unlimited)\n");
    printf("  -t <ttl>        Set packet TTL (default: 64, range: 1-255)\n");
    printf("  -n              Disable DNS resolution (numeric output only)\n");
    printf("  -W <timeout>    Timeout for each reply in seconds (default: 1)\n");
    printf("  -w <deadline>   Exit after <deadline> seconds total\n");
}

int parse_args(int argc, char **argv, t_args *args)
{
    init_args(args);

    if (argc < 2)
    {
            fprintf(stderr, "ft_ping: usage error: Destination address required\n");
     
        return 1;
    }

    int i = 1;
    bool target_set = false;

    while (i < argc)
    {
        if (argv[i][0] != '-')
        {
            if (target_set)
            {
                fprintf(stderr, "ft_ping: unexpected argument\n");
                return 1;
            }
            args->target = argv[i];
            target_set = true;
            i++;
            continue;
        }

        if (strcmp(argv[i], "-v") == 0)
        {
            args->verbose++;
            i++;
        }
        else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0 
                 || strcmp(argv[i], "--usage") == 0)
        {
            print_usage();
            exit(0);
        }
        else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0)
        {
            printf("ft_ping, from iputils-s20190709\n");
            exit(0);
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 'c'\n"), 1);
            int val;
            if (!parse_positive_int(argv[i + 1], &val) || val <= 0)
                return (fprintf(stderr, "ft_ping: bad number of packets to transmit\n"), 1);
            args->count = val;
            i += 2;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 's'\n"), 1);
            int val;
            if (!parse_positive_int(argv[i + 1], &val) || val < 0 || val > 65507)
                return (fprintf(stderr, "ft_ping: bad packet size\n"), 1);
            args->packet_size = val;
            i += 2;
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 't'\n"), 1);
            int val;
            if (!parse_positive_int(argv[i + 1], &val) || val <= 0)
                return (fprintf(stderr, "ft_ping: bad ttl value\n"), 1);
            if (val < 1 || val > 255)
            {
                fprintf(stderr, "ft_ping: ttl out of range\n");
                return 1;
            }
            args->ttl = val;
            i += 2;
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            args->numeric = true;
            i++;
        }
        else if (strcmp(argv[i], "-W") == 0)
        {
            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 'W'\n"), 1);
            int val;
            if (!parse_positive_int(argv[i + 1], &val) || val <= 0)
                return (fprintf(stderr, "ft_ping: bad timeout value\n"), 1);
            args->timeout_ms = val * 1000;
            i += 2;
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 'w'\n"), 1);
            int val;
            if (!parse_positive_int(argv[i + 1], &val) || val <= 0)
                return (fprintf(stderr, "ft_ping: bad deadline value\n"), 1);
            args->deadline_sec = val;
            i += 2;
        }
        else
        {
            if (argv[i][1] != '\0')
                fprintf(stderr, "ft_ping: invalid option -- '%c'\n", argv[i][1]);
            else
                fprintf(stderr, "ft_ping: invalid option -- '%s'\n", argv[i]);
            return 1;
        }
    }

    if (!target_set || !args->target)
    {
        fprintf(stderr, "ft_ping: missing host operand\n");
        return 1;
    }

    return 0;
}
