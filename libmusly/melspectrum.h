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

#ifndef MUSLY_MELSPECTRUM_H_
#define MUSLY_MELSPECTRUM_H_

#include <vector>
#include <Eigen/Core>
#include <Eigen/Sparse>

namespace musly {

class melspectrum {
private:
    /** Stores the triangular filterbank for each mel bin.
     */
    Eigen::SparseMatrix<float> filterbank;

public:
    /** Initializes the Mel filterbanks. The Mel filterbanks are computed using
     * the sample rate and number of bins in the powerspectum.
     * \param powerspectrum_bins The number of input powerspectrum bins.
     * \param mel_bins The number of mel filters or bins to compute from the
     * powerspectrum.
     * \param sample_rate The original sample rate of the PCM signal.
     */
    melspectrum(
            int powerspectrum_bins,
            int mel_bins,
            int sample_rate);

    /** Currently empty destructor
     */
    virtual ~melspectrum();

    /** Computes the Mel spectrum from the given powerspectrum. The triangular
     * filterbank is precomputed in the constructor.
     * \param ps The powerspectrum as a matrix (frequency, time).
     * \returns The Mel spectrum as a matrix (frequency, time). Column major.
     */
    Eigen::MatrixXf from_powerspectrum(
            const Eigen::MatrixXf& ps);
};

} /* namespace musly */
#endif /* MUSLY_MELSPECTRUM_H_ */
