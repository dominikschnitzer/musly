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

#include "musly/musly.h"
#include "minilog.h"
#include "plugins.h"
#include "decoder.h"
#include "method.h"


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
    if (jukebox && jukebox->method) {
        musly::method* m = reinterpret_cast<musly::method*>(jukebox->method);
        return m->guess_neighbors(seed, neighbors, num_neighbors);
    } else {
        return -1;
    }
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
        int max_seconds,
        musly_track* track)
{
    if (jukebox && jukebox->decoder) {

        // try decoding the given audio file
        musly::decoder* d = reinterpret_cast<musly::decoder*>(jukebox->decoder);

        // decode a maximum of max_seconds
        std::vector<float> pcm =
                d->decodeto_22050hz_mono_float(audiofile, max_seconds);
        if (pcm.size() == 0) {
            return -1;
        }

        // select the central 30s of the decoded part
        int start = 0;
        int sel_central_samples = 30*22050;
        int len = pcm.size();
        if (len > sel_central_samples) {
            start = (len - sel_central_samples) / 2;
            len = sel_central_samples;
        }

        return musly_track_analyze_pcm(jukebox, pcm.data()+start, len, track);

    } else {
        return -1;
    }
    return 0;
}

