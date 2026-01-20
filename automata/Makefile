CC   = gcc
CXX  = g++

CFLAGS   = -Werror -Wextra -g -std=c99
CXXFLAGS = -Werror -Wextra -g -std=c++17

RAYLIB_INC = -I./raylib-5.5_linux_amd64/include
RAYLIB_LIB = -L./raylib-5.5_linux_amd64/lib -l:libraylib.a \
             -lm -lpthread -ldl -lrt -lX11

C_BINS   = rule110 wireworld conway
CPP_BINS = smooth_life

BINS = $(C_BINS) $(CPP_BINS)

C_OBJS   = $(C_BINS:=.o)
CPP_OBJS = $(CPP_BINS:=.o)

all: $(BINS)

%.o: %.c
	$(CC) $(CFLAGS) $(RAYLIB_INC) -c $< -o $@

$(C_BINS): %: %.o
	$(CC) $< -o $@ $(RAYLIB_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(RAYLIB_INC) -c $< -o $@

$(CPP_BINS): %: %.o
	$(CXX) $< -o $@ $(RAYLIB_LIB)

clean:
	rm -f $(BINS) *.o

.PHONY: all clean

