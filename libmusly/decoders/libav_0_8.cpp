/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include "minilog.h"
#include "resampler.h"
#include "libav_0_8.h"

namespace musly {
namespace decoders {

MUSLY_DECODER_REGIMPL(libav_0_8, 0);

libav_0_8::libav_0_8()
{
    av_register_all();
    avcodec_register_all();
}

int
libav_0_8::samples_tofloat(
        void* const out,
        const void* const in,
        const int in_stride,
        const AVSampleFormat in_fmt,
        int len)
{
    const int is = in_stride;
    const uint8_t* pi = (uint8_t*)in;
    uint8_t* po = (uint8_t*)out;
    uint8_t* end = po + sizeof(float)*len;

#define CONVFLOAT(ifmt, expr)\
if (in_fmt == ifmt) {\
    do{\
        *(float*)po = expr; pi += is; po += sizeof(float);\
    } while(po < end);\
}
    CONVFLOAT(AV_SAMPLE_FMT_U8, (*(const uint8_t*)pi - 0x80)*(1.0 / (1<<7)))
    else CONVFLOAT(AV_SAMPLE_FMT_S16, *(const int16_t*)pi*(1.0 / (1<<15)))
    else CONVFLOAT(AV_SAMPLE_FMT_S32, *(const int32_t*)pi*(1.0 / (1U<<31)))
    else CONVFLOAT(AV_SAMPLE_FMT_FLT, *(const float*)pi)
    else CONVFLOAT(AV_SAMPLE_FMT_DBL, *(const double*)pi)
    else return -1;

    return 0;
}

std::vector<float>
libav_0_8::decodeto_22050hz_mono_float(
        const std::string& file,
        int max_seconds)
{
    MINILOG(logTRACE) << "Decoding: " << file << " started.";

    int target_rate = 22050;

    // silence libav
    av_log_set_level(0);


    // guess input format
    AVFormatContext* fmtx = NULL;
    int avret = avformat_open_input(&fmtx, file.c_str(), NULL, NULL);
    if (avret < 0) {
        MINILOG(logERROR) << "Could not open file, or detect file format";
        return std::vector<float>(0);
    }

    // retrieve stream information
    if (avformat_find_stream_info(fmtx, NULL) < 0) {
        MINILOG(logERROR) << "Could not find stream info";

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }

    // if there are multiple audio streams, find the best one..
    int audio_stream_idx =
            av_find_best_stream(fmtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_stream_idx < 0) {
        MINILOG(logERROR) << "Could not find audio stream in input file";

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }
    AVStream *st = fmtx->streams[audio_stream_idx];

    // find a decoder for the stream
    AVCodecContext *decx = st->codec;
    AVCodec *dec = avcodec_find_decoder(decx->codec_id);
    if (!dec) {
        MINILOG(logERROR) << "Could not find codec.";

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }

    // open the decoder
    avret = avcodec_open2(decx, dec, NULL);
    if (avret < 0) {
        MINILOG(logERROR) << "Could not open codec.";

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }

    // Currently only mono and stereo files are supported.
    if ((decx->channels != 1) && (decx->channels != 2)) {
        MINILOG(logWARNING) << "Unsupported number of channels: "
                << decx->channels;

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }

    // check if we have a planar audio file
    if (av_sample_fmt_is_planar(decx->sample_fmt)) {
        MINILOG(logERROR) << "Unsupported sample format: planar";

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }

    // allocate a frame
    AVFrame* frame = avcodec_alloc_frame();
    if (!frame) {
        MINILOG(logWARNING) << "Could not allocate frame";

        avformat_close_input(&fmtx);
        return std::vector<float>(0);
    }

    // allocate and initialize a packet
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    int got_frame = 0;

    // configuration
    int input_stride = av_get_bytes_per_sample(decx->sample_fmt);

    // read packets
    float* buffer = NULL;
    int buffersize = 0;
    std::vector<float> decoded_pcm;
    int subsequent_errors = 0;
    int subsequent_errors_max = 20;
    while ((av_read_frame(fmtx, &pkt) >= 0) &&
            ((max_seconds == 0) ||
                    ((int)decoded_pcm.size() < (max_seconds*decx->sample_rate))))
   {
        // use only audio frames
        if (pkt.stream_index == audio_stream_idx) {
            uint8_t* data = pkt.data;
            int size = pkt.size;
            while (pkt.size > 0) {

                // try to decode a frame
                avcodec_get_frame_defaults(frame);

                int len = avcodec_decode_audio4(decx, frame, &got_frame, &pkt);
                if (len < 0) {
                    MINILOG(logWARNING) << "Error decoding an audio frame";

                    // allow some frames to fail
                    if (subsequent_errors < subsequent_errors_max) {
                        subsequent_errors++;
                        break;
                    }

                    // if too many frames failed decoding, abort
                    MINILOG(logERROR) << "Too many errors, aborting.";
                    av_free(frame);
                    av_free_packet(&pkt);
                    avformat_close_input(&fmtx);
                    if (buffer) {
                        delete[] buffer;
                    }
                    return std::vector<float>(0);
                } else {
                    subsequent_errors = 0;
                }

                // if we got a frame
                if (got_frame) {
                    // do we need to increase the buffer size?
                    int input_samples = frame->nb_samples*decx->channels;
                    if (input_samples > buffersize) {
                        if (buffer) {
                            delete[] buffer;
                        }
                        buffer = new float[input_samples];
                    }

                    // convert samples to float
                    if (samples_tofloat(buffer, frame->data[0], input_stride,
                            decx->sample_fmt, input_samples) < 0) {
                        MINILOG(logERROR) << "Strange sample format. Abort.";

                        av_free(frame);
                        av_free_packet(&pkt);
                        avformat_close_input(&fmtx);
                        if (buffer) {
                            delete[] buffer;
                        }
                        return decoded_pcm;
                    }

                    // inplace downmix to mono, if required
                    if (decx->channels == 2) {
                        for (int i = 0; i < frame->nb_samples; i++) {
                            buffer[i] = (buffer[i*2] + buffer[i*2+1]) / 2.0f;
                        }
                    }

                    // store raw pcm data
                    decoded_pcm.insert(decoded_pcm.end(), buffer,
                            buffer+frame->nb_samples);
                }

                // consume the packet
                pkt.data += len;
                pkt.size -= len;
            }
            pkt.data = data;
            pkt.size = size;
        }

        av_free_packet(&pkt);
    }
    MINILOG(logTRACE) << "Decoding loop finished.";

    // do we need to resample?
    std::vector<float> pcm;
    if (target_rate != decx->sample_rate) {
        MINILOG(logTRACE) << "Resampling signal. input="
                << decx->sample_rate << ", target=" << target_rate;
        resampler r(decx->sample_rate, target_rate);
        pcm = r.resample(decoded_pcm.data(), decoded_pcm.size());
        MINILOG(logTRACE) << "Resampling finished.";
    } else {
        pcm.resize(decoded_pcm.size());
        std::copy(decoded_pcm.begin(), decoded_pcm.end(), pcm.begin());
    }

    // cleanup
    if (buffer) {
        delete[] buffer;
    }
    av_free(frame);
    avcodec_close(decx);
    avformat_close_input(&fmtx);

    MINILOG(logTRACE) << "Decoding: " << file << " finalized.";
    return pcm;
}

} /* namespace decoders */
} /* namespace musly */
