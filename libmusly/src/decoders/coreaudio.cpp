/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "coreaudio.h"
#include "resampler.h"
#include "minilog.h"

#include <algorithm>
#include <numeric>
#include <AudioToolbox/AudioToolbox.h>

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)

#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioFormat.h>

#else // __COREAUDIO_USE_FLAT_INCLUDES__

#include "CoreAudioTypes.h"
#include "AudioFile.h"
#include "AudioFormat.h"

#endif // __COREAUDIO_USE_FLAT_INCLUDES__


class AudioFile
{
public:
    AudioFile(const std::string& filename)
        : _filename(filename)
    {}

    ~AudioFile()
    {
        ExtAudioFileDispose(_audio_file);
    }

    operator ExtAudioFileRef()
    {
        return _audio_file;
    }

    bool open()
    {
        if (_audio_file)
        {
            ExtAudioFileDispose(_audio_file);
        }

        MINILOG(logTRACE) << "coreaudio: Decoding: " << _filename << " started.\n";

        OSStatus err;
        UInt32 size;
        SInt32 channel_map[1] = {0};
        AudioStreamBasicDescription input_format;
        AudioStreamBasicDescription output_format = {0};
        AudioConverterRef audio_converter;
        AudioConverterPrimeInfo prime_info = {0};
        SInt64 total_frame_count;

        CFStringRef url_str = CFStringCreateWithCString(kCFAllocatorDefault, _filename.c_str(), kCFStringEncodingUTF8);
        CFURLRef url_ref = CFURLCreateWithFileSystemPath(nullptr, url_str, kCFURLPOSIXPathStyle, false);

        err = ExtAudioFileOpenURL(url_ref, &_audio_file);
        CFRelease(url_ref);
        CFRelease(url_str);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: Could not open file";
            return false;
        }

        size = sizeof(input_format);
        err = ExtAudioFileGetProperty(_audio_file, kExtAudioFileProperty_FileDataFormat, &size, &input_format);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: Could not get input audio format";
            return false;
        }


        output_format.mSampleRate = input_format.mSampleRate;
        output_format.mFormatID = kAudioFormatLinearPCM;
        output_format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
        output_format.mChannelsPerFrame = std::min(2U, input_format.mChannelsPerFrame);
        output_format.mBytesPerFrame = sizeof(float) * output_format.mChannelsPerFrame;
        output_format.mFramesPerPacket = 1;
        output_format.mBytesPerPacket = output_format.mBytesPerFrame * output_format.mFramesPerPacket;
        output_format.mBitsPerChannel = sizeof(float) * 8;
        output_format.mReserved = 0;

        size = sizeof(output_format);
        err = ExtAudioFileSetProperty(_audio_file, kExtAudioFileProperty_ClientDataFormat, size, &output_format);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: Could not set output audio format";
            return false;
        }

        size = sizeof(total_frame_count);
        err = ExtAudioFileGetProperty(_audio_file, kExtAudioFileProperty_FileLengthFrames, &size, &total_frame_count);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: Could not get numer of frames in audio file" << input_format.mSampleRate;
            return false;
        }


        size = sizeof(AudioConverterRef);
        err = ExtAudioFileGetProperty(_audio_file, kExtAudioFileProperty_AudioConverter, &size, &audio_converter);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: Could not get audio converter";
            return false;
        }

        size = sizeof(AudioConverterPrimeInfo);
        err = AudioConverterGetProperty(audio_converter, kAudioConverterPrimeInfo, &size, &prime_info);
        if (err != kAudioConverterErr_PropertyNotSupported)
        {
            _offset = prime_info.leadingFrames;
        }

        if(input_format.mSampleRate != output_format.mSampleRate)
        {
            UInt32 outputBitRate = output_format.mSampleRate;
            AudioConverterSetProperty(audio_converter, kAudioConverterEncodeBitRate, sizeof(outputBitRate), &outputBitRate);
        }

        _duration = total_frame_count / static_cast<float>(output_format.mSampleRate);
        _channels = output_format.mChannelsPerFrame;
        _sample_rate = output_format.mSampleRate;

        seek(0.0);

        return true;
    }

    float duration() const
    {
        return _duration;
    }

    UInt32 channels() const
    {
        return _channels;
    }

    Float64 sample_rate() const
    {
        return _sample_rate;
    }

    bool seek(float position)
    {
        OSStatus err;
        SInt64 start = (position * _sample_rate) + _offset;
        err = ExtAudioFileSeek(_audio_file, start);
        if (err != noErr)
        {
            MINILOG(logERROR) << "coreaudio: failed to seek in audio file";
            return false;
        }

        return true;
    }

    SInt64 read(SInt64 samples, const float* target)
    {
        OSStatus err;
        float* buffer = const_cast<float*>(target);

        UInt32 total_frames_to_read = samples / _channels;
        UInt32 frames_to_read = total_frames_to_read;
        UInt32 frames_read = 0;

        UInt32 frames_read_into_buffer = 0;

        while (frames_read < total_frames_to_read)
        {
            frames_to_read = total_frames_to_read - frames_read;

            AudioBufferList buffer_list;
            buffer_list.mNumberBuffers = 1;
            buffer_list.mBuffers[0].mNumberChannels = _channels;
            buffer_list.mBuffers[0].mDataByteSize = frames_to_read * _channels * sizeof(float);
            buffer_list.mBuffers[0].mData = (void*)(&buffer[frames_read * _channels]);

            frames_read_into_buffer = frames_to_read;
            err = ExtAudioFileRead(_audio_file, &frames_read_into_buffer, &buffer_list);
            if (!frames_read_into_buffer)
            {
                break;
            }
            frames_read += frames_read_into_buffer;
        }

        return frames_read * _channels;
    }

private:
    const std::string _filename;
    ExtAudioFileRef   _audio_file = nullptr;
    Float64           _sample_rate;
    SInt32            _offset = 0;
    UInt32            _channels;
    float             _duration = 0;
};

namespace musly {namespace decoders {

MUSLY_DECODER_REGIMPL(coreaudio, 0);

coreaudio::coreaudio()
{
    MINILOG(logTRACE) << "coreaudio: Created CoreAudio decoder.";
}

std::vector<float>
coreaudio::decodeto_22050hz_mono_float(const std::string& file, float excerpt_length, float excerpt_start)
{
    AudioFile audio_file(file);
    if (!audio_file.open())
    {
        return std::vector<float>(0);
    }

    MINILOG(logDEBUG) << "coreaudio: Audio file duration : " << audio_file.duration() << " seconds";

    excerpt_length = std::min(excerpt_length, audio_file.duration());

    if(excerpt_length <= 0 || (excerpt_length > audio_file.duration()))
    {
        excerpt_start = 0;
        excerpt_length = audio_file.duration();
    }
    else if (excerpt_start < 0)
    {
        excerpt_start = std::min(-excerpt_start, (audio_file.duration() - excerpt_length) / 2);
    }
    else if (excerpt_length + excerpt_start > audio_file.duration())
    {
        excerpt_start = std::max(0.0f, audio_file.duration() - excerpt_length);
        excerpt_length = std::min(audio_file.duration(), audio_file.duration() - excerpt_start);
    }

    MINILOG(logDEBUG) << "coreaudio: Will decode from " << excerpt_start << " to " << (excerpt_start + excerpt_length);
    audio_file.seek(excerpt_start);

    SInt64 samples_to_read = excerpt_length * audio_file.sample_rate() * audio_file.channels();

    std::vector<float> decoded_pcm(samples_to_read);
    SInt64 samples_read = audio_file.read(samples_to_read, decoded_pcm.data());

    std::vector<float> pcm(samples_read / audio_file.channels());
    if (audio_file.channels() == 1)
    {
        std::copy(decoded_pcm.begin(), decoded_pcm.begin() + samples_read, pcm.begin());
    }
    else
    {
        MINILOG(logDEBUG) << "coreaudio: Will downmix audio from " << audio_file.channels() << " channels to mono";
        for (SInt64 i = 0; i < samples_read/2; ++i)
        {
            pcm[i] = (decoded_pcm[i*2] + decoded_pcm[i*2+1]) / 2.0f;
        }
    }

    if (audio_file.sample_rate() != 22050)
    {
        MINILOG(logDEBUG) << "coreaudio: Will downsample audio from " << audio_file.sample_rate() << "Hz to 22050Hz";
        resampler r(audio_file.sample_rate(), 22050);
        return r.resample(pcm.data(), pcm.size());
    }

    MINILOG(logTRACE) << "Decoding: " << file << " finalized.";

    return pcm;
}

}} // namespace musly::decoders
