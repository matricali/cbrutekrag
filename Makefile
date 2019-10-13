CC	= gcc

CFLAGS	+= -Wall -g -std=gnu99 -O3 -Iinclude
LDFLAGS	+= -lpthread -lssh

NAME	= cbrutekrag
SRCS	:= cbrutekrag.c log.c str.c wordlist.c iprange.c progressbar.c bruteforce_ssh.c detection.c
OBJS	:= $(SRCS:%.c=obj/%.o)

all: dirs $(NAME)

dirs:
	mkdir -p obj

$(NAME): $(OBJS)
	@$(CC) $(OBJS) $(LDFLAGS) -o $@
	@echo "Linking complete!"

$(OBJS): obj/%.o : src/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

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
