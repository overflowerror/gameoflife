LFLAGS = 
CFLAGS =

all: gif

gif: LFLAGS += -lgif
gif: CFLAGS += -DGIF_SUPPORT
gif: gameoflife

gameoflife: src/main.o src/field.o src/misc.o src/cli.o src/gif.o
	gcc $(LFLAGS) -o $@ $^

src/main.o: src/field.h src/misc.h src/cli.h
src/field.o: src/field.h src/misc.h
src/misc.o: src/misc.h
src/cli.o: src/field.h

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	@rm src/*.o
	@rm gameoflife
