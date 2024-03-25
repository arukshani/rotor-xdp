CC = gcc

# CC flags:
#  -pthread links pthread library to the project
#  -g       adds debugging information to the executable file
#  -Wall    turns on most, but not all, CC warnings
OPTS = -pthread -g -Wall

# the build target executable:
target1 = udp_client
target2 = udp_server

all: target1 target2

default: $(target1)

clean:
	$(RM) count *.o *.gch *~ ../*.gch $(target1)