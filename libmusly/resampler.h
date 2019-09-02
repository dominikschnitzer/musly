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

#ifndef MUSLY_RESAMPLER_H_
#define MUSLY_RESAMPLER_H_

#include <vector>
extern "C" {
    #include "libresample/libresample.h"
}

namespace musly {

class resampler {
private:
    double resample_factor;

    void* libresample;

public:
    resampler(int input_rate, int output_rate);
    virtual ~resampler();

    std::vector<float> resample(float* pcm_input, int pcm_len);
};

} /* namespace musly */
#endif /* MUSLY_RESAMPLER_H_ */
