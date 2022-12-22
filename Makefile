CFLAGS?=-O3
override CFLAGS+=-MMD -MP
LDFLAGS?=
LIBXXH=xxHash/libxxhash.a
LIB_SRC=map.c
TEST_SRC=test.c
ALL_SRC=$(LIB_SRC) $(TEST_SRC)

HASH_DEP=
ifndef USE_SIMPLE_HASH
HASH_DEP=$(LIBXXH)
else
override CFLAGS+=-DUSE_SIMPLE_HASH
endif

all:test lib

test:$(patsubst %.c,%.oo,$(TEST_SRC)) $(patsubst %.c,%.o,$(LIB_SRC)) $(HASH_DEP)
	$(CC) $(LDFLAGS) -o $@ $^

lib:libmap.a

libmap.a:$(patsubst %.c,%.o,$(LIB_SRC)) $(HASH_DEP)
ifndef USE_SIMPLE_HASH
	$(AR) -x $(HASH_DEP)
endif
	$(AR) -rcs $@ *.o

$(LIBXXH):xxHash
	$(MAKE) -C $< libxxhash.a

%.oo:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o:%.c
	$(CC) $(CFLAGS) -c $<

-include $(ALL_SRC:.c=.d)

clean:
	$(MAKE) -C xxHash clean
	rm -f *.oo *.o *.d test libmap.a 
