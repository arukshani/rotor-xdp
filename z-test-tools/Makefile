CC = gcc

# CC flags:
#  -pthread links pthread library to the project
#  -g       adds debugging information to the executable file
#  -Wall    turns on most, but not all, CC warnings
OPTS = -pthread -g -Wall

# the build target executable:
# TARGET = udp_client udp_server
# TARGET = udp_client_fwd udp_server_fwd
TARGET = tcp_send_file tcp_server
# TARGET = udp_server

# TARGET = tcp_client tcp_server

default: $(TARGET)

clean:
	$(RM) count *.o *.gch *~ ../*.gch $(TARGET)