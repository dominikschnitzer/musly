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
#ifndef MUSLY_COLLECTIONFILE_H_
#define MUSLY_COLLECTIONFILE_H_

#include <string>
#include <map>

class collection_file {
private:
    std::string coll;
    std::string method;
    std::string version;
    std::string header;
    std::string dash;

    FILE* fid;

    std::map<std::string, int> filemap;

    bool
    exists();


public:
    collection_file(
            const std::string& coll);

    virtual
    ~collection_file();

    bool
    open(std::string mode);

    bool
    write_header(
            const std::string& meth);

    bool
    read_header();

    bool
    append_track(
            const std::string& filename,
            const unsigned char* bindata,
            int size);

    int
    read_track(
            unsigned char* buffer,
            int buffersize,
            std::string& file);

    bool
    contains_track(const std::string& trackfile);

    std::string
    get_method();

    std::string
    get_file();
};

#endif /* MUSLY_COLLECTIONFILE_H_ */
