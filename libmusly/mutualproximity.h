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

#ifndef MUSLY_MUTUALPROXIMITY_H_
#define MUSLY_MUTUALPROXIMITY_H_

#include <vector>
#include "musly/musly_types.h"
#include "method.h"

namespace musly {

class mutualproximity {
public:
    mutualproximity(method* m);
    virtual ~mutualproximity();

    virtual int
    set_normtracks(
            musly_track** tracks,
            int length);

    std::vector<musly_track*>*
    get_normtracks();

    void
    set_normfacts(
            musly_trackid trackid,
            Eigen::VectorXf& sim);

    int
    normalize(
            musly_trackid seed_trackid,
            musly_trackid* trackids,
            int length,
            float* sim);

private:
    method* m;
    std::vector<musly_track*> norm_tracks;
    struct normfact {
        float mu;
        float std;
    };
    std::vector<normfact> norm_facts;


    void
    new_cache(
            int size);

    double
    normcdf(
            double x);

};

} /* namespace musly */
#endif /* MUSLY_MUTUALPROXIMITY_H_ */
