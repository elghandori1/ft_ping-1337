NAME		= ft_ping
CC			= cc
CFLAGS		= -Wall -Wextra -Werror -Iincludes
RM			= rm -f

SRCS_DIR	= src
SRCS		= $(SRCS_DIR)/main.c \
			  $(SRCS_DIR)/args.c \
			  $(SRCS_DIR)/utils.c \
			  $(SRCS_DIR)/ping.c
OBJS		= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
			$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -lm

%.o: %.c
			$(CC) $(CFLAGS) -c $< -o $@

clean:
			$(RM) $(OBJS)

fclean: clean
			$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re