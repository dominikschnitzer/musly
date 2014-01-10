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

#ifndef MUSLY_DISCRETECOSINETRANSFORM_H_
#define MUSLY_DISCRETECOSINETRANSFORM_H_

#include <Eigen/Core>

namespace musly {

/** Class implementing a DCT-II (Discrete Cosine Transform). It is implemented
 * as a rather slow matrix multiplication. A faster variant would use DFTs.
 */
class discretecosinetransform {
private:
    Eigen::MatrixXf m;

public:
    /** Construct a Discrete Cosine Transform (DCT-II) compression matrix.
     * \param in_bins The numnber of input bins, i.e. the input DCT dimension.
     * \param out_bins The number of bins to use after the DCT.
     */
    discretecosinetransform(int in_bins, int out_bins);

    /** Compress the given vectors using a DCT-II.
     * \param in The input matrix, each vector is compressed using a DCT. The
     * matrix has to have in_bins rows.
     * \returns The compressed matrix with out_bins rows.
     */
    Eigen::MatrixXf compress(const Eigen::MatrixXf& in);
};

} /* namespace musly */
#endif /* MUSLY_DISCRETECOSINETRANSFORM_H_ */
