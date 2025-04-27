#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <glib.h>
#include <stdbool.h>
#include <armadillo.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

using namespace arma;

//Armadillo reference lib PENDIENTE!!

//Bluetooth config
#define ESP32_BT_MAC "00:12:34:56:78:9A"  // Replace with ESP32's MAC

//Thread control
void start_audio_processing_threads(void);
void stop_audio_processing_threads(void);

//Shared DATA
extern float fft_magnitude[512];

//ESP32 Interlink
void* esp32_communication_thread(void* user_data);

//FFT processing
void* fft_processing_thread(void *user_data);

//Match results
const uvec* audio_get_top5_matches(void);

#endif