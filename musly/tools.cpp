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

#include <algorithm>

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
            return static_cast<int>(str.length());
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


void
field_from_strings(
        const std::vector<std::string>& strings,
        int fidx,
        std::map<int, std::string>& id2string,
        std::vector<int>& ids)
{
    int prefix_len = 0;
    if (fidx < 0) {
        prefix_len = static_cast<int>(longest_common_prefix(strings).length());
        fidx = 0;
    }

    std::map<std::string, int> genre_idx;
    int running_id = 0;

    for (size_t i = 0; i < strings.size(); i++) {

        std::string file = strings[i].substr(prefix_len,
                strings[i].length()-prefix_len);

        std::vector<std::string> filesplit = split(file, '/');
        if (fidx < (int)filesplit.size()) {
            std::string g = filesplit[fidx];
            int id;
            if (genre_idx.find(g) != genre_idx.end()) {
                id = genre_idx[g];
            } else {
                genre_idx[g] = running_id;
                id2string[running_id] = g;
                id = running_id;
                running_id++;
            }
            ids.push_back(id);
        } else {
            std::string g = "Unknown";
            int id = -1;
            if (genre_idx.find(g) == genre_idx.end()) {
                id2string[id] = g;
            }
            ids.push_back(id);
        }
    }
}

std::string
limit_string(
        const std::string& s,
        int maxsize)
{
    if ((int)s.size() <= maxsize) {
        return s;
    }

    if (maxsize == 0) {
        return "";
    } else if (maxsize == 1) {
        return ".";
    } else if (maxsize == 2) {
        return "..";
    } else {
        std::string s2 = s.substr(s.size()-maxsize, maxsize);
        s2[0] = '.';
        s2[1] = '.';
        return s2;
    }
}
