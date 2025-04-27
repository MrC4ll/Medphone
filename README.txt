sudo apt install build-essential libgtk-4-dev libfftw3-dev libarmadillo-dev libbluetooth-dev

g++ -O3 -march=native -o spectrogram main.c audio_processing.cpp \
    -lgtk-4 -lfftw3 -larmadillo -lpthread `pkg-config --cflags --libs gtk4`
    
./spectrogram

**PAIR ESP32 with LINUX
bluetoothctl
> power on
> scan on
> pair 00:12:34:56:78:9A
> trust 00:12:34:56:78:9A