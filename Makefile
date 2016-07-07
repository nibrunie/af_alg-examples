
CFLAGS += -Wall -Werror 
LFLAGS += -lcrypto

TEST_LIST = basic_cipher stream_hash

# ARG 1: test name
define gen_test_rule
$(1).build: examples/$(1).c
	$(CC) $(CFLAGS) -o $(1) examples/$(1).c $(LFLAGS)

$(1).run: $(1).build
	./$(1)

$(1).clean: 
	rm -f ./$(1)

test: $(1).run

build: $(1).build

clean: $(1).clean

endef

$(foreach test,$(TEST_LIST),$(eval $(call gen_test_rule,$(test))))


.PHONY: test build clean
