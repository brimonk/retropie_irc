CC = cc
FLAGS = -Wall
DEBUG = -gdwarf
TARGET = brimbot
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

LIB_FLAGS = -shared -fPIC
LIB_SOURCES = $(wildcard src/lib/*.c)
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

all: $(TARGET)

src/%.o: src/%.c # rule to compile each $(TARGET) object
	$(CC) -c $(DEBUG) $(FLAGS) -o $@ $<

src/lib/%.so: src/lib/%.c # rule to compile each $(LIB) by itsel
	$(CC) -c $(DEBUG) $(LIB_FLAGS) -o $@ $<

clean: clean-obj clean-bin

clean-obj:
	rm -f $(OBJECTS) $(LIB_OBJECTS)
	
clean-bin:
	rm -f $(TARGET)
	
$(TARGET): $(OBJECTS) | $(LIB_OBJECTS)
	$(CC) $(DEBUG) $(FLAGS) -o $(TARGET) $(OBJECTS)

