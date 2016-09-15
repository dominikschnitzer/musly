/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *                2014, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MUSLY_METHODS_MANDELELLIS_H_
#define MUSLY_METHODS_MANDELELLIS_H_

#include "method.h"
#include "powerspectrum.h"
#include "melspectrum.h"
#include "mfcc.h"
#include "gaussianstatistics.h"
#include "idpool.h"

namespace musly {
namespace methods {

class mandelellis :
        public musly::method

{
MUSLY_METHOD_REGCLASS(mandelellis);

private:

    const int sample_rate;
    const int window_size;
    const float hop;
    const int max_pcmlength;
    const int ps_bins;
    const int mel_bins;
    const int mfcc_bins;

    int track_mu;
    int track_covar;
    int track_covar_inverse;

    powerspectrum ps;
    melspectrum mel;
    mfcc mfccs;
    gaussian_statistics gs;
    unordered_idpool<musly_trackid> idpool;

    void
    similarity_raw(
                musly_track* track,
                musly_track** tracks,
                int length,
                float* similarities);

public:
    mandelellis();

    virtual
    ~mandelellis();

    virtual const char*
    about();

    virtual int
    analyze_track(
            float* pcm,
            int length,
            musly_track* track);

    virtual int
    similarity(
            musly_track* track,
            musly_trackid seed_trackid,
            musly_track** tracks,
            musly_trackid* trackids,
            int length,
            float* similarities);

    virtual int
    add_tracks(
            musly_track** tracks,
            musly_trackid* trackids,
            int length,
            bool generate_ids);

    virtual void
    remove_tracks(
            musly_trackid* trackids,
            int length);

    virtual int
    get_trackcount();

    virtual int
    get_maxtrackid();

    virtual int
    get_trackids(
            musly_trackid* trackids);

    virtual int
    serialize_metadata(
            unsigned char* buffer);

    virtual int
    deserialize_metadata(
            unsigned char* buffer);

    virtual int
    serialize_trackdata(
            unsigned char* buffer,
            int num_tracks,
            int skip_tracks = 0);

    virtual int
    deserialize_trackdata(
            unsigned char* buffer,
            int num_tracks);

};

} /* namespace methods */
} /* namespace musly */
#endif /* MUSLY_METHODS_MANDELELLIS_H_ */
