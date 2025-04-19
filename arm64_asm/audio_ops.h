#ifndef AUDIO_OPS_H
#define AUDIO_OPS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

void fft_arm64(float *real, float *imag, size_t n, int inverse);

int read_wav_arm64(const char *filename, float **samples, size_t *sample_count, int *sample_rate);

void apply_filter_arm64(float *samples, size_t count, const float *filter, size_t filter_size);

#ifdef __cplusplus
}
#endif

#endif //AUDIO_OPS_H