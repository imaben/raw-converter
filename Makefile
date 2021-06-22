CC=gcc
CFLAGS=`pkg-config --cflags libraw`
CLIBS=`pkg-config --libs libraw`

main:
	$(CC) -o rc rc.cc $(CFLAGS) $(CLIBS)

clean:
	rm -rf rc
