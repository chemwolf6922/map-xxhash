CFLAGS?=-O3
CFLAGS+=-MMD -MP
LDFLAGS?=
LIBXXH=xxHash/libxxhash.a
LIB_SRC=map.c
TEST_SRC=test.c $(LIB_SRC)

HASH_DEP=
ifndef USE_SIMPLE_HASH
HASH_DEP=$(LIBXXH)
else
CFLAGS+=-DUSE_SIMPLE_HASH
endif

all:test lib

test:$(patsubst %.c,%.o,$(TEST_SRC)) $(HASH_DEP)
	$(CC) $(LDFLAGS) -o $@ $^

lib:libmap.a

libmap.a:$(patsubst %.c,%.o,$(LIB_SRC)) $(HASH_DEP)
ifndef USE_SIMPLE_HASH
	$(AR) -x $(HASH_DEP)
endif
	$(AR) -rcs -o $@ *.o

$(LIBXXH):xxHash
	$(MAKE) -C $< libxxhash.a

%.o:%.c
	$(CC) $(CFLAGS) -c $<

-include $(TEST_SRC:.c=.d)

clean:
	$(MAKE) -C xxHash clean
	rm -f *.o *.d test libmap.a 
