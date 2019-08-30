/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *           2014-2016, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <map>
#include <Eigen/Core>

#define MUSLY_SUPPORT_STDIO
#include "musly/musly.h"

#include "tools.h"
#include "programoptions.h"
#include "fileiterator.h"
#include "collectionfile.h"

musly_jukebox* mj = 0;



int
read_collectionfile(
        collection_file& cf,
        char mode,
        std::vector<musly_track*>* tracks = NULL,
        std::vector<std::string>* tracks_files = NULL)
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

bool read_jukebox(std::string &filename, musly_jukebox** jukebox, int* last_reinit) {
    std::cout << "Reading jukebox file: " << filename << std::endl;
    if (FILE* f = fopen(filename.c_str(), "rb")) {
        *jukebox = musly_jukebox_fromstream(f);
        if (*jukebox && fread(last_reinit, sizeof(*last_reinit), 1, f) != 1) {
            *last_reinit = 0;
        }
        fclose(f);
        return (*jukebox != NULL);
    }
    return false;
}

bool write_jukebox(std::string &filename, musly_jukebox* jukebox, int last_reinit) {
    std::cout << "Writing jukebox file: " << filename << std::endl;
    if (FILE* f = fopen(filename.c_str(), "wb")) {
        bool result = (musly_jukebox_tostream(jukebox, f) > 0)
                && (fwrite(&last_reinit, sizeof(last_reinit), 1, f) == 1);
        fclose(f);
        return result;
    }
    return false;
}

void
tracks_add(collection_file& cf, std::string directory_or_file, std::string extension) {
    fileiterator fi(directory_or_file, extension);
    std::string afile;
    if (!fi.get_nextfilename(afile)) {
        std::cout << "No files found while scanning: " <<
                directory_or_file << std::endl;
    }
    else {
        int buffersize = musly_track_binsize(mj);
#ifdef _OPENMP
        // collect all file names in a vector first
        std::vector<std::string> files;
        do {
            files.push_back(afile);
        } while (fi.get_nextfilename(afile));
        #pragma omp parallel if (files.size() > 1)
#endif
        {
        unsigned char* buffer =
                new unsigned char[buffersize];
        musly_track* mt = musly_track_alloc(mj);
#ifdef _OPENMP
        // do a parallel for loop over the collected file names
        // use a dynamic schedule because computation may differ per file
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < (int)files.size(); i++) {
            // set file to files[i] for the loop body
            std::string& file = files[i];
#else
        // do a while loop over the fileiterator
        int i = 0;
        do {
            // set file to our existing afile for the loop body
            std::string& file = afile;
#endif
            if (cf.contains_track(file)) {
#ifdef _OPENMP
                #pragma omp critical
#endif
                {
                std::cout << "Skipping already analyzed [" << i+1 << "]: "
                        << limit_string(file, 60) << std::endl;
                }  // pragma omp critical
                continue;
            }
#ifndef _OPENMP
            std::cout << "Analyzing [" << i+1 << "]: "
                    << limit_string(file, 60) << std::flush;
#endif
            int ret = musly_track_analyze_audiofile(mj, file.c_str(), 30, -48, mt);
#ifdef _OPENMP
            #pragma omp critical
            {
            std::cout << "Analyzing [" << i+1 << "]: "
                    << limit_string(file, 60);
#endif
            if (ret == 0) {
                int serialized_buffersize =
                        musly_track_tobin(mj, mt, buffer);
                if (serialized_buffersize == buffersize) {
                    cf.append_track(file, buffer, buffersize);
                    std::cout << " - [OK]" << std::endl;
                } else {
                    std::cout << " - [FAILED]." << std::endl;
                }

            } else {
                std::cout << " - [FAILED]." << std::endl;
            }
#ifdef _OPENMP
            }  // pragma omp critical
        }  // for loop
#else
            i++;
        } while (fi.get_nextfilename(afile));
#endif
        delete[] buffer;
        musly_track_free(mt);
        }  // pragma omp parallel
    }
}


bool
tracks_initialize(
        std::vector<musly_track*>& tracks)
{
    std::vector<musly_trackid> trackids(tracks.size(), -1);

    // initialize the jukebox music style
    int ret;
    if (trackids.size() <= 1000) {
        // use all available tracks
        ret = musly_jukebox_setmusicstyle(mj, tracks.data(),
                static_cast<int>(tracks.size()));
    }
    else {
        // use a random subset of 1000 tracks
        std::vector<musly_track*> tracks2(tracks);
        std::random_shuffle(tracks2.begin(), tracks2.end());
        ret = musly_jukebox_setmusicstyle(mj, tracks2.data(), 1000);
    }
    if (ret != 0) {
        return false;
    }

    // add the tracks to the jukebox
    ret = musly_jukebox_addtracks(mj, tracks.data(), trackids.data(),
            static_cast<int>(tracks.size()), true);
    if (ret != 0) {
        return false;
    }

    // do a sanity check of the trackids
    for (int i = 0; i < (int)trackids.size(); i++) {
        if (i != trackids[i]) {
            return false;
        }
    }

    return true;
}


void
tracks_free(
        std::vector<musly_track*>& tracks)
{
    // free the tracks
    for (int i = 0; i < (int)tracks.size(); i++) {
        musly_track* mti = tracks[i];
        musly_track_free(mti);
    }
}


int
write_mirex_full(
        std::vector<musly_track*>& tracks,
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

    std::vector<musly_trackid> alltrackids(tracks.size());
    for (int i = 0; i < (int)alltrackids.size(); i++) {
        alltrackids[i] = i;
    }

#ifdef _OPENMP
    #pragma omp parallel
    {
#endif
    std::vector<float> similarities(tracks.size());
#ifdef _OPENMP
    #pragma omp for schedule(static) ordered
#endif
    for (int i = 0; i < (int)tracks.size(); i++) {
        int ret = musly_jukebox_similarity(mj, tracks[i], i,
                tracks.data(), alltrackids.data(),
                static_cast<int>(tracks.size()), similarities.data());
        if (ret != 0) {
            fill(similarities.begin(), similarities.end(),
                    std::numeric_limits<float>::max());
        }

#ifdef _OPENMP
        #pragma omp ordered
        {
#endif
        // write to file
        f << i+1;
        for (int j = 0; j < (int)similarities.size(); j++) {
            f << '\t' << similarities[j];
        }
        f << std::endl;
#ifdef _OPENMP
        }  // pragma omp ordered
#endif
    }
#ifdef _OPENMP
    }  // pragma omp parallel
#endif

    f.close();

    return 0;
}


typedef std::pair<int, float> similarity_knn;
struct similarity_comp {
  bool
  operator()(
          similarity_knn const& lhs,
          similarity_knn const& rhs) {
    return lhs.second < rhs.second;
  }
};

std::vector<similarity_knn>
compute_similarity(
        musly_jukebox* local_mj,
        int k,
        std::vector<int>& artists,
        musly_trackid seed,
        std::vector<musly_track*>& alltracks,
        std::vector<musly_trackid>& alltrackids)
{
    int guess_len = std::max(k, (int)(alltracks.size()*0.1));
    std::vector<musly_trackid> guess_ids(guess_len);
    guess_len = musly_jukebox_guessneighbors(local_mj, seed,
            guess_ids.data(), guess_len);
    guess_ids.resize(std::max(guess_len, 0));

    std::vector<similarity_knn> knn_sim;
    std::vector<float> similarities;
    std::vector<musly_trackid>* ids;

    // need to compute the full similarity
    if (guess_len <= 0) {
        similarities.resize(alltracks.size());
        int ret = musly_jukebox_similarity(local_mj,
                alltracks[seed], seed, alltracks.data(),
                alltrackids.data(), static_cast<int>(alltrackids.size()), similarities.data());
        if (ret != 0) {
            return knn_sim;
        }
        ids = &alltrackids;

    // use the results of the pre-filter
    } else {
        std::vector<musly_track*> guess_tracks(guess_len);
        for (int i = 0; i < guess_len; i++) {
           guess_tracks[i] = alltracks[guess_ids[i]];
        }
        similarities.resize(guess_ids.size());
        int ret = musly_jukebox_similarity(local_mj,
                alltracks[seed], seed, guess_tracks.data(),
                guess_ids.data(), static_cast<int>(guess_ids.size()), similarities.data());
        if (ret != 0) {
            return knn_sim;
        }
        ids = &guess_ids;
    }

    for (int i = 0; i < (int)ids->size(); i++) {

        musly_trackid curid = ids->at(i);

        // skip self
        if (seed == curid) {
            continue;
        }

        // artist filter
        if ((artists.size() > 0) && (artists[seed] == artists[curid])) {
            continue;
        }

        if ((int)knn_sim.size() < k) {
            knn_sim.push_back(std::make_pair(curid, similarities[i]));
            std::push_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());

        // if the neighbors are already filled && our distance is smaller
        // than the maximum in the heap, update the heap
        } else if (similarities[i] < knn_sim.front().second) {
            std::pop_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());
            knn_sim.back() = std::make_pair(curid, similarities[i]);
            std::push_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());
        }
    }

    std::sort_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());

    return knn_sim;
}


int
write_mirex_sparse(
        std::vector<musly_track*>& tracks,
        std::vector<std::string>& tracks_files,
        const std::string& file,
        const std::string& method,
        int k)
{
    std::ofstream f(file.c_str());
    if (f.fail()) {
        return -1;
    }

    f << "Musly MIREX similarity matrix (Version: " << musly_version()
            << "), Method: " <<
            method << std::endl;

    k = std::min(k, (int)tracks.size());
    std::vector<int> artists_null; // disable artist filtering

    std::vector<musly_trackid> trackids(tracks.size());
    for (musly_trackid i = 0; i < (int)trackids.size(); i++) {
        trackids[i] = i;
    }

#ifdef _OPENMP
    #pragma omp parallel for schedule(static)
#endif
    for (int i = 0; i < (int)tracks.size(); i++) {
        // compute k nearest neighbors
        std::vector<similarity_knn> track_idx = compute_similarity(
                mj, k, artists_null,
                i, tracks, trackids);
        if (track_idx.size() == 0) {
            continue;
        }

#ifdef _OPENMP
        #pragma omp critical
        {
#endif
        // write to file
        f << tracks_files[i];
        for (int l = 0; l < k; l++) {
            int j = track_idx[l].first;
            f << "\t" << tracks_files[j] << "," << track_idx[l].second;
        }
        f << std::endl;
#ifdef _OPENMP
        }
#endif
    }

    f.close();

    return 0;
}


std::string
compute_playlist(
        std::vector<musly_track*>& alltracks,
        std::vector<musly_trackid>& alltrackids,
        std::vector<std::string>& tracks_files,
        musly_trackid seed,
        int k)
{
    k = std::min(k, (int)alltracks.size());
    std::vector<int> artists_null; // disable artist filtering
    std::vector<similarity_knn> track_idx = compute_similarity(
            mj, k, artists_null,
            seed, alltracks, alltrackids);
    if (track_idx.size() == 0) {
        return "";
    }
    k = (int)(track_idx.size());

    // write playlist
    std::ostringstream pl;
    for (int i = 0; i < k; i++) {
        int j = track_idx[i].first;
        pl << tracks_files[j] << std::endl;
    }

    return pl.str();
}



Eigen::MatrixXi
evaluate_collection(
        std::vector<musly_track*>& alltracks,
        std::vector<int>& genres,
        int num_genres,
        std::vector<int>& artists,
        int /*num_artists*/,
        int k)
{
    Eigen::MatrixXi genre_confusion =
            Eigen::MatrixXi::Zero(num_genres, num_genres);
    if (k >= (int)alltracks.size()) {
        std::cerr << "Evaluation failed. Too few tracks!" << std::endl;
        return genre_confusion;
    }

    std::vector<musly_trackid> alltrackids(alltracks.size());
    for (int i = 0; i < (int)alltracks.size(); i++) {
        alltrackids[i] = i;
    }

#ifdef _OPENMP
    #pragma omp parallel
    {
#endif
    Eigen::VectorXi genre_hist(num_genres);
    Eigen::MatrixXi thread_genre_confusion =
            Eigen::MatrixXi::Zero(num_genres, num_genres);
#ifdef _OPENMP
    #pragma omp for schedule(static)
#endif
    for (int i = 0; i < (int)alltracks.size(); i++) {

        std::vector<similarity_knn> knn_tracks = compute_similarity(
                mj, k, artists, i, alltracks, alltrackids);

        if (knn_tracks.size() == 0) {
            std::cerr << "Failed to compute similar tracks. Skipping."
                    << std::endl;
            continue;
        }

        // retrieve the genre of the current music piece
        int g = genres[i];

        // handle the special case of the "Unknown" (-1) music genre
        if (g < 0) {
            g = num_genres-1;
        }

        // predicted genre is decided by a majority vote of its closest k
        // neighbors
        genre_hist.fill(0);
        for (int j = 0; j < k; j++) {

            // get the index of the j'th knn
            int knn_idx = knn_tracks[j].first;
            int gj = genres[knn_idx];
            if ((gj >= 0) && (gj < num_genres)) {
                genre_hist[gj]++;
            } else if (gj == -1) {
                genre_hist[num_genres-1]++;
            } else {
                // invalid
                std::cerr << "Something went wrong..." << std::endl;
            }
        }
        int g_predicted;
        genre_hist.maxCoeff(&g_predicted);

        // update the confusion matrix
#ifdef _OPENMP
        thread_genre_confusion(g, g_predicted)++;
#else
        genre_confusion(g, g_predicted)++;
#endif
    }
#ifdef _OPENMP
    #pragma omp critical
    genre_confusion += thread_genre_confusion;
    }  // pragma omp parallel
#endif

    return genre_confusion;
}

int
main(int argc, char *argv[])
{
    std::cout << "Music Similarity Library (Musly) - http://www.musly.org"
            << std::endl;
    std::cout << "Version: " << musly_version() << std::endl;
    std::cout << "(c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>"
            << std::endl
            <<   "    2014-2016, Jan Schlüter <jan.schlueter@ofai.at>"
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

    // -i: about musly
    } else if (po.get_action() == "i") {
        std::cout << "Version: " << musly_version() << std::endl;
        std::cout << "Available similarity methods: " <<
                musly_jukebox_listmethods() << std::endl;
        std::cout << "Available audio file decoders: " <<
                musly_jukebox_listdecoders() << std::endl;
#ifdef _OPENMP
        std::cout << "OpenMP support: enabled" << std::endl;
#else
        std::cout << "OpenMP support: disabled" << std::endl;
#endif

    // wrong parameter combination
    } else if (po.get_action() == "error") {
        std::cerr << "Error: Invalid parameter combination!" << std::endl;
        std::cerr << "Use '-h' for more information." << std::endl;

        // indicate error
        ret = 1;

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
        if (!cf.open("wb")) {
            std::cerr << "Cannot create collection file: " << cf.get_file() << std::endl;
            return 1;
        }

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

    // -a: add one or more songs to the collection
    } else if (po.get_action() == "a") {

        // read the collection file in quiet ('q') mode
        int track_count = read_collectionfile(cf, 'q');
        if (track_count < 0) {
            // the error message was already displayed. just quit here.
            musly_jukebox_poweroff(mj);
            return ret;
        }
        std::cout << "Read " << track_count << " musly tracks." << std::endl;

        // search for new files, analyze and add them
        tracks_add(cf, po.get_option_str("a"), po.get_option_str("x"));

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

    } else {
        // For everything else, we need a filled jukebox.
        // We will read the collection file to memory and either initialize a
        // jukebox and register all tracks, or load a jukebox state from disk.

        // read collection file to memory
        std::vector<musly_track*> tracks;
        std::vector<std::string> tracks_files;
        int track_count = read_collectionfile(cf, 't', &tracks, &tracks_files);
        if (track_count < 0) {
            std::cerr << "Reading the collection failed." << std::endl;
            musly_jukebox_poweroff(mj);
            return -1;
        }
        std::cout << "Loaded " << track_count << " musly tracks to memory."
                << std::endl;

        // if a jukebox state file was given, try to read it
        std::string jukebox_file = po.get_option_str("j");
        int last_reinit = 0;
        if (!jukebox_file.empty()) {
            musly_jukebox* mj2 = NULL;
            if (!read_jukebox(jukebox_file, &mj2, &last_reinit)) {
                std::cout << "Reading failed.";
            }
            else if (strcmp(mj2->method_name, mj->method_name)) {
                std::cout << "Jukebox file is for method '" << mj2->method_name
                        << "', but collection file is for method '"
                        << mj->method_name << "'.";
            }
            else if (track_count < musly_jukebox_trackcount(mj2)) {
                std::cout << "Jukebox file is for " << musly_jukebox_trackcount(mj2)
                        << " tracks, but collection file has " << track_count
                        << " tracks only.";
            }
            else if (track_count == musly_jukebox_trackcount(mj2)) {
                // everything is fine, use loaded jukebox directly
                musly_jukebox_poweroff(mj);
                mj = mj2;
            }
            else if (track_count > (int)(last_reinit * 1.1f)) {
                std::cout << "Jukebox file was initialized for " << last_reinit
                        << " tracks, but collection file has " << track_count
                        << " tracks (an increase of over 10%).";
            }
            else {
                int num_new = track_count - musly_jukebox_trackcount(mj2);
                std::cout << "Jukebox file has " << num_new <<
                        " track(s) less than collection; updating..." << std::endl;
                musly_trackid* trackids = new musly_trackid[num_new];
                if (musly_jukebox_addtracks(mj2,
                        tracks.data() + track_count - num_new,
                        trackids, num_new, true) < 0) {
                    std::cout << "Updating jukebox failed." << std::endl;
                }
                else {
                    // updating went fine, use loaded jukebox
                    musly_jukebox_poweroff(mj);
                    mj = mj2;
                    // and write updated jukebox
                    write_jukebox(jukebox_file, mj, last_reinit);
                }
            }
            if (mj != mj2) {
                std::cout << std::endl << "Initializing new jukebox..." << std::endl;
            }
        }
        else {
            std::cout << "Initializing jukebox..." << std::endl;
        }

        if (!musly_jukebox_trackcount(mj)) {
            // initialize all loaded tracks
            if (!tracks_initialize(tracks)) {
                std::cerr << "Initialization failed! Aborting" << std::endl;
                tracks_free(tracks);
                musly_jukebox_poweroff(mj);
                return -1;
            }
            else if (!jukebox_file.empty()) {
                // if a jukebox state file was given, update it
                write_jukebox(jukebox_file, mj, track_count);
            }
        }

        // -e: evaluation
        if (po.get_action() == "e") {

            // do we need an artist filter
            int f = po.get_option_int("f");
            std::vector<int> artists;
            std::map<int, std::string> artist_ids;
            if (f >= 0) {
                field_from_strings(tracks_files, f, artist_ids, artists);
                std::cout << "Artist filter active (-f)." << std::endl
                        << "Found " << artist_ids.size() << " artists."
                        << std::endl;
            }

            // get the position of the genre in the path
            int e = po.get_option_int("e");
            std::vector<int> genres;
            std::map<int, std::string> genre_ids;
            field_from_strings(tracks_files, e, genre_ids, genres);
            std::cout << "Found " << genre_ids.size() << " genres." << std::endl;

            int k = po.get_option_int("k");
            std::cout << "k-NN Genre classification (k=" << k << "): "
                    << cf.get_file() << std::endl;

            std::cout << "Evaluating collection..." << std::endl;
            Eigen::MatrixXi genre_confusion = evaluate_collection(tracks,
                    genres, static_cast<int>(genre_ids.size()), artists,
                    static_cast<int>(artist_ids.size()), k);

            std::cout << "Genre Confusion matrix:" << std::endl;
            std::cout << genre_confusion << std::endl;
            std::cout << "Correctly classified: " << genre_confusion.diagonal().sum()
                    << "/" << genre_confusion.sum() << " (" <<
                    ((float)genre_confusion.diagonal().sum()/
                            (float)genre_confusion.sum())*100.0 << "%)"<< std::endl;


        // -m: write a MIREX full similarity matrix to the given file
        // -s: write a MIREX sparse similarity matrix to the given file
        } else if (po.get_action() == "m" || po.get_action() == "s") {
            std::string file = po.get_option_str(po.get_action());

            // compute a similarity matrix and write MIREX formatted to the
            // given file
            std::cout << "Computing and writing similarity matrix to: " << file
                    << std::endl;
            if (po.get_action() == "m") {
                std::cout << "Note: no neighbor guessing is applied here!" << std::endl;
                ret = write_mirex_full(tracks, tracks_files, file, cf.get_method());
            } else {
                int k = po.get_option_int("k");
                ret = write_mirex_sparse(tracks, tracks_files, file, cf.get_method(), k);
            }
            if (ret == 0) {
                std::cout << "Success." << std::endl;
            } else {
                std::cerr << "Failed to open file for writing." << std::endl;
            }

        // -p: compute and display a playlist for a single seed track
        } else if (po.get_action() == "p") {
            std::string seed_file = po.get_option_str("p");

            std::vector<std::string>::iterator it = std::find(
                    tracks_files.begin(), tracks_files.end(), seed_file);
            if (it == tracks_files.end()) {
                std::cerr << "File not found in collection! Aborting." << std::endl;
                tracks_free(tracks);
                musly_jukebox_poweroff(mj);
                return -1;
            }

            // compute a single playlist
            int k = po.get_option_int("k");
            std::cout << "Computing the k=" << k << " most similar tracks to: "
                    << seed_file << std::endl;
            std::vector<musly_trackid> trackids(tracks.size());
            for (int i = 0; i < (int)trackids.size(); i++) {
                trackids[i] = i;
            }
            musly_trackid seed = static_cast<int>(std::distance(tracks_files.begin(), it));
            std::string pl = compute_playlist(tracks, trackids, tracks_files,
                    seed, k);
            if (pl == "") {
                std::cerr << "Failed to compute similar tracks for given file."
                        << std::endl;
            } else {
                std::cout << pl;
            }
        }

        // cleanup
        tracks_free(tracks);
    }

    // cleanup
    musly_jukebox_poweroff(mj);

    return ret;
}

