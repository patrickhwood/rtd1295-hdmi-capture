Debug = 1

LDFLAGS = -lpthread
CFLAGS = -Wall
INC_DIR = 
EXEC = cap

ifeq ($(Debug), 1)
 CFLAGS += -O0 -g -DDEBUG
else
 CFLAGS += -O3 -g
endif

.PHONY = all clean

all:$(EXEC)

$(EXEC): v4l2_driver.o main.c
	cc $^ $(CFLAGS) $(INC_DIR) $(LDFLAGS) -o $@ 

v4l2_driver.o: v4l2_driver.c v4l2_driver.h
	cc $(CFLAGS) -o $@ -c v4l2_driver.c

clean:
	rm -f *.o $(EXEC)
