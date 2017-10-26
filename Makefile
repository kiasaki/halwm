NAME= halwm
CFLAGS= -Wall -Wextra -pedantic
CFLAGS+= -std=c99 -O2
LIBS= -lxcb
CC= cc
INSTALLPATH= ~/bin

FLAGS= $(CFLAGS) $(LIBS)

all: main

install: main
	cp $(NAME) $(INSTALLPATH)

uninstall: clean
	rm -f $(INSTALLPATH)/$(NAME)

main:
	$(CC) $(FLAGS) $(NAME).c -o $(NAME)

clean:
	rm -f $(NAME) *.core
