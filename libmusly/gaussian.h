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

#ifndef MUSLY_GAUSSIAN_H_
#define MUSLY_GAUSSIAN_H_

/** A structure modeling a multivariate Gaussian.
 *
 */
struct gaussian {
    float* mu;
    float* covar;
    float* covar_inverse;
    float* covar_logdet;
};


#endif /* MUSLY_GAUSSIAN_H_ */
