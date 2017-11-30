CC = cc
FLAGS = -Wall -Werror -g
TARGET = brimbot
OBJECTS = main.o irc.o

all: $(TARGET)

%.o: %.c
	$(CC) -g -c $(FLAGS) -o $@ $<

clean: clean-obj clean-bin

clean-obj:
	rm -rf *.o
	
clean-bin:
	rm -rf $(TARGET)
	
$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS)
