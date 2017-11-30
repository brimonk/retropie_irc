CC = cc
FLAGS = -Wall -Werror -g
TARGET = brimbot
OBJECTS = main.o irc.o

all: $(TARGET)

%.o: %.c
	$(CC) -g -c $(FLAGS) -o $@ $<

clean: clean-obj clean-bin

clean-obj:
	rm -f *.o
	
clean-bin:
	rm -f $(TARGET)
	
$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS)
