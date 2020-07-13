###############################################################################
# Gather all flags.
#
CFLAGS+=-DPOSIXLY_CORRECT -Wall -O2 -std=c++11
CC=g++
TARGET=epcs

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o, $(SOURCES))

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS) $(LDFLAGS) -lpthread

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

