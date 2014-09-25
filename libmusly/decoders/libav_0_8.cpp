/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *                2014, Jan Schlueter <jan.schlueter@ofai.at>
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
#ifdef HAVE_AVUTIL_CHANNEL_LAYOUT
    #include <libavutil/channel_layout.h>
#endif
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
        const int out_stride,
        const int in_stride,
        const AVSampleFormat in_fmt,
        int len)
{
    const int is = in_stride;
    const int os = out_stride;
    const uint8_t* pi = (uint8_t*)in;
    uint8_t* po = (uint8_t*)out;
    uint8_t* end = po + os*len;

#define CONVFLOAT(ifmt, ifmtp, expr)\
if (in_fmt == ifmt || in_fmt == ifmtp) {\
    do{\
        *(float*)po = expr; pi += is; po += os;\
    } while(po < end);\
}
    // Implementation note: We could use av_get_packed_sample_fmt
    // to avoid checking for two formats each time, but we want to
    // stay compatible to libav versions that do not have it yet.
    CONVFLOAT(AV_SAMPLE_FMT_U8, AV_SAMPLE_FMT_U8P,
            (*(const uint8_t*)pi - 0x80)*(1.0 / (1<<7)))
    else CONVFLOAT(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16P,
            *(const int16_t*)pi*(1.0 / (1<<15)))
    else CONVFLOAT(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S32P,
            *(const int32_t*)pi*(1.0 / (1U<<31)))
    else CONVFLOAT(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLTP,
            *(const float*)pi)
    else CONVFLOAT(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_DBLP,
            *(const double*)pi)
    else return -1;

    return 0;
}

std::vector<float>
libav_0_8::decodeto_22050hz_mono_float(
        const std::string& file,
        float excerpt_length,
        float excerpt_start)
{
    MINILOG(logTRACE) << "Decoding: " << file << " started.";

    const int target_rate = 22050;

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
    // (kindly ask for stereo downmix and floats, but not all decoders care)
    decx->request_channel_layout = AV_CH_LAYOUT_STEREO_DOWNMIX;
    decx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
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
    const int input_stride = av_get_bytes_per_sample(decx->sample_fmt);
    const int num_planes = av_sample_fmt_is_planar(decx->sample_fmt) ? decx->channels : 1;
    const int output_stride = sizeof(float) * num_planes;
    int decode_samples;  // how many samples to decode; zero to decode all

    if (st->duration) {  // if the file length is (at least approximately) known:
        float file_length = (float)st->duration * st->time_base.num / st->time_base.den;
        MINILOG(logDEBUG) << "Audio file length: " << file_length << " seconds";

        // adjust excerpt boundaries
        if ((excerpt_length <= 0) || (excerpt_length > file_length)) {
            // use full file
            excerpt_length = 0;
            excerpt_start = 0;
        }
        else if (excerpt_start < 0) {
            // center in file, but start at -excerpt_start the latest
            excerpt_start = std::min(-excerpt_start,
                    (file_length - excerpt_length) / 2);
        }
        else if (excerpt_start + excerpt_length > file_length) {
            // right-align excerpt
            excerpt_start = file_length - excerpt_length;
        }
        MINILOG(logTRACE) << "Will decode from " << excerpt_start << " to " <<
                (excerpt_length > 0 ? (excerpt_start + excerpt_length) : file_length);

        // try to skip to requested position in stream
        if ((excerpt_start > 0) and (av_seek_frame(fmtx, audio_stream_idx,
                    excerpt_start * st->time_base.den / st->time_base.num,
                    AVSEEK_FLAG_ANY) >= 0)) {
            // skipping went fine: decode only what's needed
            decode_samples = excerpt_length * decx->sample_rate;
            excerpt_start = 0;
            avcodec_flush_buffers(decx);
        }
        else {
            if (excerpt_start > 0) {
                MINILOG(logDEBUG) << "Could not seek in audio file.";
            }
            // skipping failed or not needed: decode from beginning
            decode_samples = (excerpt_start + excerpt_length) * decx->sample_rate;
        }
    }
    else {  // if the file length is unknown:
        MINILOG(logDEBUG) << "Audio file length: unknown";
        if (excerpt_length <= 0) {
            // use full file
            excerpt_length = 0;
            excerpt_start = 0;
            decode_samples = 0;
        }
        else if (excerpt_start < 0) {
            // center in file, but start at -excerpt_start the latest,
            // so decode at most -excerpt_start+excerpt_length seconds
            decode_samples = (-excerpt_start + excerpt_length) * decx->sample_rate;
        }
        else {
            // uncentered excerpt: decode from beginning, cut out afterwards
            decode_samples = (excerpt_start + excerpt_length) * decx->sample_rate;
        }
    }
    // After this lengthy adjustment, decode_samples tells us how many samples
    // to decode at most (where zero means to decode the full file), and
    // excerpt_start tells us up to how many seconds to cut from the beginning.

    // read packets
    float* buffer = NULL;
    int buffersize = 0;
    std::vector<float> decoded_pcm;
    int subsequent_errors = 0;
    const int subsequent_errors_max = 20;
    while (((decode_samples == 0) || ((int)decoded_pcm.size() < decode_samples))
            && (av_read_frame(fmtx, &pkt) >= 0))
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
                        buffersize = input_samples;
                    }

                    // convert samples to float
                    // If we have planar samples (num_planes > 1), the channels
                    // are stored in separate frame->data[i] arrays and we
                    // convert to interleaved samples on the way.
                    for (int i = 0; i < num_planes; i++) {
                        if (samples_tofloat(buffer + i, frame->data[i],
                                output_stride, input_stride,
                                decx->sample_fmt,
                                input_samples / num_planes) < 0) {
                            MINILOG(logERROR) << "Strange sample format. Abort.";

                            av_free(frame);
                            av_free_packet(&pkt);
                            avformat_close_input(&fmtx);
                            if (buffer) {
                                delete[] buffer;
                            }
                            return decoded_pcm;
                        }
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

    // cut out the requested excerpt if needed
    int skip_samples = 0;
    if (excerpt_start < 0) {
        // center excerpt, but start at -excerpt_start the latest
        float file_length = decoded_pcm.size() / decx->sample_rate;
        if (file_length > excerpt_length) {
            // skip beginning as needed
            excerpt_start = std::min(-excerpt_start,
                    (file_length - excerpt_length) / 2);
            skip_samples = excerpt_start * decx->sample_rate;
            // truncate end if needed
            int target_samples = skip_samples + excerpt_length * decx->sample_rate;
            if (target_samples < (int)decoded_pcm.size()) {
                decoded_pcm.resize(target_samples);
            }
        }
    }
    else if (excerpt_length > 0) {
        // truncate to target length if needed
        if ((int)decoded_pcm.size() > decode_samples) {
            decoded_pcm.resize(decode_samples);
        }
        // skip beginning if needed
        if (excerpt_start > 0) {
            skip_samples = excerpt_start * decx->sample_rate;
            int missed_samples = decode_samples - decoded_pcm.size();
            skip_samples = std::max(0, skip_samples - missed_samples);
        }
    }

    // do we need to resample?
    std::vector<float> pcm;
    if (target_rate != decx->sample_rate) {
        MINILOG(logTRACE) << "Resampling signal. input="
                << decx->sample_rate << ", target=" << target_rate;
        resampler r(decx->sample_rate, target_rate);
        pcm = r.resample(decoded_pcm.data() + skip_samples, decoded_pcm.size() - skip_samples);
        MINILOG(logTRACE) << "Resampling finished.";
    } else {
        pcm.resize(decoded_pcm.size() - skip_samples);
        std::copy(decoded_pcm.begin() + skip_samples, decoded_pcm.end(), pcm.begin());
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
