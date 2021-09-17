CFLAGS?=-O3
CFLAGS+=-MMD -MP
LDFLAGS?=
LIBXXH=xxHash/libxxhash.a
LIB_SRC=map.c
TEST_SRC=test.c $(LIB_SRC)

all:test lib

test:$(patsubst %.c,%.o,$(TEST_SRC)) $(LIBXXH)
	$(CC) $(LDFLAGS) -o $@ $^

lib:libmap.a

libmap.a:$(patsubst %.c,%.o,$(LIB_SRC)) $(LIBXXH)
	$(AR) -x $(LIBXXH)
	$(AR) -rcs -o $@ *.o

$(LIBXXH):xxHash
	$(MAKE) -C $< libxxhash.a CFLAGS=-Os

%.o:%.c
	$(CC) $(CFLAGS) -c $<

-include $(TEST_SRC:.c=.d)

clean:
	$(MAKE) -C xxHash clean
	rm -f *.o *.d test libmap.a 
