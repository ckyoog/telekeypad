CFLAGS=`pkg-config gtk+-2.0 --cflags`
LDFLAGS=`pkg-config gtk+-2.0 --libs`

SRC=$(wildcard *.c)
EXEC=$(subst .c,,$(SRC))


all: $(EXEC)

$(EXEC): $(EXEC).o

clean:
	rm -f *.o $(EXEC)
