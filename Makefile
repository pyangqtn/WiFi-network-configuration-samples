# Compiler
CC = gcc 

# Compiler flags
CFLAGS = -Wall -g

# Directories for includes and libraries
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib -lcjson

# Source files
CLI_SRC_UDP = client_json_udp.c
CLI_SRC_TCP = client_json_tcp.c
SRV_SRC_UDP = server_json_udp.c
SRV_SRC_TCP = server_json_tcp.c
SMC_SRC_TEST = smc_test.c

# Object directory
OBJ_DIR = obj

# Object files
CLI_OBJ_UDP = $(OBJ_DIR)/client_json_udp.o
CLI_OBJ_TCP = $(OBJ_DIR)/client_json_tcp.o
SRV_OBJ_UDP = $(OBJ_DIR)/server_json_udp.o
SRV_OBJ_TCP = $(OBJ_DIR)/server_json_tcp.o
SMC_OBJ_TEST = $(OBJ_DIR)/smc_test.o

# Output executable
CLI_UDP = client_json_udp
SRV_UDP = server_json_udp
CLI_TCP = client_json_tcp
SRV_TCP = server_json_tcp
SMC_TEST = smc_test

# Targets
all: $(CLI_UDP) $(SRV_UDP) $(CLI_TCP) $(SRV_TCP) $(SMC_TEST)

# Ensure the object directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)


$(CLI_UDP): $(CLI_OBJ_UDP)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(CLI_UDP) $(CLI_OBJ_UDP) $(LIBS)

$(CLI_TCP): $(CLI_OBJ_TCP)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(CLI_TCP) $(CLI_OBJ_TCP) $(LIBS)

$(SRV_UDP): $(SRV_OBJ_UDP)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(SRV_UDP) $(SRV_OBJ_UDP) $(LIBS)

$(SRV_TCP): $(SRV_OBJ_TCP)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(SRV_TCP) $(SRV_OBJ_TCP) $(LIBS)

$(SMC_TEST): $(SMC_OBJ_TEST)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(SMC_TEST) $(SMC_OBJ_TEST)

# Compile source files into object files
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up the build
clean:
	rm -f $(CLI_UDP) $(SRV_UDP) $(CLI_TCP) $(SRV_TCP) $(SMC_TEST)
	rm -rf $(OBJ_DIR)

# Phony targets
.PHONY: all clean

