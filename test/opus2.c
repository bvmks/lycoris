#include <opus/opus.h>
#include <portaudio.h>
#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE         48000
#define FRAMES_PER_BUFFER   960
#define PACKET_SIZE         3000
#define CHUNKS_AMT          100

// todo: подумать как накапливать несколько порций в пакете и проигрывать их (при этом не затирать данные и следить чтобы)

typedef struct {
    OpusEncoder *encoder;
    OpusDecoder *decoder;
    unsigned char packet[PACKET_SIZE];
    int chunks_writ;
    int chunks_read;
    int offset;
    int read;
    int ready;
} AudioData;

static int recordCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags, void *userData) 
{
    AudioData *data = (AudioData*)userData;
    const short *in = (const short*)inputBuffer;
    static unsigned char buf[PACKET_SIZE];
    static int enc = 0;

    if (data->ready)
        return paContinue;

    if (enc == 0)
        enc = opus_encode(data->encoder, in, framesPerBuffer, buf, PACKET_SIZE);
    
    if (enc > 0) {
        if (data->offset + enc + sizeof(int) > PACKET_SIZE) {
            printf("Encoded piece is too big to fit (%d bytes), skipping...\n", enc);
            data->ready = 1;
            return paContinue;
        }
        printf("Encoded %d byte chunk. ", enc);
        memcpy(data->packet + data->offset, &enc, sizeof(int));
        data->offset += sizeof(int);

        memcpy(data->packet + data->offset, buf, enc);
        data->offset += enc;

        data->chunks_writ++;
        printf("Chunks written: %d\n", data->chunks_writ);

        if (data->chunks_writ >= CHUNKS_AMT)
            data->ready = 1;
    }

    enc = 0;
    return paContinue;
}

static int playCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags, void *userData) 
{
    AudioData *data = (AudioData*)userData;
    short *out = (short*)outputBuffer;
    int l, dec;

    if (data->ready && data->chunks_read < data->chunks_writ) {
        memcpy(&l, data->packet + data->read, sizeof(int));
        data->read += sizeof(int);
        printf("Trying to read chunk %d (%d bytes)...\n", data->chunks_read + 1, l);
        dec = opus_decode(data->decoder, data->packet + data->read, l, out, framesPerBuffer, 0);
        if (dec < 0)
            memset(out, 0, framesPerBuffer * sizeof(short));
        data->read += l;
        data->chunks_read++;

        if (data->chunks_read >= data->chunks_writ) {
            data->ready = 0;
            data->offset = 0;
            data->chunks_read = 0;
            data->chunks_writ = 0;
            data->read = 0;
        }
    }   
    else {
        memset(out, 0, framesPerBuffer * sizeof(short));
    }

    return paContinue;
}

int main() {
    AudioData data;
    PaStream *inStream, *outStream;
    int err;
    memset(&data, 0, sizeof(AudioData));
    data.encoder = opus_encoder_create(SAMPLE_RATE, 1, OPUS_APPLICATION_VOIP, &err);
    data.decoder = opus_decoder_create(SAMPLE_RATE, 1, &err);
    opus_encoder_ctl(data.encoder, OPUS_SET_BITRATE(24000));

    Pa_Initialize();

    Pa_OpenDefaultStream(&inStream, 1, 0, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, recordCallback, &data);
    Pa_OpenDefaultStream(&outStream, 0, 1, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, playCallback, &data);

    Pa_StartStream(inStream);
    Pa_StartStream(outStream);

    getchar(); 

    Pa_StopStream(inStream);
    Pa_StopStream(outStream);
    Pa_Terminate();

    return 0;
}
