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

#ifndef MUSLY_FILEITERATOR_H_
#define MUSLY_FILEITERATOR_H_

#include <sys/types.h>
#include <dirent.h>
#include <list>
#include <string>
#include <fstream>

class fileiterator {
private:
    DIR* dir;
    std::list<std::string> dir_queue;

    std::string current_dir;
    std::string search_ext;

    bool scan_dir;
    bool scan_file;

    bool has_extension(std::string path);
    bool get_nextfilename_file(std::string& f);
    bool get_nextfilename_dir(std::string& f);

public:
    fileiterator(const std::string& path, const std::string& extension);
    virtual ~fileiterator();

    bool get_nextfilename(std::string& file);
};

#endif /* MUSLY_FILEITERATOR_H_ */
