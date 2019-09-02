/**
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *                2014, Jan Schlueter <jan.schlueter@ofai.at>
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
            float excerpt_length,
            float excerpt_start) = 0;

};

/** A macro to facilitating registering a decoder class with musly. This macro
 * has to be used in the header and class declaration. Call it with the class
 * name as parameter.
 */
#ifndef BUILD_STATIC
#define MUSLY_DECODER_REGCLASS(classname) \
private: \
    static const plugin_creator_impl<classname> creator
#else
#define MUSLY_DECODER_REGCLASS(classname)
#endif

/** A macro to facilitate registering a decoder class with musly. This macro
 * has to be used in the source file, it has two parameters. Call it with the
 * name of your class and the priority. The priority value is used if the user
 * does not request a special musly::decoder when calling \sa
 * musly_jukebox_poweron(). The method with the highest priority value is used.
 */
#ifndef BUILD_STATIC
#define MUSLY_DECODER_REGIMPL(classname, priority) \
    const plugin_creator_impl<classname> classname::creator(#classname, \
            plugins::DECODER_TYPE, priority)
#else
#define MUSLY_DECODER_REGIMPL(classname, priority)
#endif

/** Alternative form for MUSLY_DECODER_REGIMPL to be used for static builds of
 * the library. This form has to be placed in global scope in lib.cpp to make
 * the plugin available in static builds.
 */
#ifdef BUILD_STATIC
#define MUSLY_DECODER_REGSTATIC(classname, priority) \
    static const musly::plugin_creator_impl<musly::decoders::classname> \
            create ## _ ## classname (#classname, \
            musly::plugins::DECODER_TYPE, priority)
#endif

} /* namespace musly */
#endif /* MUSLY_DECODER_H_ */
