CC = gcc

# CC flags:
#  -pthread links pthread library to the project
#  -g       adds debugging information to the executable file
#  -Wall    turns on most, but not all, CC warnings
OPTS = -pthread -g -Wall

# the build target executable:
# target1 = udp_client
# target2 = udp_server

# all: target1 target2

target1_SRC=udp_client
target2_SRC=udp_server

all: target1 target2

target1: TARGET=target1
target2: TARGET=target2

target1: ; @echo $($(TARGET)_SRC)
target2: ; @echo $($(TARGET)_SRC)

default: $(TARGET)

clean:
	$(RM) count *.o *.gch *~ ../*.gch $(TARGET)