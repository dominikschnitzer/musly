/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MUSLY_DECODERS_MEDIAFOUNDATION_H_
#define MUSLY_DECODERS_MEDIAFOUNDATION_H_


#include "decoder.h"


namespace musly { namespace decoders {

class mediafoundation : public musly::decoder
{
    MUSLY_DECODER_REGCLASS(mediafoundation);

public:
    mediafoundation();

    virtual ~mediafoundation();

    virtual std::vector<float>
    decodeto_22050hz_mono_float(const std::string& file, float excerpt_length, float excerpt_start);
};

}} // namespace musly::decoders


#endif // !MUSLY_DECODERS_MEDIAFOUNDATION_H_
