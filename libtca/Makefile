CC=gcc
NAME=libtca.a
CFLAGS=-O3 -g -I.
OBJSDIR=build
SRCS= \
    utils.c \
    hashtable.c \
    vector.c \
    intmap.c \
    iterator.c \
    dynarray.c \

OBJS=$(addprefix $(OBJSDIR)/,$(SRCS:.c=.o))

DEPS= \
    utils.h \
    hashtable.h \
    vector.h \
    intmap.h \
    iterator.h \
    dynarray.h \
    Makefile \

all: $(NAME)

$(OBJSDIR):
	mkdir $(OBJSDIR)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

$(OBJS): $(OBJSDIR)/%.o: %.c $(DEPS) | $(OBJSDIR)
	$(CC) -c $(CFLAGS) $< -o $@

fclean: clean
	rm -f $(NAME)
clean:
	rm -f $(OBJS)
