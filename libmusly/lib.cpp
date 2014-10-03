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

#include <arpa/inet.h>
#include <vector>
#include <fstream>

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
#include "decoders/libav_0_8.h"

MUSLY_METHOD_REGSTATIC(mandelellis, 0);
MUSLY_METHOD_REGSTATIC(timbre, 1);
MUSLY_DECODER_REGSTATIC(libav_0_8, 0);

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
        m->add_tracks(tracks, trackids, length, (generate_ids != 0));
        return 0;
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
        int read = 0;
        if (header) {
            int expected_tracks = m->deserialize_metadata(buffer);
            if (expected_tracks < 0) {
                return -1;
            }
            if (num_tracks < 0) {
                num_tracks = expected_tracks;
            }
            read = musly_jukebox_binsize(jukebox, 1, 0);
            buffer += read;
        }
        if (num_tracks) {
            num_tracks = m->deserialize_trackdata(buffer, num_tracks);
            read += musly_jukebox_binsize(jukebox, 0, num_tracks);
        }
        return num_tracks;
    } else {
        return -1;
    }
}

int
musly_jukebox_tofile(
        musly_jukebox* jukebox,
        const char* filename) {
    if (!jukebox || !jukebox->method) {
        return -1;
    }

    // obtain size of serialized jukebox header and track information
    const int size_head = musly_jukebox_binsize(jukebox, 1, 0);
    const int size_track = musly_jukebox_binsize(jukebox, 0, 1);
    if ((size_head < 0) || (size_track < 0)) {
        return -1;
    }

    // prepare file
    std::ofstream f(filename, std::ios::binary);
    if (f.fail()) {
        return -1;
    }

    // write musly version
    f << musly_version() << '\0';
    if (f.fail()) {
        return -1;
    }

    // write platform information
    const uint8_t intsize = sizeof(int);
    const uint32_t byteorder = 0x01020304;
    f.write((char*)&intsize, sizeof(intsize));
    if (f.fail()) {
        return -1;
    }
    f.write((char*)&byteorder, sizeof(byteorder));
    if (f.fail()) {
        return -1;
    }

    // write general jukebox information
    f << jukebox->method_name << '\0' <<
            jukebox->decoder_name << '\0';
    if (f.fail()) {
        return -1;
    }

    unsigned char* buffer;
    int written;

    // write jukebox-specific header
    f.write((char*)&size_head, sizeof(size_head));
    if (f.fail()) {
        return -1;
    }
    buffer = new unsigned char[size_head];
    written = musly_jukebox_tobin(jukebox, buffer, 1, 0, 0);
    if ((written < 0) || !f.write((char*)buffer, written)) {
        delete[] buffer;
        return -1;
    }
    delete[] buffer;

    // write jukebox-specific track information
    // write in chunks of about 64 KiB
    const int num_tracks = musly_jukebox_trackcount(jukebox);
    const int batchsize = std::min(std::max(64<<10 / size_track, 1), num_tracks);
    buffer = new unsigned char[size_track * batchsize];
    for (int i = 0; i < num_tracks; i += batchsize) {
        written = musly_jukebox_tobin(jukebox, buffer, 0, batchsize, i);
        if ((written < 0) || !f.write((char*)buffer, written)) {
            delete[] buffer;
            return -1;
        }
    }
    delete[] buffer;

    // return total bytes written
    return f.tellp();
}

musly_jukebox*
musly_jukebox_fromfile(
        const char* filename) {
    // open file
    std::ifstream f(filename, std::ios::binary);
    if (f.fail()) {
        return NULL;
    }

    // read musly version and platform information
    std::string version;
    std::getline(f, version, '\0');
    if (f.fail() || (version.compare(MUSLY_VERSION) != 0)) {
        MINILOG(logERROR) << "File was written with musly version " << version
                << ", expected " << MUSLY_VERSION;
        return NULL;
    }
    uint8_t intsize;
    f.read((char*)&intsize, sizeof(intsize));
    if (f.fail() || (intsize != sizeof(int))) {
        MINILOG(logERROR) << "File was written with integer size " << intsize
                << ", expected " << sizeof(int);
        return NULL;
    }
    uint32_t byteorder;
    f.read((char*)&byteorder, sizeof(byteorder));
    if (f.fail() || (byteorder != (uint32_t)0x01020304)) {
        MINILOG(logERROR) << "File was written with different byte order";
        return NULL;
    }

    // read general jukebox information
    std::string method;
    std::getline(f, method, '\0');
    if (f.fail()) {
        return NULL;
    }
    std::string decoder;
    std::getline(f, decoder, '\0');
    if (f.fail()) {
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
    f.read((char*)&size_head, sizeof(size_head));
    if (f.fail()) {
        musly_jukebox_poweroff(jukebox);
        return NULL;
    }
    buffer = new unsigned char[size_head];
    if (!f.read((char*)buffer, size_head) ||
            (musly_jukebox_frombin(jukebox, buffer, 1, 0) < 0)) {
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
    const int batchsize = std::max(64<<10 / size_track, 1);
    buffer = new unsigned char[batchsize * size_track];
    while (1) {
        f.read((char*)buffer, batchsize * size_track);
        if (f.gcount() == 0) {
            break;
        }
        if (f.gcount() % size_track) {
            // bytes read not divisible by track information size,
            // the file must be wrong
            musly_jukebox_poweroff(jukebox);
            delete[] buffer;
            return NULL;
        }
        if (musly_jukebox_frombin(jukebox, buffer, 0, f.gcount() / size_track) < 0) {
            musly_jukebox_poweroff(jukebox);
            delete[] buffer;
            return NULL;
        }
        if (f.gcount() < batchsize * size_track) {
            break;
        }
    }
    delete[] buffer;

    return jukebox;
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
        return musly_track_analyze_pcm(jukebox, pcm.data(), pcm.size(), track);

    } else {
        return -1;
    }
    return 0;
}

