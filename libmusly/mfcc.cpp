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
#include "mfcc.h"

namespace musly {

mfcc::mfcc(int mel_bins, int mfcc_bins) :
        dct(mel_bins, mfcc_bins)
{
}

Eigen::MatrixXf mfcc::from_melspectrum(const Eigen::MatrixXf& mel)
{
    MINILOG(logTRACE) << "Computing MFCCs.";

    Eigen::MatrixXf mfcc_coeffs = dct.compress((1.0f + mel.array()).log());
    MINILOG(logTRACE) << "MFCCS: " << mfcc_coeffs;

    MINILOG(logTRACE) << "Finished Computing MFCCs.";
    return mfcc_coeffs;
}

} /* namespace musly */
