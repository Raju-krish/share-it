TARGET = shareit
CFLAGS = -Wall -ggdb3

all: 
	@gcc $(CFLAGS) *.c -o $(TARGET) -lpthread
	@echo "$(TARGET) Compiled successfully !"

clean:
	@rm -f $(TARGET)
	@echo "$(TARGET) cleaned !"
