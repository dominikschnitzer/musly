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

#include <cstdio>
#include "method.h"

namespace musly {

method::method() :
        track_size(0)
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
    const int buffer_size = 256;
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

int
method::set_musicstyle(
        musly_track** /*tracks*/,
        int /*length*/)
{
    return 0;
}

int
method::guess_neighbors(
        musly_trackid /*seed*/,
        musly_trackid* /*neighbors*/,
        int /*length*/,
        musly_trackid* /*limit_to*/,
        int /*num_limit_to*/)
{
    // all tracks
    return -1;
}

int
method::serialize_metadata(
        unsigned char* buffer) {
    if (buffer) {
        *(int*)(buffer) = get_trackcount();
    }
    return sizeof(int);
}

int
method::deserialize_metadata(
        unsigned char* buffer) {
    int expected_tracks = *(int*)(buffer);
    return expected_tracks;
}

int
method::serialize_trackdata(
        unsigned char* /*buffer*/,
        int /*num_tracks*/,
        int /*skip_tracks*/) {
    // default: not implemented
    return -1;
}

int
method::deserialize_trackdata(
        unsigned char* /*buffer*/,
        int /*num_tracks*/) {
    // default: not implemented
    return -1;
}


} /* namespace musly */
