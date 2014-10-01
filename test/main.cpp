/*
 * Copyright 2014, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define _USE_MATH_DEFINES
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "musly/musly.h"
#include "tools.h"
#include "idpool.h"
#include "minilog.h"

/** poor man's test framework */
int FAILED = 0;
int PASSED = 0;
#define REQUIRE(msg, val) \
    if (!(val)) {std::cout << "Failed: " << msg << \
        " (in test/main.cpp, line " << __LINE__ << ")" \
        << std::endl; FAILED += 1; } else { PASSED += 1; }
#define SUMMARY() \
    std::cout << std::endl << PASSED << " tests passed, " << FAILED << " tests failed." << std::endl;


void test_unordered_idpool() {
    std::cout << "Testing component \"unordered_idpool\"..." << std::endl;

    musly::unordered_idpool<int> pool;
    int count;
    REQUIRE( "initially empty", pool.get_size() == 0 );

    int generate_some[4];
    pool.generate_ids(generate_some, 4);
    REQUIRE( "generated some", pool.get_size() == 4 );
    REQUIRE( "generated 0", generate_some[0] == 0 );
    REQUIRE( "generated 1", generate_some[1] == 1 );
    REQUIRE( "generated 2", generate_some[2] == 2 );
    REQUIRE( "generated 3", generate_some[3] == 3 );
    REQUIRE( "max_seen 3", pool.get_max_seen() == 3 );

    int add_some[] = {3, 5, 7};
    count = pool.add_ids(add_some, 3);
    REQUIRE( "added some", pool.get_size() == 6 );
    REQUIRE( "added 2", count == 2 );
    REQUIRE( "max_seen 7", pool.get_max_seen() == 7);

    int remove_some[] = {3, 4, 6};
    count = pool.remove_ids(remove_some, 3);
    REQUIRE( "removed some", pool.get_size() == 5 );
    REQUIRE( "removed 1", count == 1 );
    REQUIRE( "max_seen 7", pool.get_max_seen() == 7);

    int add_more[] = {1, 11, 10, 3, 12, 9};
    count = pool.add_ids(add_more, 6);
    REQUIRE( "added more", pool.get_size() == 10 );
    REQUIRE( "added 5", count == 5 );
    REQUIRE( "max_seen 12", pool.get_max_seen() == 12);

    int remove_more[] = {1, 12};
    count = pool.remove_ids(remove_more, 2);
    REQUIRE( "removed more", pool.get_size() == 8 );
    REQUIRE( "removed 2", count == 2 );
    REQUIRE( "max_seen 12", pool.get_max_seen() == 12);

    int generate_more[1];
    pool.generate_ids(generate_more, 1);
    REQUIRE( "generated more", pool.get_size() == 9 );
    REQUIRE( "generated 13", generate_more[0] == 13 );
}


void check_ordered_idpool_mapping(musly::ordered_idpool<int>& pool) {
    REQUIRE( "size consistency", pool.get_size() == (int) pool.posmap().size() );
    REQUIRE( "size consistency", pool.get_size() == (int) pool.idlist().size() );
    for (int i = 0; i < pool.get_size(); i++) {
        REQUIRE( "mapping consistency", pool.position_of(pool.idlist()[i]) == i );
    }
}


void test_ordered_idpool() {
    std::cout << "Testing component \"ordered_idpool\"..." << std::endl;

    musly::ordered_idpool<int> pool;
    int count;
    REQUIRE( "initially empty", pool.get_size() == 0 );

    int generate_some[4];
    pool.generate_ids(generate_some, 4);
    REQUIRE( "generated some", pool.get_size() == 4 );
    REQUIRE( "generated 0", generate_some[0] == 0 );
    REQUIRE( "generated 1", generate_some[1] == 1 );
    REQUIRE( "generated 2", generate_some[2] == 2 );
    REQUIRE( "generated 3", generate_some[3] == 3 );
    REQUIRE( "max_seen 3", pool.get_max_seen() == 3 );
    REQUIRE( "position 0", pool.position_of(0) == 0 );
    REQUIRE( "position 1", pool.position_of(1) == 1 );
    REQUIRE( "position 2", pool.position_of(2) == 2 );
    REQUIRE( "position 3", pool.position_of(3) == 3 );
    check_ordered_idpool_mapping(pool);

    int add_some[] = {3, 5, 7};
    count = pool.add_ids(add_some, 3);
    REQUIRE( "added some", pool.get_size() == 6 );
    REQUIRE( "added 2", count == 2 );
    REQUIRE( "max_seen 7", pool.get_max_seen() == 7);
    REQUIRE( "position 3", pool.position_of(3) == 3 );
    REQUIRE( "position 5", pool.position_of(5) == 4 );
    REQUIRE( "position 7", pool.position_of(7) == 5 );
    check_ordered_idpool_mapping(pool);

    int remove_some[] = {3, 4, 6};
    count = pool.remove_ids(remove_some, 3);
    REQUIRE( "removed some", pool.get_size() == 5 );
    REQUIRE( "removed 1", count == 1 );
    REQUIRE( "max_seen 7", pool.get_max_seen() == 7);
    check_ordered_idpool_mapping(pool);

    int add_more[] = {1, 11, 10, 3, 12, 9};
    count = pool.add_ids(add_more, 6);
    REQUIRE( "added more", pool.get_size() == 10 );
    REQUIRE( "added 5", count == 5 );
    REQUIRE( "max_seen 12", pool.get_max_seen() == 12);
    REQUIRE( "position 1", pool.position_of(1) == 4 );
    REQUIRE( "position 11", pool.position_of(11) == 5 );
    REQUIRE( "position 10", pool.position_of(10) == 6 );
    REQUIRE( "position 3", pool.position_of(3) == 7 );
    REQUIRE( "position 12", pool.position_of(12) == 8 );
    REQUIRE( "position 9", pool.position_of(9) == 9 );
    check_ordered_idpool_mapping(pool);
    count = pool.add_ids(add_more, 6);
    REQUIRE( "added more again", pool.get_size() == 10 );
    REQUIRE( "added 0", count == 0 );
    REQUIRE( "max_seen 12", pool.get_max_seen() == 12);
    REQUIRE( "position 1", pool.position_of(1) == 4 );
    REQUIRE( "position 11", pool.position_of(11) == 5 );
    REQUIRE( "position 10", pool.position_of(10) == 6 );
    REQUIRE( "position 3", pool.position_of(3) == 7 );
    REQUIRE( "position 12", pool.position_of(12) == 8 );
    REQUIRE( "position 9", pool.position_of(9) == 9 );
    check_ordered_idpool_mapping(pool);

    int remove_more[] = {1, 12};
    count = pool.remove_ids(remove_more, 2);
    REQUIRE( "removed more", pool.get_size() == 8 );
    REQUIRE( "removed 2", count == 2 );
    REQUIRE( "max_seen 12", pool.get_max_seen() == 12);
    check_ordered_idpool_mapping(pool);

    int generate_more[1];
    pool.generate_ids(generate_more, 1);
    REQUIRE( "generated more", pool.get_size() == 9 );
    REQUIRE( "generated 13", generate_more[0] == 13 );
    REQUIRE( "position 13", pool.position_of(13) == 8 );
    check_ordered_idpool_mapping(pool);
}


void generate_music(float* out, int length, unsigned int seed = 0) {
    if (!seed) {
        seed = time(NULL);
    }
    srand(seed);
    const float sample_rate = 22050.0f;
    // Create a signal by adding up some sine waves
    std::fill(out, &out[length], 0.0f);
    for (int i = 5 + rand() % 20; i >= 0; i--) {
        int len = length / 10 + rand() % (length / 10);
        int start = rand() % (length - len);
        float basefreq = 100 + 1000.0f * pow(rand() / (float)(RAND_MAX), 2);
        float baseamp = 0.1f + 0.9f * rand() / (float)(RAND_MAX);
        float tremolosize = std::abs(baseamp - 0.5f) * rand() / (float)(RAND_MAX);
        float tremolospeed = 5.0f * pow(rand() / (float)(RAND_MAX), 3);
        for (int s = start; s < start + len; s++) {
            float t = 2*M_PI * s / sample_rate;
            float amp = baseamp + tremolosize * std::sin(t * tremolospeed);
            out[s] += amp * std::sin(t * basefreq);
        }
    }
    // Normalize to range [-1.0, 1.0]
    float absmax = 0.0f;
    for (int s = 0; s < length; s++) {
        float absval = std::abs(out[s]);
        if (absval > absmax) {
            absmax = absval;
        }
    }
    for (int s = 0; s < length; s++) {
        out[s] /= absmax;
    }
}


void test_method(std::string method) {
    std::cout << "Testing method \"" << method << "\"..." << std::endl;
    musly_jukebox* box = musly_jukebox_poweron(method.c_str(), NULL);
    musly_track* tracks[100];
    musly_trackid trackids[100];
    for (int i = 0; i < 100; i++) {
        tracks[i] = musly_track_alloc(box);
    }
    float similarities[100];
    float similarities2[100];
    int num_neighbors_guessed;
    musly_trackid candidates[20];
    musly_trackid candidates2[20];
    for (int i = 0; i < 20; i++) {
        candidates[i] = 0;
        candidates2[i] = 0;
    }

    // First, we create a jukebox
    REQUIRE( "initially empty", musly_jukebox_trackcount(box) == 0 );

    // We generate some tracks to play with
    float* song = new float[22050 * 30];
    for (int i = 0; i < 100; i++) {
        generate_music(song, 22050 * 30, 42*i);
        REQUIRE( "analyzed song", musly_track_analyze_pcm(box, song, 22050*30, tracks[i]) == 0);
    }
    delete[] song;

    // We initialize the jukebox
    REQUIRE( "set music style", musly_jukebox_setmusicstyle(box, tracks, 25) == 0 );

    // We add 50 tracks with automatically generated ids
    REQUIRE( "added tracks", musly_jukebox_addtracks(box, tracks, trackids, 50, true) == 0 );
    REQUIRE( "track count 50", musly_jukebox_trackcount(box) == 50 );
    REQUIRE( "max seen 49", musly_jukebox_maxtrackid(box) == 49 );
    for (int i = 0; i < 50; i++) {
        REQUIRE( "generated track ids", trackids[i] == i );
    }

    // We add 40 more tracks with user-defined ids
    for (int i = 50; i < 90; i++) {
        trackids[i] = 50 + i*27%367;  // rather unordered without duplicates
    }
    trackids[60] = 1000;
    REQUIRE( "added more tracks", musly_jukebox_addtracks(box, &tracks[50], &trackids[50], 40, false) == 0 );
    REQUIRE( "track count 90", musly_jukebox_trackcount(box) == 90);
    REQUIRE( "max seen 1000", musly_jukebox_maxtrackid(box) == 1000);

    // We check whether similarity and candidate computation work
    REQUIRE( "computed similarities", musly_jukebox_similarity(box, tracks[42], trackids[42], tracks, trackids, 90, similarities) == 0 );
    num_neighbors_guessed = musly_jukebox_guessneighbors(box, trackids[30], candidates, 20);
    REQUIRE( "guessed neighbors", (num_neighbors_guessed == -1) || (num_neighbors_guessed == 20) );

    // We check whether they even work deterministically (they should)
    REQUIRE( "re-computed similarities", musly_jukebox_similarity(box, tracks[42], trackids[42], tracks, trackids, 90, similarities2) == 0 );
    for (int i = 0; i < 90; i++) {
        REQUIRE( "consistent similarities", similarities[i] == similarities2[i] );
    }
    REQUIRE( "re-guessed neighbors", musly_jukebox_guessneighbors(box, trackids[30], candidates2, 20) == num_neighbors_guessed );
    if (num_neighbors_guessed > 0) {
        for (int i = 0; i < num_neighbors_guessed; i++) {
            REQUIRE( "consistent neighbor candidates", candidates[i] == candidates2[i] );
        }
    }

    // We add 10 more tracks with generated ids...
    REQUIRE( "added even more tracks", musly_jukebox_addtracks(box, &tracks[90], &trackids[90], 10, true) == 0 );
    REQUIRE( "track count 100", musly_jukebox_trackcount(box) == 100 );
    REQUIRE( "max seen 1010", musly_jukebox_maxtrackid(box) == 1010 );
    for (int i = 0; i < 10; i++) {
        REQUIRE( "generated track ids", trackids[90 + i] == 1001 + i );
    }
    // ...and do some removals and adds to restore the original jukebox state,
    // with possibly different ordering of the jukebox-internal index
    // and the trackids of the first 30 tracks increased by 1011
    REQUIRE( "removed first 30 tracks", musly_jukebox_removetracks(box, trackids, 30) == 0 );
    REQUIRE( "track count 70", musly_jukebox_trackcount(box) == 70 );
    REQUIRE( "max seen 1010", musly_jukebox_maxtrackid(box) == 1010 );
    REQUIRE( "removed last 10 tracks", musly_jukebox_removetracks(box, &trackids[90], 10) == 0 );
    REQUIRE( "track count 60", musly_jukebox_trackcount(box) == 60 );
    REQUIRE( "max seen 1010", musly_jukebox_maxtrackid(box) == 1010 );
    REQUIRE( "re-added first 30 tracks", musly_jukebox_addtracks(box, tracks, trackids, 30, true) == 0 );
    REQUIRE( "track count 90", musly_jukebox_trackcount(box) == 90 );
    REQUIRE( "max seen 1040", musly_jukebox_maxtrackid(box) == 1040 );
    for (int i = 0; i < 30; i++) {
        REQUIRE( "generated track ids", trackids[i] == 1011 + i );
    }

    // We check whether similarity and candidate computation yield the same results as before
    REQUIRE( "re-computed similarities", musly_jukebox_similarity(box, tracks[42], trackids[42], tracks, trackids, 90, similarities2) == 0 );
    for (int i = 0; i < 90; i++) {
        REQUIRE( "consistent similarities", similarities[i] == similarities2[i] );
    }
    REQUIRE( "re-guessed neighbors", musly_jukebox_guessneighbors(box, trackids[30], candidates2, 20) == num_neighbors_guessed );
    if (num_neighbors_guessed > 0) {
        // the ids of the first 30 tracks have changed; we need to adapt `candidates`
        for (int i = 0; i < num_neighbors_guessed; i++) {
            if (candidates[i] < 30) {
                candidates[i] += 1011;
            }
        }
        // candidates are not required to be returned in a particular order,
        // so if the internal jukebox index has changed, the order may change
        std::sort(candidates, candidates + num_neighbors_guessed);
        std::sort(candidates2, candidates2 + num_neighbors_guessed);
        for (int i = 0; i < num_neighbors_guessed; i++) {
            REQUIRE( "consistent neighbor candidates", candidates[i] == candidates2[i] );
        }
    }

    // We export and import the jukebox state to/from a file
    char *tmpfile = tmpnam(NULL);
    REQUIRE( "exported jukebox state", musly_jukebox_tofile(box, tmpfile) > 0 );
    musly_jukebox* box2;
    REQUIRE( "imported jukebox state", (box2 = musly_jukebox_fromfile(tmpfile)));
    remove(tmpfile);

    // We check whether the re-imported jukebox is consistent with the original one
    REQUIRE( "max seen 1040 (imported jukebox)", musly_jukebox_maxtrackid(box2) == 1040 );
    REQUIRE( "computed similarities (imported jukebox)", musly_jukebox_similarity(box2, tracks[42], trackids[42], tracks, trackids, 90, similarities2) == 0 );
    for (int i = 0; i < 90; i++) {
        REQUIRE( "consistent similarities", similarities[i] == similarities2[i] );
    }
    REQUIRE( "guessed neighbors (imported jukebox)", musly_jukebox_guessneighbors(box2, trackids[30], candidates2, 20) == num_neighbors_guessed );
    if (num_neighbors_guessed > 0) {
        std::sort(candidates2, candidates2 + num_neighbors_guessed);
        for (int i = 0; i < num_neighbors_guessed; i++) {
            REQUIRE( "consistent neighbor candidates", candidates[i] == candidates2[i] );
        }
    }

    // We check if the two jukeboxes are also consistent when adding new tracks
    // (so the music style state has been exported and imported properly)
    REQUIRE( "added 10 tracks to first jukebox", musly_jukebox_addtracks(box, &tracks[90], &trackids[90], 10, true) == 0 );
    for (int i = 0; i < 10; i++) {
        REQUIRE( "generated track ids", trackids[90 + i] == 1041 + i );
    }
    REQUIRE( "added 10 tracks to imported jukebox", musly_jukebox_addtracks(box2, &tracks[90], &trackids[90], 10, true) == 0 );
    for (int i = 0; i < 10; i++) {
        REQUIRE( "generated track ids", trackids[90 + i] == 1041 + i );
    }
    REQUIRE( "computed similarities (first jukebox)", musly_jukebox_similarity(box, tracks[10], trackids[10], tracks, trackids, 100, similarities) == 0 );
    REQUIRE( "computed similarities (imported jukebox)", musly_jukebox_similarity(box2, tracks[10], trackids[10], tracks, trackids, 100, similarities2) == 0 );
    for (int i = 0; i < 100; i++) {
        REQUIRE( "consistent similarities", similarities[i] == similarities2[i] );
    }
    num_neighbors_guessed = musly_jukebox_guessneighbors(box, trackids[23], candidates, 20);
    REQUIRE( "guessed neighbors (both jukeboxes or none)", musly_jukebox_guessneighbors(box2, trackids[23], candidates2, 20) == num_neighbors_guessed );
    if (num_neighbors_guessed > 0) {
        std::sort(candidates, candidates + num_neighbors_guessed);
        std::sort(candidates2, candidates2 + num_neighbors_guessed);
        for (int i = 0; i < num_neighbors_guessed; i++) {
            REQUIRE( "consistent neighbor candidates", candidates[i] == candidates2[i] );
        }
    }

    // Clean up whatever is left on the heap
    for (int i = 0; i < 100; i++) {
        musly_track_free(tracks[i]);
    }
    musly_jukebox_poweroff(box);
    musly_jukebox_poweroff(box2);
}


int main() {
    MiniLog::current_level() = logERROR;

    // Unit tests
    std::cout << "Components to test: unordered_idpool,ordered_idpool" << std::endl;
    test_unordered_idpool();
    test_ordered_idpool();
    std::cout << std::endl;

    // Tests of the full library
    std::cout << "Methods to test: " << musly_jukebox_listmethods() << std::endl;
    std::vector<std::string> methods = split(musly_jukebox_listmethods(), ',');
    for (int i = 0; i < (int)methods.size(); i++) {
        test_method(methods[i]);
    }

    SUMMARY();
}
