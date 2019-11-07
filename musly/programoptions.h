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

#ifndef MUSLY_PROGRAMOPTIONS_H_
#define MUSLY_PROGRAMOPTIONS_H_

#include <string>
#include <vector>
#include <map>

class programoptions {
private:

    std::string all_methods;
    std::string default_collection;
    std::string default_outputmode;
    int default_k;
    int default_debuglevel;
    std::string action;
    std::string program_name;
    std::map<std::string, std::string> optionstr;

public:
    programoptions(
            int argc,
            char *argv[],
            const std::vector<std::string>& methods);

    virtual
    ~programoptions();

    void
    display_help();

    std::string
    get_action();

    std::string
    get_option_str(
            const std::string &option);

    int
    get_option_int(
            const std::string& option);
};

#endif /* MUSLY_PROGRAMOPTIONS_H_ */
