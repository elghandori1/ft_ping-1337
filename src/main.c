#include "../includes/ft_ping.h"

int main(int argc, char **argv)
{
    t_args args;
    struct sockaddr_in addr;
    char ipstr[INET_ADDRSTRLEN];

    if (parse_args(argc, argv, &args) != 0)
        return EXIT_FAILURE;
    if (DNS_LookUp(args.target, &addr, ipstr, sizeof(ipstr)) != 0)
        return EXIT_FAILURE;
    run_ping(&args, &addr, ipstr);
    return EXIT_SUCCESS;
}
