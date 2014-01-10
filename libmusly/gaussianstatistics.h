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

#ifndef MUSLY_GAUSSIANSTATISTICS_H_
#define MUSLY_GAUSSIANSTATISTICS_H_

#include <Eigen/Core>
#include "gaussian.h"


namespace musly {

/** A class to compute
 *
 */
class gaussian_statistics {
private:
    int d;

    int covar_elems;

public:
    /** A musly Gaussian representation.
     *
     */
    gaussian_statistics(
            int gaussian_dim);

    int get_covarelems();

    int get_dim();

    bool
    estimate_gaussian(
            const Eigen::MatrixXf& m,
            gaussian& g);

    float
    jensenshannon(
            const gaussian &g0,
            const gaussian &g1,
            gaussian &tmp);

    float
    symmetric_kullbackleibler(
            const gaussian& g0,
            const gaussian& g1,
            gaussian& tmp);
};

} /* namespace musly */
#endif /* MUSLY_GAUSSIANSTATISTICS_H_ */
