#include <opus/opus.h>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE       48000 /*  kHz  */
#define FRAMES_PER_BUFFER 960   /*  equals 20ms audio portion  */
#define PACKET_SIZE       508   /*  packet size to send over network  */

void chk_for_pa_err(PaError e);

int main(int argc, char **argv)
{
    OpusEncoder *encoder;
    OpusDecoder *decoder;
    PaStream *inp_stream, *out_stream;
    PaError pa_err;
    int err, encoded, decoded;
    unsigned char enc_packet[PACKET_SIZE];
    short enc_pcm[FRAMES_PER_BUFFER];
    short dec_pcm[FRAMES_PER_BUFFER];

    pa_err = Pa_Initialize();
    chk_for_pa_err(pa_err);
    pa_err = Pa_OpenDefaultStream(&inp_stream, 1, 0, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, NULL, NULL);
    chk_for_pa_err(pa_err);
    pa_err = Pa_StartStream(inp_stream);
    chk_for_pa_err(pa_err);

    pa_err = Pa_OpenDefaultStream(&out_stream, 0, 1, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, NULL, NULL);
    chk_for_pa_err(pa_err);
    pa_err = Pa_StartStream(out_stream);
    chk_for_pa_err(pa_err);


    encoder = opus_encoder_create(SAMPLE_RATE, 1, OPUS_APPLICATION_VOIP, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "error creating encoder: %s\n", opus_strerror(err));
        return err;
    }
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(24000));

    decoder = opus_decoder_create(SAMPLE_RATE, 1, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "error creating decoder: %s\n", opus_strerror(err));
        return err;
    }


    for(;;) {
        pa_err = Pa_ReadStream(inp_stream, enc_pcm, FRAMES_PER_BUFFER);
        /* buf now has raw audio bytes */
        chk_for_pa_err(pa_err);

        /* encode raw audio data */
        encoded = opus_encode(encoder, enc_pcm, FRAMES_PER_BUFFER, enc_packet, PACKET_SIZE);
        if (encoded < 0) {
            fprintf(stderr, "error encoding pcm: %s\n", opus_strerror(encoded));
            return encoded;
        }

        printf("Encoded %d bytes\n", encoded);

        decoded = opus_decode(decoder, enc_packet, encoded, dec_pcm, FRAMES_PER_BUFFER, 0);
        if (decoded < 0) {
            fprintf(stderr, "error decoding data: %s\n", opus_strerror(decoded));
            return decoded;
        }

        printf("Decoded %d bytes\n", decoded);
        Pa_WriteStream(out_stream, dec_pcm, decoded);
    }
    Pa_StopStream(out_stream);
    Pa_StopStream(inp_stream);
    return 0;
}

void chk_for_pa_err(PaError e)
{
    if (e != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(e));
        Pa_Terminate();
        exit(e);
    }

}
