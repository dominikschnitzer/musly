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

#ifndef MUSLY_POWERSPECTRUM_H_
#define MUSLY_POWERSPECTRUM_H_

#include <Eigen/Core>
extern "C" {
    #include <kiss_fftr.h>
}


namespace musly {

class powerspectrum {
private:
    /** The hop size in samples.
     */
    int hop_size;

    /** The window size to analyze in samples.
     */
    int win_size;

    /** The window function, as a vector of scalars. The window function is
     * multiplied with the signal before applying the FFT.
     */
    Eigen::VectorXf win_funct;

    /** KissFFT internal representation of the PCM data.
     */
    kiss_fft_scalar* kiss_pcm;

    /** KissFFT internal of the frequency spectrum.
     */
    kiss_fft_cpx* kiss_freq;

    /** KissFFT internal of the FFT status.
     */
    kiss_fftr_cfg kiss_status;

public:
    /** Initialize the powerspectrum with the window function and hop size.
     * \param win_funct The window function as a vector of scalars. It is
     * multipled with the signal before the FFT. The length of the vector
     * also specifies the window size.
     * \param hop the length of a hop when analyzing a pcm signal relative to
     * the size of the window, i.e., a value >0 and <=1 is reasonable.
     */
    powerspectrum(
            const Eigen::VectorXf& win_funct,
            float hop);

    /** Get the powerspectum from the given PCM samples. The spectrum is
     * computed with the parameters from the initialization.
     * \param pcm_samples A vector of PCM samles.
     * \returns The powerspectrum computed from the given PCM samples as
     * matrix. The matrix has the dimension (Frequency, Time) and is in
     * column major format.
     */
    Eigen::MatrixXf from_pcm(
            const Eigen::VectorXf& pcm_samples);

    /** Cleanup. Frees the internal KissFFT variables.
     */
    virtual ~powerspectrum();
};

} /* namespace musly */
#endif /* MUSLY_POWERSPECTRUM_H_ */
