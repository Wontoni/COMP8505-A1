
all:
	gcc ./src/*.c -o stego -lpng

clean:
	rm -f stego

.PHONY: all clean
