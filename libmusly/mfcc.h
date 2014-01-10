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

#ifndef MUSLY_MFCC_H_
#define MUSLY_MFCC_H_

#include <Eigen/Core>
#include "discretecosinetransform.h"

namespace musly {

/** Compute the Mel Frequency Cepstrum Coefficients (MFCC) from a given
 * MEL spectrum.
 */
class mfcc {
private:
    /** A DCT-II filter for the compression of the spectrum.
     */
    discretecosinetransform dct;

public:
    /** Preinitialize the MFCC class by specifying the number of input MEL
     * bins and output MFCC bins.
     * \param mel_bins The input MEL bins.
     * \param mfcc_bins The number of MFCC bins to compute from the mel_bins.
     */
    mfcc(int mel_bins, int mfcc_bins);

    /** Compute the MFCCs from a given MEL spectrum.
     * \param mel The MEL spectrum computed with melspecturm
     * \returns The DCT compressed MFCC representation of the input specturm.
     */
    Eigen::MatrixXf from_melspectrum(const Eigen::MatrixXf& mel);
};

} /* namespace musly */
#endif /* MUSLY_MFCC_H_ */
