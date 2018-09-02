CC	= gcc

CFLAGS	+= -Wall -g -std=gnu99 -O3
LDFLAGS	+= -lssh

NAME	= cbrutekrag
SRCS	= cbrutekrag.c log.c str.c wordlist.c iprange.c progressbar.c bruteforce_ssh.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: install
install: $(NAME)
	mkdir -p $(DESTDIR)$(PREFIX)/$(BINDIR)
	cp $(NAME) $(DESTDIR)$(PREFIX)/$(BINDIR)/$(NAME)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(BINDIR)/$(NAME)
