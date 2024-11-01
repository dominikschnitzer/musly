/**
 * Copyright 2024, Andreas Bannach <andreas@borntohula.de>
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "minilog.h"
#include "none.h"

namespace musly { namespace decoders {

MUSLY_DECODER_REGIMPL(none, 0);

none::none()
{
}

std::vector<float>
none::decodeto_22050hz_mono_float(const std::string&, float, float)
{
    MINILOG(logWARNING) << "attempting to decode audio file using 'none' decoder. use musly_jukebox_analyze_pcm() directly.";
    return std::vector<float>(0);
}

}} // namespace musly::decoders
