include ../Makefile.inc

LIBS=libstm32f4.a \
     liblwip.a

all: $(LIBS)

libstm32f4.a:
	$(MAKE) -C stm32f4

liblwip.a:
	$(MAKE) -C lwip

clean:
	rm -f $(LIB_FILES)
	$(MAKE) -C stm32f4 clean
	$(MAKE) -C lwip clean
