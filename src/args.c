#include "../includes/ft_ping.h"

static void init_args(t_args *args)
{
    memset(args, 0, sizeof(t_args));
    args->target = NULL;
    args->verbose = 0;
}

void print_usage(void)
{
    printf("Usage: ft_ping [-v] [-?] destination\n");
    printf("  -v     verbose output\n");
    printf("  -?     print usage and exit\n");
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
