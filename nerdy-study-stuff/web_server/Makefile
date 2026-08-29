# discover OS
OS := $(shell uname -s)

# default config
CC = gcc
CFLAGS = -Wall -Wextra
SRC = revolver.c
TARGET = revolver

# specific complication (based on OS)

# macOS
ifeq ($(OS), Darwin)
    $(info macOS detected. configuring OpenSSL with Homebrew)
    OPENSSL_DIR = $(shell brew --prefix openssl)
    CFLAGS += -I$(OPENSSL_DIR)/include -fsanitize=address -g
    LDFLAGS = -L$(OPENSSL_DIR)/lib -lssl -lcrypto
# linux
else
    $(info linux detected. using default libs)
    LDFLAGS = -lssl -lcrypto
endif

# compilation rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
