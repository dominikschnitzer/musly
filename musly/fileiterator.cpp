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


#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>

#include "fileiterator.h"

fileiterator::fileiterator(const std::string& path,
        const std::string& extension) :
        scan_dir(false),
        scan_file(false)

{
    current_dir = path;

    // set scan extension
    if (extension.length() > 0) {
        search_ext = "." + extension;

        std::transform(search_ext.begin(), search_ext.end(),
                search_ext.begin(), ::tolower);
    } else {
        search_ext = "";
    }

    // check if we have to scan a file or a directory
    dir = opendir(path.c_str());
    if (dir) {
        scan_dir = true;

    // try opening as file
    } else {

        std::ifstream file(path.c_str());
        if (file.is_open()) {
             scan_file = true;
             file.close();
         }
    }
}


fileiterator::~fileiterator()
{
    if (dir) {
        closedir(dir);
        dir = NULL;
    }
}

bool
fileiterator::has_extension(std::string path)
{
    if (search_ext.length() == 0) {
        return true;
    }

    std::transform(path.begin(), path.end(), path.begin(), ::tolower);

    if (path.length() >= search_ext.length()) {
        return (0 == path.compare(path.length()-search_ext.length(),
                search_ext.length(), search_ext));
    } else {
        return false;
    }
}


bool
fileiterator::get_nextfilename(std::string& file)
{
    if (scan_file) {
        file = current_dir;
        scan_file = false;
        return true;
    } else if (scan_dir) {
        return get_nextfilename_dir(file);
    }
    return false;
}


bool
fileiterator::get_nextfilename_dir(std::string& file)
{
    if (!dir) {
        return false;
    }

    // first search directory entries
    struct dirent* entry = readdir(dir);
    struct stat s;
    while (entry || !dir_queue.empty()){

        // process current directory
        if (entry) {
            std::string p = entry->d_name;
            std::string fp = current_dir + "/" + p;

            int stat_res = stat(fp.c_str(), &s);
            if (stat_res != 0) {
                // skip entry on error
                continue;
            }

            // Directory found
            if (S_ISDIR(s.st_mode)) {

                // skip . and ..
                // add to queue to process after the current directory is finished
                if ((p.compare(".") != 0) && (p.compare("..") != 0)) {
                    dir_queue.push_back(fp);
                }

            // file found
            } else if ((S_ISREG(s.st_mode)) && (has_extension(fp))) {
                file = fp;
                return true;
            }

        // process the next directory
        } else if (!dir_queue.empty()) {

            // finish and close the current directory
            closedir(dir);
            dir = NULL;

            // try to get a new directory handle
            while (!dir && !dir_queue.empty()) {
                current_dir = dir_queue.front();
                dir_queue.pop_front();
                dir = opendir(current_dir.c_str());
            }
        }

        // read the next directory entry
        if (dir) {
            entry = readdir(dir);
        }
    }

    if (dir) {
        closedir(dir);
        dir = NULL;
    }
    return false;
}

