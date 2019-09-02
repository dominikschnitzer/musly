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
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cctype>


#include "programoptions.h"

programoptions::programoptions(int argc, char *argv[],
        const std::vector<std::string>& methods) :
        default_collection("collection.musly"),
        default_k(5),
        default_debuglevel(0),
        action(""),
        program_name(argv[0])
{
    optionstr["c"] = default_collection;
    optionstr["j"] = "";
    optionstr["n"] = "";
    optionstr["x"] = "";
    std::stringstream kstr;
    kstr << default_k;
    optionstr["k"] = kstr.str();
    optionstr["e"] = "-1";
    optionstr["f"] = "-1";

    // Build a CSV string with all methods available.
    all_methods = methods[0];
    for (int i = 1; i < (int)methods.size(); i++) {
        all_methods += "," + methods[i];
    }

    while (1) {

        int c = getopt(argc, argv, "v:ihc:Jj:a:x:Ee:f:Nn:k:ldm:s:p:");
        if (c == -1) {
            break;
        }

        switch (c) {


        // actions
        case 'E':
        case 'N':
            if (action.length() != 0) {
                action = "error";
            } else {
                action = static_cast<char>(tolower(c));
            }
            break;

        case 'i':
        case 'h':
        case 'a':
        case 'n':
        case 'e':
        case 'l':
        case 'd':
        case 'm':
        case 's':
        case 'p':
            if (action.length() != 0) {
                action = "error";
            } else {
                action = (char)(c);
                if (optarg) {
                    optionstr[action] = optarg;
                }
            }
            break;

        // parameters
        case 'v':
        case 'x':
        case 'c':
        case 'j':
        case 'k':
        case 'f':
            if (optarg) {
                std::string copt;
                copt = (char)(c);
                optionstr[copt] = optarg;
            }
            break;
        case 'J':
            optionstr["j"] = "*";
            break;

        // errors
        case '?':
            break;
        default:
            break;
        }
    }
    if (optind < argc) {
        action = "error";
    }

    // handle -J
    if (optionstr["j"] == "*") {
        optionstr["j"] = optionstr["c"] + ".jbox";
    }

    // show help if no action given
    if (action.length() == 0) {
        action = "error";
    }
}

programoptions::~programoptions() {
}

std::string
programoptions::get_action()
{
    return action;
}

std::string
programoptions::get_option_str(
        const std::string& option)
{
    if (optionstr.find(option) != optionstr.end()) {
        return optionstr[option];
    }

    return "";
}

int
programoptions::get_option_int(
        const std::string& option)
{
    if (optionstr.find(option) != optionstr.end()) {
        return atoi(optionstr[option].c_str());
    }

    return -1;
}


void
programoptions::display_help()
{
    using namespace std;
// Helper to format the output max 70 chars wide
//       ======================================================================
cout << "Options for " << program_name << endl;
cout << "  -h           this help screen." << endl;
cout << "  -v 0-5       set the libmusly debug level: (0: none, 5: trace)." << endl;
cout << "               DEFAULT: " << default_debuglevel << endl;
cout << "  -i           information about the music similarity library" << endl;
cout << "  -c COLL      set the file to write the music similarity features to" << endl
     << "               and to use for computing similarities." << endl
     << "               DEFAULT: " << default_collection << endl;
cout << "  -j JBOX | -J set the file to write the jukebox state to, to speed up" << endl
     << "               repeated calls of '-p', '-e', '-E', '-m', or '-s'." << endl
     << "               Use -J to set it to COLL.jbox." << endl
     << "               DEFAULT: Do not store the jukebox state on disk." << endl;
cout << "  -k NUM       set number of similar songs per item when computing" << endl
     << "               playlists ('-p'), sparse distance matrices ('-s')" << endl
     << "               or when evaluating the collection ('-e')." << endl
     << "               DEFAULT: " << default_k << endl;
cout << " INITIALIZATION:" << endl;
cout << "  -n MTH | -N  initialize the collection (set with '-c') using the" << endl
     << "               music similarity method MTH. Available methods:" << endl
     << "               " << all_methods << endl
     << "               '-N' automatically selects the best method." << endl;
cout << " MUSIC ANALYSIS/PLAYLIST GENERATION:" << endl;
cout << "  -a DIR/FILE  analyze and add the given audio FILE to the collection" << endl
     << "               file. If a Directory is given, the directory is scanned" << endl
     << "               recursively for audio files." << endl;
cout << "  -x EXT       only analyze files with file extension EXT when adding" << endl
     << "               audio files with '-a'. DEFAULT: '' (any)" << endl;
cout << "  -p FILE      print a playlist of the '-k' most similar tracks for" << endl
     << "               the given FILE. If FILE is not found in the collection" << endl
     << "               file, it is analyzed and then compared to all other" << endl
     << "               tracks found in the collection file ('-c')." << endl;
cout << " LISTING:" << endl;
cout << "  -l           list all files in the collection file." << endl;
cout << "  -d           dump the features in the collection file to the console" << endl;
cout << " EVALUATION:" << endl;
cout << "  -e NUM | -E  perform a basic kNN (k-nearest neighbor) music genre" << endl
     << "               classification experiment using the selected collection" << endl
     << "               file. The parameter k is set with option '-k'. The" << endl
     << "               genre is inferred from the path element at position NUM." << endl
     << "               The genre position within the path is guessed with '-E'." << endl;
cout << "  -f NUM       Use an artist filter for the evaluation ('-e'). The " << endl
     << "               artist name is inferred from the path element at" << endl
     << "               position NUM." << endl
     << "               DEFAULT: -1 (No artist filter)" << endl;
cout << "  -m FILE      compute the full similarity matrix for the specified" << endl
     << "               collection and write it to FILE. It is written in MIREX" << endl
     << "               text format (see http://www.music-ir.org/mirex under" << endl
     << "               Audio Music Similarity and Retrieval, Distance matrix" << endl
     << "               output files)." << endl;
cout << "  -s FILE      compute a sparse similarity matrix giving the k nearest" << endl
     << "               neighbors for each item of the specified collection and" << endl
     << "               write it to FILE. It is written in MIREX text format." << endl;
cout << endl;
//       ======================================================================
}
