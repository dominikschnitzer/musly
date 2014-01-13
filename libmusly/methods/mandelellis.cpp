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

#include <Eigen/Core>

#include "minilog.h"
#include "windowfunction.h"
#include "mandelellis.h"


namespace musly {
namespace methods {

/** Register mandelellis with musly with a low priority (0)
 */
MUSLY_METHOD_REGIMPL(mandelellis, 0);

mandelellis::mandelellis() :

        // initialize method configuration parameters
        sample_rate(22050),
        window_size(1024),
        hop(0.5f),
        max_pcmlength(60*sample_rate),
        ps_bins(window_size/2+1),
        mel_bins(36),
        mfcc_bins(20),

        // spectra and filters
        ps(windowfunction::hann(window_size), hop),
        mel(ps_bins, mel_bins, sample_rate),
        mfccs(mel_bins, mfcc_bins),
        gs(mfcc_bins)
{
    // Configure the musly_track features and save the musly_track offsets

    // the feature mean
    track_mu = track_addfield_floats("gaussian.mu", gs.get_dim());
    // add the covariance (symmetric matrix)
    track_covar = track_addfield_floats("gaussian.covar", gs.get_covarelems());
    // add the covariance (symmetric matrix)
    track_covar_inverse = track_addfield_floats("gaussian.covar_inverse",
            gs.get_covarelems());
}

mandelellis::~mandelellis()
{
}

const char*
mandelellis::about()
{
    return
        "The most basic timbre music similarity measure published by:\n"
        "M. Mandel and D. Ellis in: Song-level features and support vector\n"
        "machines for music classification. In the proceedings of the 6th\n"
        "International Conference on Music Information Retrieval,\n"
        "ISMIR, 2005.\n"
        "MUSLY computes a single Gaussian representation from the songs.\n"
        "The similarity between two tracks represented as Gaussians\n"
        "is computed with the symmetrized Kullback-Leibler divergence";
}

int
mandelellis::analyze_track(
        float* pcm,
        int length,
        musly_track* track)
{
    MINILOG(logTRACE) << "ME analysis started. samples=" << length;

    // select the central max_pcmlength (usually 30s) of the piece
    int start = 0;
    if (length > max_pcmlength) {
        start = (length - max_pcmlength) / 2;
        length = max_pcmlength;
    }

    // PCM --> powerspectrum
    Eigen::Map<Eigen::VectorXf> pcm_vector(pcm+start, length);
    Eigen::MatrixXf power_spectrum = ps.from_pcm(pcm_vector);

    // powerspectrum -> Mel
    Eigen::MatrixXf mel_spectrum = mel.from_powerspectrum(power_spectrum);

    // Mel -> MFCC representation
    Eigen::MatrixXf mfcc_representation =
            mfccs.from_melspectrum(mel_spectrum);

    // estimate the Gaussian from the MFCC representation
    gaussian g = {0, 0, 0, 0};
    g.mu = &track[track_mu];
    g.covar = &track[track_covar];
    g.covar_inverse = &track[track_covar_inverse];
    if (gs.estimate_gaussian(mfcc_representation, g) == false) {
        MINILOG(logTRACE) << "ME Gaussian model estimation failed.";
        return 2;
    }

    MINILOG(logTRACE) << "ME analysis finished!";

    return 0;
}


int
mandelellis::similarity(
        musly_track* track,
        musly_trackid seed_trackid,
        musly_track** tracks,
        musly_trackid* trackids,
        int length,
        float* similarities)
{
    if ((length <= 0) || !track || ! tracks || !similarities) {
        return -1;
    }

    // map seed track to gaussian structure
    gaussian g0;
    g0.mu = &track[track_mu];
    g0.covar = &track[track_covar];
    g0.covar_inverse = &track[track_covar_inverse];

    // create the temporary buffer required for the Kullback-Leibler divergence
    musly_track* tmp_t = track_alloc();
    gaussian tmp;
    tmp.mu = &tmp_t[track_mu];
    tmp.covar = &tmp_t[track_covar];
    tmp.covar_inverse = &tmp_t[track_covar_inverse];

    // iterate over all musly_tracks to compute the Kullback-Leibler divergence
    for (int i = 0; i < length; i++) {
        gaussian gi;
        musly_track* track1 = tracks[i];
        gi.mu = &track1[track_mu];
        gi.covar = &track1[track_covar];
        gi.covar_inverse = &track1[track_covar_inverse];

        similarities[i] = gs.symmetric_kullbackleibler(g0, gi, tmp);
    }

    delete[] tmp_t;

    return 0;
}


} /* namespace methods */
} /* namespace musly */
