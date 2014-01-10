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

#ifndef MUSLY_WINDOWFUNCTION_H_
#define MUSLY_WINDOWFUNCTION_H_

#include <Eigen/Core>

namespace musly {

class windowfunction {
public:

    /** Return a Hann window of the given size.
     * \window_size The size of the Hann window.
     * \returns The hann window weights vector.
     */
    static Eigen::VectorXf hann(
            int window_size);
};

} /* namespace musly */
#endif /* MUSLY_WINDOWFUNCTION_H_ */
