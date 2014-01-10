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

#ifndef MUSLY_DECODER_H_
#define MUSLY_DECODER_H_

#include <string>
#include <vector>
#include "plugins.h"

namespace musly {

class decoder :
        public plugin
{
public:
    decoder();
    virtual ~decoder();

    virtual std::vector<float>
    decodeto_22050hz_mono_float(
            const std::string& file,
            int max_seconds) = 0;

};

#define MUSLY_DECODER_REGCLASS(classname) \
private: \
    static const plugin_creator_impl<classname> creator;

#define MUSLY_DECODER_REGIMPL(classname, priority) \
    const plugin_creator_impl<classname> classname::creator(#classname, \
            plugins::DECODER_TYPE, priority);

} /* namespace musly */
#endif /* MUSLY_DECODER_H_ */
