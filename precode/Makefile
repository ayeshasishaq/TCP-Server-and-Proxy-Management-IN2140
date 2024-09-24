#
# The recommended compile flags.
#
# CFLAGS=-g -Wall -Wextra -std=gnu99
CFLAGS=-g -Wall -std=gnu99
VFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --malloc-fill=0x40 --free-fill=0x23
#
# The following 4 programs are created
#
all: proxy xmlSender binSender anyReceiver

#
# Linkning the 4 programs
#
proxy: proxy.o connection.o record.o recordToFormat.o recordFromFormat.o
	gcc $(CFLAGS) -o proxy proxy.o connection.o record.o recordToFormat.o recordFromFormat.o

xmlSender: xmlSender.o connection.o xmlfile.o
	gcc $(CFLAGS) -o xmlSender xmlSender.o connection.o xmlfile.o

binSender: binSender.o connection.o binfile.o
	gcc $(CFLAGS) -o binSender binSender.o connection.o binfile.o

anyReceiver: anyReceiver.o connection.o
	gcc $(CFLAGS) -o anyReceiver anyReceiver.o connection.o

#
# Compiling from C files to object files
#
xmlSender.o: xmlSender.c connection.h xmlfile.h
	gcc $(CFLAGS) -c $<

binSender.o: binSender.c connection.h binfile.h
	gcc $(CFLAGS) -c $<

anyReceiver.o: anyReceiver.c connection.h
	gcc $(CFLAGS) -c $<

connection.o: connection.c connection.h
	gcc $(CFLAGS) -c $<

xmlfile.o: xmlfile.c xmlfile.h
	gcc $(CFLAGS) -c $<

binfile.o: binfile.c binfile.h record.h
	gcc $(CFLAGS) -c $<

record.o: record.c record.h
	gcc $(CFLAGS) -c $<

recordToFormat.o: recordToFormat.c recordToFormat.h record.h
	gcc $(CFLAGS) -c $<

recordFromFormat.o: recordFromFormat.c recordFromFormat.h record.h
	gcc $(CFLAGS) -c $<

proxy.o: proxy.c connection.h record.h
	gcc $(CFLAGS) -c $<

valgrind:
	valgrind $(VFLAGS) ./proxy 2022

#
# Deleting the only object files
#
clean:
	rm -f *.o

#
# Deleting the object files and binary files
#
realclean: clean
	rm -f xmlSender
	rm -f binSender
	rm -f anyReceiver
	rm -f X.xml
	rm -f X.bin
	rm -f Y.bin
	rm -f Z.bin
	rm -f W.bin
	rm -f Y.xml

