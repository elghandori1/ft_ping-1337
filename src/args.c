#include "../includes/ft_ping.h"

static void init_args(t_args *args)
{
    memset(args, 0, sizeof(t_args));
    args->ttl = DEFAULT_TTL;
    args->size = PACKET_SIZE;
    args->timeout_ms = DEFAULT_TIMEOUT_MS;
    args->deadline_sec = -1;
    args->target = NULL;
    args->verbose = 0;
    args->count = 0;
    args->numeric = false;
}

static int parse_positive_int(const char *str, int *out)
{
    char *endptr;
    long val;

    if (!str || !*str)
        return 0;
    errno = 0;
    val = strtol(str, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || val < 0 || val > INT_MAX)
        return 0;
    *out = (int)val;
    return 1;
}

void print_usage(void)
{
    printf("Usage: ft_ping [-c count] [-v] [-s size] [-n] [-W timeout] [-w deadline] [-?] destination\n");
    printf("\n");
    printf("  -v              Verbose mode (show packet errors)\n");
    printf("  -?              Show this help message\n");
    printf("\n");
    printf("  -c <count>      Stop after <count> packets (default: unlimited)\n");
    printf("  -s <size>       Set packet size (default: 56, range: 0-65507)\n");
    printf("  -n              Disable DNS resolution (numeric output only)\n");
    printf("  -W <timeout>    Timeout for each reply in seconds (default: 1)\n");
    printf("  -w <deadline>   Exit after <deadline> seconds total\n");
}

int parse_args(int argc, char **argv, t_args *args)
{
    int i;
    int target_set;

    init_args(args);
    if (argc < 2)
    {
        fprintf(stderr, "ft_ping: missing host operand\n");
        return 1;
    }
    i = 1;
    target_set = 0;
    while (i < argc)
    {
        if (strcmp(argv[i], "--") == 0)
        {
            i++;
            if (i >= argc)
            {
                fprintf(stderr, "ft_ping: missing host operand\n");
                return 1;
            }
            if (target_set)
            {
                fprintf(stderr, "ft_ping: unexpected argument\n");
                return 1;
            }
            args->target = argv[i];
            target_set = 1;
            i++;
            continue;
        }
        if (strcmp(argv[i], "-") == 0)
        {
            if (target_set)
            {
                fprintf(stderr, "ft_ping: unexpected argument\n");
                return 1;
            }
            args->target = argv[i];
            target_set = 1;
            i++;
            continue;
        }
        if (argv[i][0] != '-')
        {
            if (target_set)
            {
                fprintf(stderr, "ft_ping: unexpected argument\n");
                return 1;
            }
            args->target = argv[i];
            target_set = 1;
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
            printf("ft_ping (GNU inetutils) 2.0\n");
            exit(0);
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            int val;

            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 'c'\n"), 1);
            if (!parse_positive_int(argv[i + 1], &val))
                return (fprintf(stderr, "ft_ping: bad number of packets to transmit\n"), 1);
            args->count = val;
            i += 2;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            int val;

            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 's'\n"), 1);
            if (!parse_positive_int(argv[i + 1], &val) || val < 0 || val > 65507)
                return (fprintf(stderr, "ft_ping: bad packet size\n"), 1);
            args->size = val;
            i += 2;
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            args->numeric = true;
            i++;
        }
        else if (strcmp(argv[i], "-W") == 0)
        {
            int val;

            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 'W'\n"), 1);
            if (!parse_positive_int(argv[i + 1], &val) || val <= 0)
                return (fprintf(stderr, "ft_ping: bad timeout value\n"), 1);
            args->timeout_ms = val * 1000;
            i += 2;
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            int val;

            if (i + 1 >= argc)
                return (fprintf(stderr, "ft_ping: option requires an argument -- 'w'\n"), 1);
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
