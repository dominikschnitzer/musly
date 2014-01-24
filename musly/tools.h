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

#ifndef MUSLY_TOOLS_H_
#define MUSLY_TOOLS_H_

#include <cstdio>
#include <string>
#include <sstream>
#include <vector>
#include <map>



int
fwritestr(FILE* fid, const std::string& str);

std::string
freadstr(FILE* fid, int max_size);

std::vector<std::string>&
split(
        const std::string &s,
        char delim,
        std::vector<std::string> &elems);

std::vector<std::string>
split(
        const std::string &s,
        char delim);

std::string
longest_common_prefix(
        const std::vector<std::string> &strs);

void
field_from_strings(
        const std::vector<std::string>& strings,
        int fidx,
        std::map<int, std::string>& id2string,
        std::vector<int>& ids);

std::string
limit_string(
        const std::string& s,
        int size);

#endif /* MUSLY_TOOLS_H_ */
