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


#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <map>
#include <Eigen/Core>

#include "musly/musly.h"

#include "tools.h"
#include "programoptions.h"
#include "fileiterator.h"
#include "collectionfile.h"

musly_jukebox* mj = 0;


void
genre_from_filename(
        const std::vector<std::string>& tracks_files,
        int e,
        std::map<int, std::string>& genre_ids,
        std::vector<int>& genres)
{
    int prefix_len = 0;
    if (e < 0) {
        prefix_len = longest_common_prefix(tracks_files).length();
        e = 0;
    }

    std::map<std::string, int> genre_idx;
    int running_id = 0;

    for (size_t i = 0; i < tracks_files.size(); i++) {

        std::string file = tracks_files[i].substr(prefix_len,
                tracks_files[i].length()-prefix_len);

        std::vector<std::string> filesplit = split(file, '/');
        if (e < (int)filesplit.size()) {
            std::string g = filesplit[e];
            int id;
            if (genre_idx.find(g) != genre_idx.end()) {
                id = genre_idx[g];
            } else {
                genre_idx[g] = running_id;
                genre_ids[running_id] = g;
                id = running_id;
                running_id++;
            }
            genres.push_back(id);
        } else {
            std::string g = "Unknown";
            int id = -1;
            if (genre_idx.find(g) == genre_idx.end()) {
                genre_ids[id] = g;
            }
            genres.push_back(id);
        }
    }
}

int
read_collectionfile(
        collection_file& cf,
        char mode,
        std::vector<musly_track*>* tracks = 0,
        std::vector<std::string>* tracks_files = 0)
{
    // check if collection file is readable
    if (!cf.open("r+b")) {
        std::cerr << "Collection file: " << cf.get_file() << " not found."
                << std::endl;
        std::cerr << "Initialize with '-n'" << std::endl;

        return -1;
    }

    // check if the file is valid
    if (!cf.read_header()) {
        std::cerr << "Collection file: " << cf.get_file() << " invalid."
                << std::endl;
        std::cerr << "Reinitialize with '-n'" << std::endl;

        return -1;
    }

    // check if we can initialize musly with the given method
    const std::string method = cf.get_method();
    mj = musly_jukebox_poweron(method.c_str(), 0);
    if (!mj) {
        std::cerr << "Unknown Musly method: " << method << std::endl;
        return -1;
    } else {
        std::cout << "Initialized music similarity method: " << mj->method_name
                << std::endl;
        std::cout << "~~~" << std::endl;
        std::cout << musly_jukebox_aboutmethod(mj) << std::endl;
        std::cout << "~~~" << std::endl;
        std::cout << "Installed audio decoder: " << mj->decoder_name
                << std::endl;
    }

    // skip files && read files/tracks in database
    std::string current_file;
    int buffersize = musly_track_binsize(mj);
    unsigned char* buffer =
            new unsigned char[buffersize];
    musly_track* mt = musly_track_alloc(mj);

    // read the collection file, and position cursor after the last
    // record
    std::cout << "Reading collection file: " << cf.get_file() << std::endl;
    int count = 0;
    int read = 0;
    while ((read = cf.read_track(buffer, buffersize, current_file)) >= 0) {

        // 'list files' mode
        if (mode == 'l') {
            std::cout << "track-id: " << count << ", track-size: " << read
                    << " bytes, track-origin: " << current_file << std::endl;
        // 'dump tracks' mode
        } else if (mode == 'd') {
            std::cout << current_file << std::endl;
            if (musly_track_frombin(mj, buffer, mt) > 0) {
                std::cout << musly_track_tostr(mj, mt) << std::endl;
            }
        } else if (mode == 't') {
            musly_track* current_mt = musly_track_alloc(mj);
            if (musly_track_frombin(mj, buffer, current_mt) > 0) {
                tracks->push_back(current_mt);
                tracks_files->push_back(current_file);
            }
        } else if (mode == 'q') {
            // do nothing, read the next track
        }

        count++;
    }

    delete[] buffer;
    musly_track_free(mt);



    return count;
}

std::vector<musly_trackid>
initialize_collection(std::vector<musly_track*>& tracks)
{
    std::vector<musly_trackid> trackids(tracks.size(), -1);

    int ret = musly_jukebox_setmusicstyle(mj, tracks.data(),
            tracks.size());
    if (ret != 0) {
        std::cout << "ERROR" << std::endl;
        return trackids;
    }

    // ignore return value, return trackids anyways.
    ret = musly_jukebox_addtracks(mj, tracks.data(), trackids.data(),
            tracks.size());
    if (ret != 0) {
        return trackids;
    }

    return trackids;
}



int
write_mirex(
        std::vector<musly_track*>& tracks,
        std::vector<musly_trackid>& trackids,
        std::vector<std::string>& tracks_files,
        const std::string& file,
        const std::string& method)
{
    std::ofstream f;
    f.open(file.c_str());
    if (!f.is_open()) {
        return -1;
    }

    f << "Musly MIREX similarity matrix (Version: " << musly_version()
            << "), Method: " <<
            method << std::endl;

    for (int i = 0; i < (int)tracks_files.size(); i++) {
        f << i+1 << "\t" << tracks_files[i] << std::endl;
    }

    f << "Q/R";
    for (int i = 0; i < (int)tracks_files.size(); i++) {
        f << "\t" << i+1;
    }
    f << std::endl;

    std::vector<float> similarities(tracks.size());
    for (int i = 0; i < (int)tracks.size(); i++) {
        int ret = musly_jukebox_similarity(mj, tracks[i], trackids[i],
                tracks.data(), trackids.data(),
                tracks.size(), similarities.data());
        if (ret != 0) {
            fill(similarities.begin(), similarities.end(),
                    std::numeric_limits<float>::max());
        }

        // write to file
        f << i+1;
        for (int j = 0; j < (int)similarities.size(); j++) {
            f << '\t' << similarities[j];
        }
        f << std::endl;
    }

    f.close();

    return 0;
}


class musly_distsort
{
private:
    float* dists;

public:
    musly_distsort(
            float* dist)
    {
        this->dists = dist;
    }

    int operator() (
            size_t i,
            size_t j)
    {
        return (dists[i] < dists[j]);
    }
};

std::string
compute_playlist(
        std::vector<musly_track*>& tracks,
        std::vector<musly_trackid>& trackids,
        std::vector<std::string>& tracks_files,
        musly_track* seed_track,
        musly_trackid seed_trackid,
        int k)
{
    k = std::min(k, (int)tracks.size());

    std::vector<float> similarities(tracks.size());
    Eigen::VectorXi track_ids(tracks.size());

    int ret = musly_jukebox_similarity(mj, seed_track, seed_trackid,
            tracks.data(), trackids.data(),
            tracks.size(), similarities.data());
    if (ret != 0) {
        return "";

    }

    // TODO: use partial_sort
    track_ids =
            Eigen::VectorXi::LinSpaced(tracks.size(), 0, tracks.size()-1);
    std::sort(track_ids.data(), track_ids.data()+track_ids.size(),
            musly_distsort(similarities.data()));

    std::ostringstream pl;
    int i = 0;
    int ik = 0;
    while ((ik < k) && (i < (int)tracks.size())) {
        int j = track_ids(i);
        if (j == seed_trackid) {
            i++;
            continue;
        }
        float sim = similarities[j];
        if (sim >= 0) {
            pl << tracks_files[j] << std::endl;
            ik++;
        }
        i++;
    }

    return pl.str();
}



Eigen::MatrixXi
evaluate_collection(
        std::vector<musly_track*>& tracks,
        std::vector<musly_trackid>& trackids,
        std::vector<int>& genres,
        int num_genres,
        const std::vector<std::string> tracks_files,
        int f,
        int k)
{
    std::vector<float> similarities(tracks.size());
    Eigen::VectorXi genre_hist(num_genres);
    Eigen::VectorXi track_ids(tracks.size());
    Eigen::MatrixXi genre_confusion =
            Eigen::MatrixXi::Zero(num_genres, num_genres);

    k = std::min(k, (int)tracks.size());

    for (int i = 0; i < (int)tracks.size(); i++) {

        // compute the similarities and find the k most similar tracks
        int ret = musly_jukebox_similarity(mj, tracks[i], trackids[i],
                tracks.data(), trackids.data(),
                tracks.size(), similarities.data());
        if (ret != 0) {
            std::cerr << "Failed to compute similar tracks. Skipping."
                    << std::endl;
            continue;
        }

        // TODO: use partial_sort
        track_ids =
                Eigen::VectorXi::LinSpaced(tracks.size(), 0, tracks.size()-1);
        std::sort(track_ids.data(), track_ids.data()+track_ids.size(),
                musly_distsort(similarities.data()));

        // retrieve the genre of the current music piece
        int g = genres[i];

        // handle the special case of the "Unknown" (-1) music genre
        if (g < 0) {
            g = num_genres-1;
        }

        // split the filename in components
        std::vector<std::string> seed_filesplit;
        if (f >= 0) {
            seed_filesplit = split(tracks_files[i], '/');
        }

        // predicted genre is decided by a majority vote of its closest k
        // neighbors
        genre_hist.fill(0);
        int j = 0;
        int jk = 0;
        while (jk < k) {
            // skip the track itself
            if (track_ids[j] == i) {
                j++;
                continue;
            }

            std::vector<std::string> query_filesplit;
            if (f >= 0) {

                query_filesplit = split(tracks_files[track_ids[j]], '/');
                if (((int)query_filesplit.size() > f) &&
                        ((int)seed_filesplit.size() > f)) {

                    // Do artist filtering.
                    if (seed_filesplit[f].compare(query_filesplit[f]) == 0) {
                        j++;
                        continue;
                    }

                    // else proceed with the evaluation of the song.
                } else {
                    std::cerr << "Something in the artist filter function " <<
                            "went wrong... continuing." << std::endl;
                }
            }


            int gj = genres[track_ids[j]];
            if ((gj >= 0) && (gj < num_genres)) {
                genre_hist[gj]++;
            } else if (gj == -1) {
                genre_hist[num_genres-1]++;
            } else {
                // invalid
                std::cerr << "Something went wrong..." << std::endl;
            }
            j++;
            jk++;
        }
        int g_predicted;
        genre_hist.maxCoeff(&g_predicted);

        // update the confusion matrix
        genre_confusion(g, g_predicted)++;
    }

    return genre_confusion;
}

int
main(int argc, char *argv[])
{
    std::cout << "Music Similarity Library (Musly) - http://www.musly.org"
            << std::endl;
    std::cout << "Version: " << musly_version() << std::endl;
    std::cout << "(c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>"
            << std::endl << std::endl;

    // Check if we compiled any music similarity methods
    std::vector<std::string> ms = split(musly_jukebox_listmethods(), ',');
    if (ms.size() < 1) {
        std::cerr << "No music similarity method found. Aborting." << std::endl;
        return 1;
    }

    int ret = 0;

    // Parse the program options
    programoptions po(argc, argv, ms);

    // initialize the collection file.
    // note: the file is not opened/read at this point
    collection_file cf(po.get_option_str("c"));

    // set the debug level
    int debug_level = po.get_option_int("v");
    if (debug_level > 0) {
        std::cout << "Set debug level to: " << debug_level << std::endl;
        musly_debug(debug_level);
    }

    // -h: want help!
    if (po.get_action() == "h") {
        po.display_help();

    // -A: about musly
    } else if (po.get_action() == "i") {
        std::cout << "Version: " << musly_version() << std::endl;
        std::cout << "Available similarity methods: " <<
                musly_jukebox_listmethods() << std::endl;
        std::cout << "Available audio file decoders: " <<
                musly_jukebox_listdecoders() << std::endl;

    // wrong parameter combination
    } else if (po.get_action() == "error") {
        std::cerr << "Error: Invalid parameter combination!" << std::endl;
        std::cerr << "Use '-h' for more information." << std::endl;

        // indicate error
        ret = 1;

    // -a: add a song to the collection
    } else if (po.get_action() == "a") {

        // read the collection file in quiet ('q') mode
        int track_count = read_collectionfile(cf, 'q');
        if (track_count < 0) {
            // the error message was already displayed. just quit here.
            if (mj) {
                musly_jukebox_poweroff(mj);
            }
            return ret;
        }
        std::cout << "Read " << track_count << " musly tracks." << std::endl;

        // search for new files and analyze them
        std::string file;
        fileiterator fi(po.get_option_str("a"), po.get_option_str("x"));
        int buffersize = musly_track_binsize(mj);
        unsigned char* buffer =
                new unsigned char[buffersize];
        musly_track* mt = musly_track_alloc(mj);
        int count = 1;
        if (fi.get_nextfilename(file)) {
            do {
                if (cf.contains_track(file)) {
                    std::cout << "Skipping file #" << count
                            << ", already in Musly collection: "
                            << cf.get_file() << std::endl;
                    std::cout << file << std::endl << std::endl;
                    count++;

                    continue;
                }
                std::cout << "Musly analyzing file #" << count << std::endl;
                std::cout << file << std::endl;
                int ret = musly_track_analyze_audiofile(mj, file.c_str(), 120, mt);
                if (ret == 0) {
                    int serialized_buffersize =
                            musly_track_tobin(mj, mt, buffer);
                    if (serialized_buffersize == buffersize) {
                        cf.append_track(file, buffer, buffersize);
                        std::cout << "Appending to collection file: "
                                << cf.get_file()
                                << std::endl;
                        count++;
                    } else {
                        std::cout << "Serialization failed." << std::endl;
                        // Do not append failed files
                        //cf.append_track(file, 0, 0);
                    }

                } else {
                    std::cout << "Analysis failed." << std::endl;
                    // Do not append failed files
                    //cf.append_track(file, 0, 0);
                }
                std::cout << std::endl;

            } while (fi.get_nextfilename(file));

            // TODO: Make analysis run in parallel/multi-threaded

        } else {
            std::cout << "No files found while scanning: " <<
                    po.get_option_str("a") << std::endl;
        }
        delete[] buffer;
        musly_track_free(mt);

    // -n: new collection file
    } else if (po.get_action() == "n") {
        // the selected method
        std::string method = po.get_option_str("n");

        // check if we can initialize musly here!
        mj = musly_jukebox_poweron(method.c_str(), 0);
        if (!mj) {
            std::cerr << "Unknown Musly method: " << method << std::endl;
            return 1;
        }

        // check if we can initialize a new collection file
        cf.open("wb");

        std::cout << "Initialized music similarity method: " << mj->method_name
                << std::endl;
        std::cout << "~~~" << std::endl;
        std::cout << musly_jukebox_aboutmethod(mj) << std::endl;
        std::cout << "~~~" << std::endl;
        std::cout << "Installed audio decoder: " << mj->decoder_name
                << std::endl;
        std::cout << "Initializing new collection: " <<
                po.get_option_str("c") << std::endl;
        std::cout << "Initialization result: "<< std::flush;
        if (cf.write_header(mj->method_name)) {
            std::cout << "OK." << std::endl;
        } else {
            std::cout << "failed." << std::endl;
        }

    // -e: evaluation
    } else if (po.get_action() == "e") {

        // read collection file to memory
        std::vector<musly_track*> tracks;
        std::vector<std::string> tracks_files;
        int track_count = read_collectionfile(cf, 't', &tracks, &tracks_files);
        if (track_count < 0) {
            if (mj) {
                musly_jukebox_poweroff(mj);
            }
            return -1;
        }
        std::cout << "Loaded " << track_count << " musly tracks to memory."
                << std::endl;

        // initialize all loaded tracks
        std::vector<musly_trackid> trackids = initialize_collection(tracks);

        // do we need an artist filter
        int f = po.get_option_int("f");

        // get the position of the genre in the path
        int e = po.get_option_int("e");
        // try to extract the genre from the filename
        std::vector<int> genres;
        std::map<int, std::string> genre_ids;
        genre_from_filename(tracks_files, e, genre_ids, genres);

        int k = po.get_option_int("k");
        std::cout << "k-NN Genre classification (k=" << k << "): "
                << cf.get_file() << std::endl;
        Eigen::MatrixXi genre_confusion = evaluate_collection(
                tracks, trackids,
                genres, genre_ids.size(),
                tracks_files, f,
                k);

        std::cout << "Genre Confusion matrix:" << std::endl;
        std::cout << genre_confusion << std::endl;
        std::cout << "Correctly classified: " << genre_confusion.diagonal().sum()
                << "/" << genre_confusion.sum() << " (" <<
                ((float)genre_confusion.diagonal().sum()/
                        (float)genre_confusion.sum())*100.0 << "%)"<< std::endl;


        // free the tracks
        for (int i = 0; i < (int)tracks.size(); i++) {
            musly_track* mti = tracks[i];
            musly_track_free(mti);
        }

    // -l: list files in collection file
    } else if (po.get_action() == "l") {
        // read the collection file in listing ('l') mode
        int track_count = read_collectionfile(cf, 'l');
        if (track_count < 0) {
            ret = -1;
        }

    // -d: dump the features in a human readable (text) format
    } else if (po.get_action() == "d") {
        // read collection file in dump ('d') mode
        int track_count = read_collectionfile(cf, 'd');
        if (track_count < 0) {
            ret = -1;
        }

    // -m: write a MIREX similarity matrix to the given file
    } else if (po.get_action() == "m") {
        std::string file = po.get_option_str("m");

        // read collection file to memory
        std::vector<musly_track*> tracks;
        std::vector<std::string> tracks_files;
        int track_count = read_collectionfile(cf, 't', &tracks, &tracks_files);
        if (track_count < 0) {
            ret = -1;
        }
        std::cout << "Loaded " << track_count << " musly tracks to memory."
                << std::endl;

        // initialize all loaded tracks
        std::vector<musly_trackid> trackids = initialize_collection(tracks);

        // could all tracks be initialized correctly?
        if (trackids.size() == tracks.size()) {
            // compute a similarity matrix and write MIREX formatted to the
            // given file
            std::cout << "Computing and writing similarity matrix to: " << file
                    << std::endl;
            ret = write_mirex(tracks, trackids, tracks_files, file,
                    cf.get_method());
            if (ret == 0) {
                std::cout << "Success." << std::endl;
            } else {
                std::cerr << "Failed to open file for writing." << std::endl;
            }
        }

        // free the tracks
        for (int i = 0; i < (int)tracks.size(); i++) {
            musly_track* mti = tracks[i];
            musly_track_free(mti);
        }
    } else if (po.get_action() == "p") {
        std::string seed_file = po.get_option_str("p");

        // read collection file to memory
        std::vector<musly_track*> tracks;
        std::vector<std::string> tracks_files;
        int track_count = read_collectionfile(cf, 't', &tracks, &tracks_files);
        if (track_count < 0) {
            ret = -1;
        }
        std::cout << "Loaded " << track_count << " musly tracks to memory."
                << std::endl;

        // initialize all loaded tracks
        std::vector<musly_trackid> trackids = initialize_collection(tracks);
        if (trackids.size() == tracks.size()) {
            // compute a single playlist
            int k = po.get_option_int("k");
            std::cout << "Computing the k=" << k << " most similar tracks to: "
                    << seed_file << std::endl;

            std::vector<std::string>::iterator it = std::find(
                    tracks_files.begin(), tracks_files.end(), seed_file);

            if (it != tracks_files.end()) {
                size_t pos = std::distance(tracks_files.begin(), it);
                std::string pl = compute_playlist(tracks, trackids, tracks_files,
                        tracks[pos], trackids[pos], k);
                if (pl == "") {
                    std::cerr << "Failed to compute similar tracks for given file."
                            << std::endl;
                } else {
                    std::cout << pl;
                }
            }
        }

        // free the tracks
        for (int i = 0; i < (int)tracks.size(); i++) {
            musly_track* mti = tracks[i];
            musly_track_free(mti);
        }

    }

    // cleanup
    if (mj) {
        musly_jukebox_poweroff(mj);
    }

    return ret;
}

