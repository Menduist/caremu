NAME=caremu

SRCS=caremu.c gpu.c cars.c maps.c input.c

LDLOCS=-Llibtca
LDINCS=-Ilibtca

HEADERS=caremu.h

C=gcc
LDLIBS=-ltca -lm -lcsfml-graphics -lcsfml-window -lcsfml-system
CFLAGS=-Wall -Wextra $(LDLIBS) $(LDLIBS) $(LDLOCS) -O5 -g `xml2-config --cflags --libs`

all: dependencies $(NAME)

dependencies: $(SRCS) $(HEADERS)
	$(C) -MM $(SRCS) > dependencies

-include dependencies

$(NAME): $(SRCS) $(HEADERS) libtca
	$(C) $(SRCS) $(CFLAGS) -o $(NAME)

libtca/libtca.a: libtca
	make -C libtca

clean:

fclean: clean
	rm $(NAME)

re: fclean all


.PHONY: all clean fclean re
