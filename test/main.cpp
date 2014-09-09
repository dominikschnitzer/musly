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
    float song[22050 * 30];
    musly_track* tracks[100];
    musly_trackid trackids[100];
    for (int i = 0; i < 100; i++) {
        tracks[i] = musly_track_alloc(box);
    }

    REQUIRE( "initially empty", musly_jukebox_trackcount(box) == 0 );

    for (int i = 0; i < 100; i++) {
        generate_music(song, 22050 * 30, 42*i);
        REQUIRE( "analyzed song", musly_track_analyze_pcm(box, song, 22050*30, tracks[i]) == 0);
    }

    REQUIRE( "set music style", musly_jukebox_setmusicstyle(box, tracks, 25) == 0 );

    REQUIRE( "added tracks", musly_jukebox_addtracks(box, tracks, trackids, 50, true) == 0 );
    REQUIRE( "track count 50", musly_jukebox_trackcount(box) == 50 );
    REQUIRE( "max seen 49", musly_jukebox_maxtrackid(box) == 49 );
    for (int i = 0; i < 50; i++) {
        REQUIRE( "generated track ids", trackids[i] == i );
    }

    for (int i = 50; i < 90; i++) {
        trackids[i] = 50 + i*27%367;  // rather unordered without duplicates
    }
    trackids[60] = 1000;
    REQUIRE( "added more tracks", musly_jukebox_addtracks(box, &tracks[50], &trackids[50], 40, false) == 0 );
    REQUIRE( "track count 90", musly_jukebox_trackcount(box) == 90);
    REQUIRE( "max seen 1000", musly_jukebox_maxtrackid(box) == 1000);

    float similarities[90];
    REQUIRE( "computed similarities", musly_jukebox_similarity(box, tracks[42], trackids[42], tracks, trackids, 90, similarities) == 0 );

    REQUIRE( "added even more tracks", musly_jukebox_addtracks(box, &tracks[90], &trackids[90], 10, true) == 0 );
    REQUIRE( "track count 100", musly_jukebox_trackcount(box) == 100 );
    REQUIRE( "max seen 1010", musly_jukebox_maxtrackid(box) == 1010 );
    for (int i = 0; i < 10; i++) {
        REQUIRE( "generated track ids", trackids[90 + i] == 1001 + i );
    }

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

    float similarities2[90];
    REQUIRE( "re-computed similarities", musly_jukebox_similarity(box, tracks[42], trackids[42], tracks, trackids, 90, similarities2) == 0 );
    for (int i = 0; i < 90; i++) {
        REQUIRE( "consistent similarities", similarities[i] == similarities2[i] );
    }

    for (int i = 0; i < 100; i++) {
        musly_track_free(tracks[i]);
    }
    musly_jukebox_poweroff(box);
}


int main() {
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
