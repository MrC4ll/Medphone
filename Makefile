CC = gcc
AS = aarch64-linux-gnu-as
CFLAGS = -O3 -march=armv8-a+simd -Wall `pkg-config --cflags gtk4`
LDFLAGS = `pkg-config --libs gtk4` -lm

TARGET = wav_analyzer
ASM_TARGET = libaudioops.a
COMM_TARGET = esp32_daemon

SRC = src/main.c src/wave_view.c src/fft_view.c
ASM_SRC = arm64_asm/audio_ops.S
COMM_SRC = esp32_comm/esp32_daemon.c

OBJ = $(SRC:.c=.o)
ASM_OBJ = $(ASM_SRC:.S=.o)

all: $(TARGET) $(COMM_TARGET)

$(TARGET): $(OBJ) $(ASM_TARGET)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(ASM_TARGET) $(LDFLAGS)

$(ASM_TARGET): $(ASM_OBJ)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(AS) -o $@ $<

$(COMM_TARGET): $(COMM_SRC)
	$(CC) $(CFLAGS) -o $@ $< -lbluetooth

clean:
	rm -f $(OBJ) $(ASM_OBJ) $(TARGET) $(ASM_TARGET) $(COMM_TARGET)

.PHONY: all clean