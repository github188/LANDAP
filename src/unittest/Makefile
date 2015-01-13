
include ../../Makefile.inc

UNITTESTDIR = $(PWD)
INCLUDEPATH = -I . -I $(PWD)/../../$(INCLUDE_DIRNAME)
CFLAGS := $(INCLUDEPATH) $(CFLAGS)

EXES = base64_test ts_queue_test tftp_test ipv6_serv switch_test srv_test ap_test basic_test

ap_test:
	$(CC) $(CFLAGS) -o $@ $@.c -L../../build -lddp -lddp_platform -lrt -lpthread

switch_test:
	$(CC) $(CFLAGS) -o $@ $@.c -L../../build -lddp -lddp_platform -lrt -lpthread

base64_test:
	$(CC) $(CFLAGS) -o base64_test base64_test.c ../$(DDP_DIRNAME)/base64.o

ts_queue_test:
	$(CC) $(CFLAGS) -o ts_queue_test ts_queue_test.c ../$(DDP_DIRNAME)/ts_queue.o

tftp_test:
	$(CC) $(CFLAGS) -o tftp_test tftp_test.c ../../build/libddp.a ../../build/libddp_platform.a -lrt -lpthread

ipv6_serv:
	$(CC) $(CFLAGS) -o ipv6_serv ipv6_serv.c -L../../build -lddp -lddp_platform -lrt -lpthread

srv_test:
	$(CC) $(CFLAGS) -o srv_test srv_test.c -L../../build -lddp -lddp_platform -lrt -lpthread

basic_test:
	$(CC) $(CFLAGS) -o $@ $@.c -L../../build -lddp -lddp_platform -lrt -lpthread

clean:
	-$(RM) -f $(UNITTESTDIR)/*.o $(UNITTESTDIR)/*.deps $(EXES)

.phony: clean

