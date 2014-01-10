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

#include <cstdio>
#include "method.h"

namespace musly {

method::method() :
        track_size(0),
        current_tid(0)
{
}

method::~method()
{
}


int
method::track_addfield_floats(
        const std::string& name,
        int num_floats)
{
    // save the field name and field size
    track_field_name.push_back(name);
    track_field_size.push_back(num_floats);

    // compute the starting offset for the feature
    int offs = track_size;

    // update the musly_track size
    track_size += num_floats;

    return offs;
}

int
method::track_getsize()
{
    return track_size;
}

musly_track*
method::track_alloc()
{
    return new musly_track[track_size];
}

const char*
method::track_tostr(
        musly_track* track)
{
    // TODO: Use and reuse a string buffer
    trackstr = "";
    int offs = 0;
    int buffer_size = 256;
    char buffer[buffer_size];
    for (int i = 0; i < (int)track_field_name.size(); i++) {
        trackstr += track_field_name[i] + ":";
        for (int j = 0; j < (int)track_field_size[i]; j++) {
            snprintf(buffer, buffer_size-1, " %f", track[offs]);
            trackstr += buffer;
            offs++;
        }
        trackstr += "\n";
    }

    return trackstr.c_str();
}

musly_trackid
method::add_track(
        musly_track* track)
{
    //get next trackid
    musly_trackid trackid = current_tid;
    current_tid++;

    init_track(track, trackid);
    return trackid;
}

int
method::set_musicstyle(
        musly_track** tracks,
        int length)
{
    return 0;
}


int
method::init_track(
        musly_track* track,
        musly_trackid trackid)
{
    return 0;
}

} /* namespace musly */
