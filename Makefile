
CFLAGS += -Wall -Werror 

%.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $^

cipher_basic: examples/basic_cipher.o
	$(CC) $(CFLAGS) -o $@ $^ -lcrypto

build: cipher_basic

test: build
	./cipher_basic

clean: 
	rm -f ./cipher_basic ./examples/basic_cipher.o

.PHONY: test build clean
