#include "audio_processing.h"
#include <pthread.h>
#include <fftw3.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <armadillo.h>

using namespace arma; 

// Thread-safe match storage
static uvec top5_matches(5);
static pthread_mutex_t match_mutex = PTHREAD_MUTEX_INITIALIZER;

void* esp32_communication_thread(void* user_data) {
    struct sockaddr_rc addr = {
        .rc_family = AF_BLUETOOTH,
        .rc_channel = 1  // RFCOMM default
    };
    str2ba(ESP32_BT_MAC, &addr.rc_bdaddr);

    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    float pcm_sample;
    while (threads_running) {
        ssize_t len = recv(sock, &pcm_sample, sizeof(float), 0);
        if (len > 0) {
            ring_buffer_write(pcm_sample);  // Lock-free write
        }
    }
    close(sock);
    return NULL;
}

// ... [Keep existing FFT processing code] ...

// Update matches (call from FFT thread)
static void update_matches(const vec &scores) {
    pthread_mutex_lock(&match_mutex);
    top5_matches = sort_index(scores, "descend").head(5);
    pthread_mutex_unlock(&match_mutex);
}

// Thread-safe access to matches
const uvec* audio_get_top5_matches(void) {
    pthread_mutex_lock(&match_mutex);
    static uvec result;  // Copy to avoid threading issues
    result = top5_matches;
    pthread_mutex_unlock(&match_mutex);
    return &result;
}