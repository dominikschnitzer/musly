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

#include <iostream>
#include <cmath>

#include "windowfunction.h"

namespace musly {

Eigen::VectorXf windowfunction::hann(
        int window_size)
{
    float N = window_size - 1.f;
    Eigen::VectorXf n = Eigen::VectorXf::LinSpaced(window_size, 0.f, N);
    Eigen::VectorXf w = 0.5f * (1.0f - (2.0f*M_PI*n/N).array().cos());
    return w;
}

} /* namespace musly */
