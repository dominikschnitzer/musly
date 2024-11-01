/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mediafoundation.h"
#include "minilog.h"
#include "resampler.h"

#include <cstdlib>
#include <algorithm>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <limits>

namespace {

template<typename T> void releaseHandle(T **handle)
{
    if (handle)
    {
        (*handle)->Release();
        *handle = nullptr;
    }
}

class AudioFile
{
public:
    AudioFile(const std::string& filename)
        : _filename(filename),
          _channels(0),
          _sample_rate(0)
    {}

    ~AudioFile()
    {
        releaseHandle(&_reader);
    }

    bool open()
    {
        MINILOG(logTRACE) << "mediafoundation: Decoding " << _filename << " started";

        wchar_t url_str[MAX_PATH + 1];
        MINILOG(logTRACE) << "what";
        MultiByteToWideChar(CP_UTF8, 0, _filename.c_str(), -1, (LPWSTR)url_str, MAX_PATH);

        HRESULT res(S_OK);

        MINILOG(logTRACE) << "mediafoundation: Init COM";
        res = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: Could not initialize COM";
            return false;
        }

        MINILOG(logTRACE) << "mediafoundation: Init MediaFoundation";
        res = MFStartup(MF_VERSION);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: Could not initialize MediaFoundation";
            return false;
        }

        MINILOG(logTRACE) << "mediafoundation: Open file";
        
        res = MFCreateSourceReaderFromURL(url_str, nullptr, &_reader);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: Could not open file";
            return false;
        }

        if (!_setupStream(_reader))
        {
            return false;
        }

        seek(0.0f);

        return true;
    }

    bool seek(float position)
    {
        int64_t position_nsec = std::max(int64_t(0), (int64_t)(position * 1e7));

        HRESULT res(S_OK);  
        PROPVARIANT prop_value;
        res = InitPropVariantFromInt64(position_nsec, &prop_value);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: Could not initialize seek property value";
            return false;
        }

        res = _reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
        if (FAILED(res))
        {
            MINILOG(logWARNING) << "mediafoundation: Could not flush stream before seeking";
        }

        res = _reader->SetCurrentPosition(GUID_NULL, prop_value);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: Failure while seeking position in stream";
        }
        PropVariantClear(&prop_value);
        _reader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
        return true;
    }

    std::vector<float> read(float duration)
    {
        HRESULT res(S_OK);
        DWORD flags(0);
        int64_t timestamp;
        IMFSample* sample(nullptr);
        IMFMediaBuffer *buffer(nullptr);

        size_t total_frames_to_read = _calculateExcerptSize(duration) / channels();
        size_t frames_to_read = total_frames_to_read;
        size_t frames_read = 0;
        uint16_t* audio_buffer(nullptr);
        size_t audio_buffer_length = 0;

        std::vector<uint16_t> short_samples(total_frames_to_read * channels());

        bool had_error = false;

        MINILOG(logINFO) << "frames to read: " << total_frames_to_read << " length " << duration;

        while (frames_read < total_frames_to_read)
        {
            frames_to_read  = total_frames_to_read - frames_read;

            res = _reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, &timestamp, &sample);
            if (FAILED(res))
            {
                MINILOG(logWARNING) << "mediafoundation: failed to read sample";
                break;
            }
            
            if (flags & MF_SOURCE_READERF_ERROR)
            {
                MINILOG(logERROR) << "mediafoundation: failed to read sample because of reader error";
                break;
            }
            else if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            {
                MINILOG(logTRACE) << "mediafoundation: failed to read sample because EOF";
                break;
            }
            else if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
            {
                MINILOG(logERROR) << "mediafoundation: failed to read sample because format not supported by PCM format";
                break;
            }
            else if (sample == nullptr)
            {
                MINILOG(logERROR) << "mediafoundation: failed to read sample because no data was returned?!";
                continue;
            }

            res = sample->ConvertToContiguousBuffer(&buffer);
            if (FAILED(res))
            {
                MINILOG(logERROR) << "mediafoundation: failed to convert sample into buffer";
                had_error = true;
                goto release_sample;
            }

            res = buffer->Lock(reinterpret_cast<BYTE**>(&audio_buffer), nullptr, reinterpret_cast<DWORD*>(&audio_buffer_length));
            if (FAILED(res))
            {
                MINILOG(logERROR) << "mediafoundation: failed to read audio data";
                had_error = true;
                goto release_buffer;
            }

            audio_buffer_length /= (sizeof(int16_t) * channels());
            if (audio_buffer_length > frames_to_read)
            {
                audio_buffer_length = frames_to_read;
            }

            memcpy(&short_samples[frames_read * channels()], audio_buffer, audio_buffer_length * channels() * sizeof(int16_t));

            res = buffer->Unlock();
            audio_buffer = nullptr;
            if (FAILED(res))
            {
                had_error = true;
                goto release_buffer;
            }

            frames_read += audio_buffer_length;
release_buffer:
            releaseHandle(&buffer);
release_sample:
            releaseHandle(&sample);

            if (had_error)
            {
                break;
            }
        }

        size_t samples_read = frames_read * channels();
        const int sample_max = std::numeric_limits<int16_t>::max();

        std::vector<float> result(frames_read * channels());
        std::transform(
            short_samples.begin(), 
            short_samples.end(), 
            result.begin(), 
            [=](int16_t sample) { return static_cast<float>(sample) / (float)sample_max; });

        return result;
    }

    float duration() const
    {
        return _duration;
    }

    uint32_t channels() const
    {
        return _channels;
    }

    uint32_t sample_rate() const
    {
        return _sample_rate;
    }


private:
    const std::string _filename;
    IMFSourceReader*  _reader;
    uint32_t          _channels;
    uint32_t          _sample_rate;
    float             _duration;

    bool _setupStream(IMFSourceReader* reader)
    {
        HRESULT res(S_OK);

        res = reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure deselecting all streams";
            return false;
        }

        res = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure selecting first audio stream";
            return false;
        }

        IMFMediaType* source_media_type;
        res = reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &source_media_type);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure reading source media type";
            return false;
        }

        uint32_t channels;
        res = source_media_type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure reading number of source audio channels";
            return false;
        }

        uint32_t sample_rate;
        res = source_media_type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure reading source sample rate";
            return false;
        }

        IMFMediaType* target_media_type;
        res = MFCreateMediaType(&target_media_type);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure creating target media type";
            return false;
        }

        target_media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        target_media_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        //target_media_type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sample_rate);
        //target_media_type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
        target_media_type->SetUINT32(MF_MT_SAMPLE_SIZE, sizeof(uint16_t));
        //target_media_type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);

        res = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, target_media_type);
        releaseHandle(&target_media_type);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure setting target media type";
            return false;
        }

        res = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &target_media_type);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure re-loading media type";
            return false;
        }

        res = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure re-setting first audio stream";
            return false;
        }

        PROPVARIANT prop_value;
        res = reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop_value);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure getting audio file duration";
            return false;
        }

        _duration = static_cast<float>(prop_value.hVal.QuadPart / 1e7);
        _channels = channels;
        _sample_rate = sample_rate;

        return true;
    }

    uint32_t _calculateExcerptSize(float desired_length)
    {
        HRESULT res(S_OK);
        uint32_t desired_length_msec = (desired_length * 1000);

        IMFMediaType* target_media_type;
        res = _reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &target_media_type);
        if (FAILED(res))
        {
            MINILOG(logERROR) << "mediafoundation: failure getting target media type";
            return 0;
        }

        uint32_t block_size;
        target_media_type->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &block_size);
        
        uint32_t bytes_per_second;
        target_media_type->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &bytes_per_second);

        return desired_length * sample_rate() * channels();

    }
};

}

namespace musly { namespace decoders {


MUSLY_DECODER_REGIMPL(mediafoundation, 0);


mediafoundation::mediafoundation()
{
    MINILOG(logTRACE) << "mediafoundation: Create MediaFoundation decoder.";
}

mediafoundation::~mediafoundation()
{
    MINILOG(logTRACE) << "mediafoundation: Deleted MediaFoundation decoder.";
}

std::vector<float>
mediafoundation::decodeto_22050hz_mono_float(const std::string& file,
                                             float excerpt_length,
                                             float excerpt_start)
{
    AudioFile audio_file(file);
    if (!audio_file.open())
    {
        return std::vector<float>(0);
    }

    MINILOG(logINFO) 
            << "media: channels: " 
            << audio_file.channels() 
            << "; sample_rate: " 
            << audio_file.sample_rate()
            << "; duration: " << audio_file.duration();

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

    MINILOG(logDEBUG) << "mediafoundation: Will decode from " << excerpt_start << " to " << (excerpt_start + excerpt_length);
    audio_file.seek(excerpt_start);

    std::vector<float> decoded_pcm = audio_file.read(excerpt_length);
    std::vector<float> pcm(decoded_pcm.size() / audio_file.channels());
    if (audio_file.channels() >1)
    {
        MINILOG(logDEBUG) << "mediafoundation: Will downmix audio from " << audio_file.channels() << " channels to mono";
        for (size_t i = 0; i < pcm.size(); ++i)
        {
            pcm[i] = (decoded_pcm[i*2] + decoded_pcm[i*2+1]) / 2.0f;
        }
    }

    if (audio_file.sample_rate() != 22050)
    {
        MINILOG(logDEBUG) << "mediafoundation: Will downsample audio from " << audio_file.sample_rate() << "Hz to 22050Hz";
        resampler r(audio_file.sample_rate(), 22050);
        return r.resample(pcm.data(), pcm.size());
    }


    MINILOG(logTRACE) << "Decoding: " << file << " finalized.";

    return pcm;
}


}} // namespace musly::decoders
