/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *           2014-2016, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MUSLY_DECODERS_LIBAV_H_
#define MUSLY_DECODERS_LIBAV_H_

extern "C" {
    #include <libavcodec/avcodec.h>
}

#include "decoder.h"

namespace musly {
namespace decoders {

class libav :
    public musly::decoder
{
    MUSLY_DECODER_REGCLASS(libav);

private:
    int
    samples_tofloat(
            void* const out,
            const void* const in,
            const int out_stride,
            const int in_stride,
            const AVSampleFormat in_fmt,
            int len);

public:
    libav();

    virtual std::vector<float>
    decodeto_22050hz_mono_float(
            const std::string& file,
            float excerpt_length,
            float excerpt_start);
};

} /* namespace decoders */
} /* namespace musly */
#endif /* MUSLY_DECODERS_LIBAV_H_ */
