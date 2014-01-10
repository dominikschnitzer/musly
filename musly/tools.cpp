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

#include "tools.h"

std::string
freadstr(
        FILE* fid,
        int max_size)
{
    std::string str = "";

    int count = -1;
    do {
        count++;
        int c = fgetc(fid);
        if (c == EOF) {
            clearerr(fid);
            return "";
        } if (c == 0) {
            break;
        } else {
            str += (char)c;
        }
    } while (count < max_size);

    return str;
}


int
fwritestr(
        FILE* fid,
        const std::string& str)
{
    if (fwrite(str.c_str(), sizeof(char), str.length(), fid) == str.length()) {
        if (fputc(0, fid) == 0) {
            return str.length();
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}


std::vector<std::string>&
split(
        const std::string &s,
        char delim,
        std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string>
split(
        const std::string &s,
        char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


std::string
longest_common_prefix(const std::vector<std::string> &strs)
{
    std::string prefix;
    if(strs.size() > 0) {
        prefix = strs[0];
    }

    for (size_t i = 1; i < strs.size(); ++i) {
        std::string s = strs[i];
        size_t j = 0;
        while (j < std::min(prefix.size(), s.size())) {
            if (prefix[j] != s[j]) {
                break;
            }
            j++;
        }
        prefix = prefix.substr(0, j);
    }
    return prefix;
}
