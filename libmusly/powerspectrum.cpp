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
#include "powerspectrum.h"


namespace musly {


powerspectrum::powerspectrum(
        const Eigen::VectorXf& win_funct,
        float hop)
{
    this->win_funct = win_funct;
    this->win_size = static_cast<int>(win_funct.size());
    this->hop_size = static_cast<int>(hop*win_size);

    // initialize kiss fft
    kiss_pcm = (kiss_fft_scalar*)malloc(sizeof(kiss_fft_scalar) * win_size);
    kiss_freq = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (win_size/2 + 1));
    kiss_status = kiss_fftr_alloc(win_size, 0, NULL, NULL);
}

Eigen::MatrixXf
powerspectrum::from_pcm(const Eigen::VectorXf& pcm_samples)
{
    MINILOG(logTRACE) << "Powerspectrum computation. input samples="
            << pcm_samples.size();
    // check if inputs are sane
    if ((pcm_samples.size() < win_size) || (hop_size > win_size)) {
        return Eigen::MatrixXf(0, 0);
    }
    size_t frames = (pcm_samples.size() - (win_size-hop_size)) / hop_size;
    size_t freq_bins = win_size/2 + 1;

    // initialize power spectrum
    Eigen::MatrixXf ps(freq_bins, frames);

    // peak normalization value
    float pcm_scale = std::max(fabs(pcm_samples.minCoeff()),
            fabs(pcm_samples.maxCoeff()));

    // scale signal to 96db (16bit)
    pcm_scale =  std::pow(10.0f, 96.0f/20.0f) / pcm_scale;

    // compute the power spectrum
    for (size_t i = 0; i < frames; i++) {

        // fill pcm
        for (int j = 0; j < win_size; j++) {
            kiss_pcm[j] = pcm_samples(i*hop_size+j) * pcm_scale * win_funct(j);
        }

        // fft
        kiss_fftr(kiss_status, kiss_pcm, kiss_freq);

        // save powerspectrum frame
        Eigen::MatrixXf::ColXpr psc(ps.col(i));
        for (int j = 0; j < win_size/2+1; j++) {
            psc(j) =
                    std::pow(kiss_freq[j].r, 2) + std::pow(kiss_freq[j].i, 2);
        }
    }

    MINILOG(logTRACE) << "Powerspectrum finished. size=" << ps.rows() << "x"
            << ps.cols();
    return ps;
}

powerspectrum::~powerspectrum()
{
    free(kiss_status);
    free(kiss_pcm);
    free(kiss_freq);
}


} /* namespace musly */
