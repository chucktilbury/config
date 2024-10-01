
TARGET	=	conf
OBJS	=	memory.o \
			str.o \
			strlist.o \
			hash.o \
			scan_file.o \
			parse_file.o \
			cmdline.o \
			config.o \
			test.o
DEBS	=	-DUSE_TRACE
CARGS	=	-Wall -Wextra -Wpedantic -pedantic

.c.o:
	gcc $(CARGS) -c -g -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc -o $@ $(OBJS)

clean:
	-rm -f $(TARGET) $(OBJS)
