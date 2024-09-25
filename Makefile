
TARGET	=	conf
OBJS	=	memory.o \
			str.o \
			hash.o \
			scan.o \
			parse.o \
			config.o
CARGS	=	-Wall -Wextra -Wpedantic -pedantic

.c.o: 
	gcc $(CARGS) -c -g -o $@ $<
	
all: $(TARGET)

$(TARGET): $(OBJS)
	gcc -o $@ $(OBJS)
	
clean:
	-rm -f $(TARGET) $(OBJS)
