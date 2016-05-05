include ../Makefile.inc

# enable time units in libscpi
CFLAGS+=-DUSE_UNITS_TIME=1

# pass all defined variables down to the recursive make call (e.g. CFLAGS)
export

LIBS=libstm32f4.a \
     libtm.a \
     liblwip.a \
     libscpi.a

all: $(LIBS)

libstm32f4.a:
	$(MAKE) -C stm32f4
	cp -f stm32f4/$@ .

liblwip.a:
	$(MAKE) -C lwip
	cp -f lwip/$@ .

libtm.a:
	$(MAKE) -C tm
	cp -f tm/$@ .

libscpi.a:
	$(MAKE) -C scpi-parser/libscpi static
	cp -f scpi-parser/libscpi/dist/$@ .

clean:
	rm -f $(LIBS)
	$(MAKE) -C stm32f4 clean
	$(MAKE) -C lwip clean
	$(MAKE) -C tm clean
	$(MAKE) -C scpi-parser clean
