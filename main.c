#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <complex.h>

// Define WAV file header structure
typedef struct {
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char subchunk2ID[4];
    uint32_t subchunk2Size;
} WAVHeader;

// FFT functions from our ARM64 assembly
void fft_arm64(float complex *output, const float *input, size_t n);
void fft_init_arm64(size_t n);
void fft_cleanup_arm64();

// Helper function to check if file exists
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Read WAV file and validate header
int read_wav_file(const char *path, WAVHeader *header, float **samples, size_t *sample_count) {
    if (!file_exists(path)) {
        fprintf(stderr, "Error: File does not exist\n");
        return -1;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    // Read header
    if (read(fd, header, sizeof(WAVHeader)) != sizeof(WAVHeader)) {
        perror("Error reading WAV header");
        close(fd);
        return -1;
    }

    // Validate WAV header
    if (memcmp(header->chunkID, "RIFF", 4) != 0 || 
        memcmp(header->format, "WAVE", 4) != 0 ||
        memcmp(header->subchunk1ID, "fmt ", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file\n");
        close(fd);
        return -1;
    }

    // We only support PCM format
    if (header->audioFormat != 1) {
        fprintf(stderr, "Error: Only PCM format supported\n");
        close(fd);
        return -1;
    }

    // We only support 16-bit samples
    if (header->bitsPerSample != 16) {
        fprintf(stderr, "Error: Only 16-bit samples supported\n");
        close(fd);
        return -1;
    }

    // Calculate number of samples
    size_t num_samples = header->subchunk2Size / (header->numChannels * (header->bitsPerSample / 8));
    *sample_count = num_samples;

    // Allocate memory for samples (we'll convert to mono and float)
    *samples = (float *)malloc(num_samples * sizeof(float));
    if (*samples == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return -1;
    }

    // Read and convert samples
    int16_t *raw_samples = malloc(header->subchunk2Size);
    if (raw_samples == NULL) {
        perror("Memory allocation failed");
        free(*samples);
        close(fd);
        return -1;
    }

    if (read(fd, raw_samples, header->subchunk2Size) != header->subchunk2Size) {
        perror("Error reading samples");
        free(raw_samples);
        free(*samples);
        close(fd);
        return -1;
    }

    // Convert to mono float samples (-1.0 to 1.0 range)
    for (size_t i = 0; i < num_samples; i++) {
        float sum = 0.0f;
        for (int ch = 0; ch < header->numChannels; ch++) {
            int16_t sample = raw_samples[i * header->numChannels + ch];
            sum += (float)sample / 32768.0f;
        }
        (*samples)[i] = sum / header->numChannels;
    }

    free(raw_samples);
    close(fd);
    return 0;
}

// Find the next power of two for FFT
size_t next_pow2(size_t n) {
    size_t pow2 = 1;
    while (pow2 < n) {
        pow2 <<= 1;
    }
    return pow2;
}

// Window function (Hamming window)
void apply_window(float *samples, size_t n) {
    for (size_t i = 0; i < n; i++) {
        samples[i] *= 0.54f - 0.46f * cosf(2 * M_PI * i / (n - 1));
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <wav_file_path>\n", argv[0]);
        return 1;
    }

    const char *wav_path = argv[1];
    WAVHeader header;
    float *samples;
    size_t sample_count;

    // Read WAV file
    if (read_wav_file(wav_path, &header, &samples, &sample_count) != 0) {
        return 1;
    }

    printf("WAV File Information:\n");
    printf("  Sample Rate: %u Hz\n", header.sampleRate);
    printf("  Channels: %u\n", header.numChannels);
    printf("  Bits per Sample: %u\n", header.bitsPerSample);
    printf("  Duration: %.2f seconds\n", (float)sample_count / header.sampleRate);
    printf("  Total Samples: %zu\n", sample_count);

    // Determine FFT size (next power of two up to 32768)
    size_t fft_size = next_pow2(sample_count);
    if (fft_size > 32768) {
        fft_size = 32768;
        printf("Warning: Truncating to first %zu samples for FFT\n", fft_size);
    }

    printf("Performing FFT with size %zu\n", fft_size);

    // Initialize ARM64 FFT
    fft_init_arm64(fft_size);

    // Apply window function
    apply_window(samples, fft_size);

    // Prepare output buffer
    float complex *fft_output = (float complex *)malloc(fft_size * sizeof(float complex));
    if (fft_output == NULL) {
        perror("Memory allocation failed");
        free(samples);
        return 1;
    }

    // Perform FFT using ARM64 assembly
    fft_arm64(fft_output, samples, fft_size);

    // Calculate and print magnitude spectrum (first 10 bins)
    printf("\nFirst 10 FFT bins:\n");
    printf("Bin\tFrequency (Hz)\tMagnitude\n");
    for (size_t i = 0; i < 10 && i < fft_size / 2; i++) {
        float freq = (float)i * header.sampleRate / fft_size;
        float magnitude = cabsf(fft_output[i]);
        printf("%zu\t%.2f\t\t%.6f\n", i, freq, magnitude);
    }

    // Cleanup
    free(samples);
    free(fft_output);
    fft_cleanup_arm64();

    return 0;
}