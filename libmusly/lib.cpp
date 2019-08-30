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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <algorithm>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cstring>

#define MUSLY_SUPPORT_STDIO
#include "musly/musly.h"
#include "minilog.h"
#include "plugins.h"
#include "decoder.h"
#include "method.h"

#ifdef BUILD_STATIC
// Implementation note: Each plugin is supposed to register itself with
// the MUSLY_METHOD_REGIMPL or MUSLY_DECODER_REGIMPL call. These calls
// are usually placed in the plugin's cpp file, and executed when loading
// the shared library. However, with static linking, these calls are not
// executed, so the plugins are optimized out. Explicitly registering the
// plugins here instead ensures the plugins are present in static builds.

#include "methods/mandelellis.h"
#include "methods/timbre.h"
#include "decoders/libav.h"

MUSLY_METHOD_REGSTATIC(mandelellis, 0);
MUSLY_METHOD_REGSTATIC(timbre, 1);
MUSLY_DECODER_REGSTATIC(libav, 0);

#ifdef LIBMUSLY_EXTERNAL
#include "external/register_static.h"
#endif
#endif


const char*
musly_version()
{
    return MUSLY_VERSION;
}

void
musly_debug(
        int level)
{
    // Set the log level, too high/low values are set to the closest valid
    // log level.
    MiniLog::current_level() = minilog_level(
            std::min(std::max(0, level), (int)(minilog_level_max)-1));
}

const char*
musly_jukebox_listmethods()
{
    return musly::plugins::get_plugins(musly::plugins::METHOD_TYPE).c_str();
}

const char*
musly_jukebox_listdecoders()
{
    return musly::plugins::get_plugins(musly::plugins::DECODER_TYPE).c_str();
}

const char*
musly_jukebox_aboutmethod(
        musly_jukebox* jukebox)
{
    if (jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);

        return m->about();
    }

    return "";
}

musly_jukebox*
musly_jukebox_poweron(
        const char* method,
        const char* decoder)
{
    // try initializing the similarity method
    std::string method_str;
    if (!method) {
        method_str = "";
    } else {
        method_str = method;
    }
    musly::method* m = reinterpret_cast<musly::method*>(
        musly::plugins::instantiate_plugin(
            musly::plugins::METHOD_TYPE, method_str));
    if (!m) {
        return NULL;
    }

    // try initializing the selected decoder
    std::string decoder_str;
    if (!decoder) {
        decoder_str = "";
    } else {
        decoder_str = decoder;
    }
    musly::decoder* d = reinterpret_cast<musly::decoder*>(
            musly::plugins::instantiate_plugin(
                    musly::plugins::DECODER_TYPE, decoder_str));
    if (!d) {
        delete m;
        return NULL;
    }

    // if we succeeded in both, return the jukebox!
    musly_jukebox* mj = new musly_jukebox;
    mj->method = reinterpret_cast<void*>(m);
    mj->method_name = new char[method_str.size()+1];
    method_str.copy(mj->method_name, method_str.size());
    mj->method_name[method_str.size()] = '\0';

    mj->decoder = reinterpret_cast<void*>(d);
    mj->decoder_name = new char[decoder_str.size()+1];
    decoder_str.copy(mj->decoder_name, decoder_str.size());
    mj->decoder_name[decoder_str.size()] = '\0';
    return mj;
}

void
musly_jukebox_poweroff(
        musly_jukebox* jukebox)
{
    if (!jukebox) {
        return;
    }
    if (jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        delete m;
    }
    if (jukebox->method_name) {
        delete[] jukebox->method_name;
    }

    if (jukebox->decoder) {
        musly::decoder* d = reinterpret_cast<musly::decoder*>(jukebox->decoder);
        delete d;
    }
    if (jukebox->decoder_name) {
        delete[] jukebox->decoder_name;
    }

    delete jukebox;
}


int
musly_jukebox_setmusicstyle(
        musly_jukebox* jukebox,
        musly_track** tracks,
        int num_tracks)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->set_musicstyle(tracks, num_tracks);
    } else {
        return -1;
    }
}

int
musly_jukebox_addtracks(
        musly_jukebox* jukebox,
        musly_track** tracks,
        musly_trackid* trackids,
        int length,
        int generate_ids)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->add_tracks(tracks, trackids, length, (generate_ids != 0));
    } else {
        return -1;
    }
}

int
musly_jukebox_removetracks(
        musly_jukebox* jukebox,
        musly_trackid* trackids,
        int length)
{
	if (jukebox && jukebox->method) {
	    musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
		m->remove_tracks(trackids, length);
		return 0;
	} else {
		return -1;
	}
}

int
musly_jukebox_trackcount(
        musly_jukebox* jukebox)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->get_trackcount();
    } else {
        return -1;
    }
}

int
musly_jukebox_maxtrackid(
        musly_jukebox* jukebox)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->get_maxtrackid();
    } else {
        return -1;
    }
}

int
musly_jukebox_gettrackids(
        musly_jukebox* jukebox,
        musly_trackid* trackids) {
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->get_trackids(trackids);
    } else {
        return -1;
    }
}

int
musly_jukebox_similarity(
        musly_jukebox* jukebox,
        musly_track* seed_track,
        musly_trackid seed_trackid,
        musly_track** tracks,
        musly_trackid* trackids,
        int num_tracks,
        float* similarities)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->similarity(
                seed_track, seed_trackid,
                tracks, trackids,
                num_tracks, similarities);
    } else {
        return -1;
    }
}

int
musly_jukebox_guessneighbors(
        musly_jukebox* jukebox,
        musly_trackid seed,
        musly_trackid* neighbors,
        int num_neighbors)
{
    return musly_jukebox_guessneighbors_filtered(
            jukebox, seed, neighbors, num_neighbors, NULL, 0);
}

int
musly_jukebox_guessneighbors_filtered(
        musly_jukebox* jukebox,
        musly_trackid seed,
        musly_trackid* neighbors,
        int num_neighbors,
        musly_trackid* limit_to,
        int num_limit_to)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->guess_neighbors(seed, neighbors, num_neighbors, limit_to, num_limit_to);
    } else {
        return -1;
    }
}

int
musly_jukebox_binsize(
        musly_jukebox* jukebox,
        int header,
        int num_tracks) {
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        int binsize = 0;
        if (header) {
            binsize = m->serialize_metadata(NULL);
            if (binsize < 0) {
                return -1;
            }
        }
        if (num_tracks < 0) {
            num_tracks = m->get_trackcount();
        }
        if (num_tracks) {
            int tracksize = m->serialize_trackdata(NULL, 1);
            if (tracksize < 0) {
                return -1;
            }
            binsize += num_tracks * tracksize;
        }
        return binsize;
    } else {
        return -1;
    }
}

int
musly_jukebox_tobin(
        musly_jukebox* jukebox,
        unsigned char* buffer,
        int header,
        int num_tracks,
        int skip_tracks) {
    if (jukebox && jukebox->method && (skip_tracks >= 0)) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        int written = 0;
        if (header) {
            written += m->serialize_metadata(buffer);
        }
        int trackcount = m->get_trackcount();
        if ((num_tracks < 0) || (num_tracks + skip_tracks > trackcount)) {
            num_tracks = trackcount - skip_tracks;
            if (num_tracks < 0) {
                return -1;
            }
        }
        if (num_tracks) {
            written += m->serialize_trackdata(buffer + written, num_tracks, skip_tracks);
        }
        return written;
    } else {
        return -1;
    }
}

int
musly_jukebox_frombin(
        musly_jukebox* jukebox,
        unsigned char* buffer,
        int header,
        int num_tracks) {
    if (jukebox && jukebox->method && ((num_tracks >= 0) || header)) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        if (header) {
            int expected_tracks = m->deserialize_metadata(buffer);
            if (expected_tracks < 0) {
                return -1;
            }
            if (num_tracks == 0) {
                return expected_tracks;
            }
            else if (num_tracks < 0) {
                num_tracks = expected_tracks;
            }
            buffer += musly_jukebox_binsize(jukebox, 1, 0);
        }
        if (num_tracks) {
            num_tracks = m->deserialize_trackdata(buffer, num_tracks);
        }
        return num_tracks;
    } else {
        return -1;
    }
}

int
musly_jukebox_tostream(
        musly_jukebox* jukebox,
        FILE* stream) {
    int written = 0;

    if (!jukebox || !jukebox->method) {
        return -1;
    }

    // obtain size of serialized jukebox header and track information
    const int size_head = musly_jukebox_binsize(jukebox, 1, 0);
    const int size_track = musly_jukebox_binsize(jukebox, 0, 1);
    if ((size_head < 0) || (size_track < 0)) {
        return -1;
    }

    // write musly version
    if (fputs(musly_version(), stream) == EOF || fputc('\0', stream) == EOF) {
        return -1;
    }
    written += static_cast<int>(strlen(musly_version())) + 1;

    // write platform information
    const uint8_t intsize = sizeof(int);
    const uint32_t byteorder = 0x01020304;
    if (fwrite(&intsize, sizeof(intsize), 1, stream) != 1) {
        return -1;
    }
    if (fwrite(&byteorder, sizeof(byteorder), 1, stream) != 1) {
        return -1;
    }
    written += static_cast<int>(sizeof(intsize) + sizeof(byteorder));

    // write general jukebox information
    if (fputs(jukebox->method_name, stream) == EOF ||
            fputc('\0', stream) == EOF ||
            fputs(jukebox->decoder_name, stream) == EOF ||
            fputc('\0', stream) == EOF) {
        return -1;
    }
    written += static_cast<int>(strlen(jukebox->method_name)) + 1;
    written += static_cast<int>(strlen(jukebox->decoder_name)) + 1;

    unsigned char* buffer;
    int bcount;

    // write jukebox-specific header
    if (fwrite(&size_head, sizeof(size_head), 1, stream) != 1) {
        return -1;
    }
    buffer = new unsigned char[size_head];
    written += (bcount = musly_jukebox_tobin(jukebox, buffer, 1, 0, 0));
    if ((bcount < 0) || (int)fwrite(buffer, 1, bcount, stream) != bcount) {
        delete[] buffer;
        return -1;
    }
    delete[] buffer;

    // write jukebox-specific track information
    // write in chunks of about 64 KiB
    const int num_tracks = musly_jukebox_trackcount(jukebox);
    if (num_tracks < 0) {
        return -1;
    }
    const int batchsize = std::min(std::max(64<<10 / size_track, 1), num_tracks);
    buffer = new unsigned char[size_track * batchsize];
    for (int i = 0; i < num_tracks; i += batchsize) {
        written += (bcount = musly_jukebox_tobin(jukebox, buffer, 0, batchsize, i));
        if ((bcount < 0) || (int)fwrite(buffer, 1, bcount, stream) != bcount) {
            delete[] buffer;
            return -1;
        }
    }
    delete[] buffer;

    // return total bytes written
    return written;
}

musly_jukebox*
musly_jukebox_fromstream(
        FILE* stream) {
    struct {
        bool operator() (FILE* stream, std::string& str) {
            std::ostringstream oss;
            while (int c = fgetc(stream)) {
                if (c == EOF) return false;
                oss << (char)(c);
            }
            str = oss.str();
            return true;
        }
    } readstr;  // local function to read null-terminated string

    // read musly version and platform information
    std::string version;
    if (!readstr(stream, version) || version.compare(MUSLY_VERSION) != 0) {
        MINILOG(logERROR) << "File was written with musly version " << version
                << ", expected " << MUSLY_VERSION;
        return NULL;
    }
    uint8_t intsize;
    if (fread(&intsize, sizeof(intsize), 1, stream) != 1 ||
            intsize != sizeof(int)) {
        MINILOG(logERROR) << "File was written with integer size " << intsize
                << ", expected " << sizeof(int);
        return NULL;
    }
    uint32_t byteorder;
    if (fread(&byteorder, sizeof(byteorder), 1, stream) != 1 ||
            byteorder != (uint32_t)0x01020304) {
        MINILOG(logERROR) << "File was written with different byte order";
        return NULL;
    }

    // read general jukebox information
    std::string method;
    std::string decoder;
    if (!readstr(stream, method) || !readstr(stream, decoder)) {
        return NULL;
    }

    // create empty jukebox
    musly_jukebox* jukebox = musly_jukebox_poweron(method.c_str(), decoder.c_str());
    if (!jukebox) {
        return NULL;
    }

    unsigned char* buffer;

    // read jukebox-specific header
    int size_head;
    if (fread(&size_head, sizeof(size_head), 1, stream) != 1) {
        musly_jukebox_poweroff(jukebox);
        return NULL;
    }
    buffer = new unsigned char[size_head];
    int expected_tracks;
    if ((int)fread(buffer, 1, size_head, stream) != size_head ||
            (expected_tracks = musly_jukebox_frombin(jukebox, buffer, 1, 0)) < 0) {
        musly_jukebox_poweroff(jukebox);
        delete[] buffer;
        return NULL;
    }
    delete[] buffer;

    // read jukebox-specific track information
    // read in chunks of about 64 KiB
    const int size_track = musly_jukebox_binsize(jukebox, 0, 1);
    if (size_track <= 0) {
        musly_jukebox_poweroff(jukebox);
        return NULL;
    }
    const int batchsize = std::min(std::max(64<<10 / size_track, 1), expected_tracks);
    buffer = new unsigned char[batchsize * size_track];
    while (expected_tracks) {
        int read = std::min(expected_tracks, batchsize);
        if ((int)fread(buffer, size_track, read, stream) != read ||
                musly_jukebox_frombin(jukebox, buffer, 0, read) < 0) {
            musly_jukebox_poweroff(jukebox);
            delete[] buffer;
            return NULL;
        }
        expected_tracks -= read;
    }
    delete[] buffer;

    return jukebox;
}

int
musly_jukebox_tofile(
        musly_jukebox* jukebox,
        const char* filename) {
    if (FILE* f = fopen(filename, "wb")) {
        int result = musly_jukebox_tostream(jukebox, f);
        fclose(f);
        return result;
    }
    return -1;
}

musly_jukebox*
musly_jukebox_fromfile(
        const char* filename) {
    if (FILE* f = fopen(filename, "rb")) {
        musly_jukebox* result = musly_jukebox_fromstream(f);
        fclose(f);
        return result;
    }
    return NULL;
}

musly_track*
musly_track_alloc(
        musly_jukebox* jukebox)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->track_alloc();
    } else {
        return NULL;
    }
}

void
musly_track_free(
        musly_track* track)
{
    if (track) {
        delete[] track;
    }
}

int
musly_track_size(
        musly_jukebox* jukebox)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->track_getsize() * sizeof(float);
    } else {
        return -1;
    }
}

int
musly_track_binsize(
        musly_jukebox* jukebox)
{
    return musly_track_size(jukebox);
}

int
musly_track_tobin(
        musly_jukebox* jukebox,
        musly_track* from_track,
        unsigned char* to_buffer)
{
    if (jukebox && jukebox->method && from_track && to_buffer) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);

        // check if buffer is big enough
        int len = m->track_getsize();
        int sz = len*sizeof(float);

        // serialize to uint32 and convert to network byte order
        union v {
            float f;
            uint32_t i;
        } val;
        uint32_t* raw_uint32 = reinterpret_cast<uint32_t*>(to_buffer);
        for (int i = 0; i < len; i++) {
            val.f = from_track[i];
            raw_uint32[i] = htonl(val.i);
        }

        return sz;

    } else {
        return -1;
    }
}

int
musly_track_frombin(
        musly_jukebox* jukebox,
        unsigned char* from_buffer,
        musly_track* to_track)
{
    if (jukebox && jukebox->method && to_track && from_buffer) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);

        // check if buffer is big enough
        int len = m->track_getsize();
        int sz = len*sizeof(float);

        // deserialize from uint32 and convert from network byte order to
        // host byte order.
        union v {
            float f;
            uint32_t i;
        } val;
        uint32_t* raw_uint32 = reinterpret_cast<uint32_t*>(from_buffer);
        for (int i = 0; i < len; i++) {
            val.i = ntohl(raw_uint32[i]);
            to_track[i] = val.f;
        }

        return sz;

    } else {
        return -1;
    }
}

const char*
musly_track_tostr(musly_jukebox* jukebox,
        musly_track* from_track)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->track_tostr(from_track);
    } else {
        return "";
    }
}

int
musly_track_analyze_pcm(
        musly_jukebox* jukebox,
        float* mono_22khz_pcm,
        int length_pcm,
        musly_track* track)
{
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->analyze_track(mono_22khz_pcm, length_pcm, track);
    } else {
        return -1;
    }
}

int
musly_track_analyze_audiofile(
        musly_jukebox* jukebox,
        const char* audiofile,
        float excerpt_length,
        float excerpt_start,
        musly_track* track)
{
    if (jukebox && jukebox->decoder) {

        // try decoding the given audio file
        musly::decoder* d = reinterpret_cast<musly::decoder*>(jukebox->decoder);

        // decode the specified excerpt
        std::vector<float> pcm =
                d->decodeto_22050hz_mono_float(audiofile, excerpt_length, excerpt_start);
        if (pcm.size() == 0) {
            return -1;
        }

        // pass it on to build the similarity model
        return musly_track_analyze_pcm(jukebox, pcm.data(), static_cast<int>(pcm.size()), track);

    } else {
        return -1;
    }
}

typedef std::pair<float, musly_trackid> knn;
struct knn_comp {
    bool operator()(knn const& lhs, knn const& rhs) {
        return lhs.first < rhs.first;
    }
} knn_comp;

int
musly_findmin(
        const float* values,
        const musly_trackid* ids,
        int count,
        float* min_values,
        musly_trackid* min_ids,
        int min_count,
        int ordered) {

    // Check given arguments
    if (!values) {
        return -1;
    }
    if (min_count > count) {
        min_count = count;
    }
    if (!min_count) {
        return 0;
    }
    if (!min_values && !min_ids) {
        return -1;
    }

    // Find the top `min_count` items by iterating over `values` and
    // possibly `ids` and collecting the best items in a heap
    // TODO: Depending on `count`, `min_count` and `ordered`,
    // std::partial_sort() or std::nth_element() may be faster.
    std::vector<knn> heap(0);
    heap.reserve(min_count);

    if (ids) {
        // Fill heap with first `min_count` (value, id) pairs
        for (int i = 0; i < min_count; i++) {
            heap.push_back(std::make_pair(values[i], ids[i]));
            std::push_heap(heap.begin(), heap.end(), knn_comp);
        }
        // Update heap as needed
        for (int i = min_count; i < count; i++) {
            if (values[i] < heap.front().first) {
                std::pop_heap(heap.begin(), heap.end(), knn_comp);
                heap.back() = std::make_pair(values[i], ids[i]);
                std::push_heap(heap.begin(), heap.end(), knn_comp);
            }
        }
    }
    else {
        // Fill heap with first `min_count` (value, index) pairs
        for (int i = 0; i < min_count; i++) {
            heap.push_back(std::make_pair(values[i], (musly_trackid)i));
            std::push_heap(heap.begin(), heap.end(), knn_comp);
        }
        // Update heap as needed
        for (int i = min_count; i < count; i++) {
            if (values[i] < heap.front().first) {
                std::pop_heap(heap.begin(), heap.end(), knn_comp);
                heap.back() = std::make_pair(values[i], (musly_trackid)i);
                std::push_heap(heap.begin(), heap.end(), knn_comp);
            }
        }
    }

    if (ordered) {
        // transform into sorted list
        std::sort_heap(heap.begin(), heap.end(), knn_comp);
    }

    // Copy out the results
    if (min_values) {
        for (int i = 0; i < min_count; i++) {
            min_values[i] = heap[i].first;
        }
    }
    if (min_ids) {
        for (int i = 0; i < min_count; i++) {
            min_ids[i] = heap[i].second;
        }
    }
    return min_count;
}
