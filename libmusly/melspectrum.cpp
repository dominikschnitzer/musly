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

#include "minilog.h"
#include "melspectrum.h"

namespace musly {


melspectrum::melspectrum(
        int powerspectrum_bins,
        int mel_bins,
        int sample_rate) :
                filterbank(mel_bins, powerspectrum_bins)
{
    // our mel filters start at a minimum frequency of 20hz
    float min_freq = 20;

    // determine the frequency of each powerspectrum bin
    Eigen::VectorXf ps_freq = Eigen::VectorXf::LinSpaced(powerspectrum_bins,
            0.0f, sample_rate/2.0f);

    // determine the best mel bin for each frequency
    Eigen::VectorXf freq = Eigen::VectorXf::LinSpaced(sample_rate/2-static_cast<int>(min_freq),
            min_freq, sample_rate/2.f);
    Eigen::VectorXf mel = ((freq/700.0f).array() + 1.0f).log() * 1127.01048f;
    Eigen::VectorXf mel_idx = Eigen::VectorXf::LinSpaced(mel_bins+2,
            1.0f, mel.maxCoeff());

    // construct the triangular filters
    Eigen::VectorXf left(mel_bins);
    Eigen::VectorXf center(mel_bins);
    Eigen::VectorXf right(mel_bins);
    Eigen::VectorXf::Index min_idx;
    for (int i = 0; i < mel_bins; i++) {
        ((mel.array() - mel_idx(i)).abs()).minCoeff(&min_idx);
        left(i) = freq(min_idx);

        ((mel.array() - mel_idx(i+1)).abs()).minCoeff(&min_idx);
        center(i) = freq(min_idx);

        ((mel.array() - mel_idx(i+2)).abs()).minCoeff(&min_idx);
        right(i) = freq(min_idx);
    }
    Eigen::VectorXf heights = 2.0f / (right.array() - left.array());

    // construct filterbank
    for (int i = 0; i < mel_bins; i++) {
        for (int j = 0; j < powerspectrum_bins; j++) {
            if ((ps_freq(j) > left(i)) && (ps_freq(j) <= center(i))) {
                float weight = heights(i) *
                        ((ps_freq(j) - left(i)) / (center(i) - left(i)));
                filterbank.insert(i, j) = weight;
            }

            if ((ps_freq(j) > center(i)) && (ps_freq(j) < right(i))) {
                float weight = heights(i) *
                        ((right(i) - ps_freq(j)) / (right(i) - center(i)));
                filterbank.insert(i, j) = weight;
            }
        }
    }

    MINILOG(logTRACE) << "Mel filterbank: " << filterbank;
}

melspectrum::~melspectrum()
{
}

Eigen::MatrixXf
melspectrum::from_powerspectrum(const Eigen::MatrixXf& ps)
{
    MINILOG(logTRACE) << "Mel filtering specturm. size=" << ps.rows()
            << "x" << ps.cols();

    // iterate over each frame and apply the triangular mel filters
    Eigen::MatrixXf mels(filterbank.rows(), ps.cols());
    for (int i = 0; i < ps.cols(); i++) {
        mels.col(i) = filterbank * ps.col(i);
    }

    MINILOG(logTRACE) << "Mel specturm computed. size=" << mels.rows()
            << "x" << mels.cols();
    return mels;
}


} /* namespace musly */
