complie:
    gcc demon.c filelib.c filelib.h -o Demon

test:
    ./test.sh

signal:
    killall -s SIGUSR1 Demon

kill:
    killall Demon

.PHONY: clean

clean:
    rm demon.c filelib.c filelib.h Demon