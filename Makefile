EXEC		= 	ft_ping

CC			= 	gcc

CFLAGS		= 	 -Wall -Wextra -Werror

SRC			= 	main\
				init\
				handle_args\
				sig_handler\

OBJ			= 	$(addsuffix .o, $(SRC))

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -rf $(EXEC)

re: fclean all

.PHONY: all clean fclean re


