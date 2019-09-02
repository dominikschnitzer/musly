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
 
#include <sys/stat.h>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>

#include "tools.h"
#include "collectionfile.h"

collection_file::collection_file(
        const std::string& coll) :
    coll(coll),
    version("0"),
    header("MUSLY"),
    dash("-"),
    fid(0)
{
}


collection_file::~collection_file()
{
    if (fid) {
        fclose(fid);
    }
}


bool
collection_file::open(std::string mode)
{
    if (fid) {
        fclose(fid);
    }
    fid = fopen(coll.c_str(), mode.c_str());
    if (!fid) {
        return false;
    } else {
        return true;
    }
}


bool
collection_file::exists()
{
    struct stat buffer;
    return (stat (coll.c_str(), &buffer) == 0);
}


bool
collection_file::write_header(const std::string& meth)
{
    fwritestr(fid, header+dash+version+dash+meth);

    return true;
}



bool
collection_file::read_header()
{
    std::string headerstring = freadstr(fid, 255);
    std::vector<std::string> headersplit = split(headerstring, '-');
    if (headersplit.size() != 3) {
        return false;
    }

    if ((headersplit[0] !=  header) || (headersplit[1] != version)) {
        return false;
    }

    // save method
    method = headersplit[2];

    return true;
}


std::string
collection_file::get_method()
{
    return method;
}

std::string
collection_file::get_file()
{
    return coll;
}

bool
collection_file::contains_track(const std::string& trackfile)
{
    // check if we have analyzed the file already
    std::map<std::string, int>::iterator fm_iter = filemap.find(trackfile);
    if (fm_iter == filemap.end()) {
        return false;
    } else {
        return true;
    }
}

int
collection_file::read_track(
        unsigned char* buffer,
        int buffersize,
        std::string& file)
{
    // save the file position of the current record, rewind in case
    // of an error
    fpos_t pos;
    if (fgetpos(fid, &pos) != 0) {
        return -1;
    }

    // read the filename
    file = freadstr(fid, 4096);
    if (file.length() == 0) {
        fsetpos(fid, &pos);
        return -1;
    }

    // if the current track was already loaded
    // return an error. Maybe skip the track model in a later version.
    if (contains_track(file)) {
        fsetpos(fid, &pos);
        return -1;
    }

    // read the size of the data field
    uint32_t sz;
    if (fread(&sz, sizeof(uint32_t), 1, fid) != 1) {
        fsetpos(fid, &pos);
        return -1;
    }

    // buffer insufficient
    if ((int)sz > buffersize) {
        fsetpos(fid, &pos);
        return -1;
    }

    // read the data, if size > 0
    if (sz > 0) {
        if (fread(buffer, sizeof(unsigned char), sz, fid) != sz) {
            fsetpos(fid, &pos);
            return -1;
        }
    }

    // add the file to the map
    filemap[file] = 1;

    return sz;
}

bool
collection_file::append_track(
        const std::string& filename,
        const unsigned char* bindata,
        int size)
{
    // write the filename
    fwritestr(fid, filename);

    // write the size. A size of zero indicates an analysis error
    uint32_t sz;
    if ((size == 0) || (!bindata)) {
        sz = 0;
    } else {
        sz = size;
    }
    if (fwrite(&sz, sizeof(uint32_t), 1, fid) != 1) {
        return false;
    }

    // write the serialized musly track
    if ((size > 0) && bindata) {
        if (fwrite(bindata, size, 1, fid) != 1) {
            return false;
        }
    }

    return true;
}

