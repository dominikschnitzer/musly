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
    append_normfacts(
            int count);

    void
    set_normfacts(
            int position,
            Eigen::VectorXf& sim);

    void
    set_normfacts(
            int position,
            float mu,
            float std);

    void
    get_normfacts(
            int position,
            float* mu,
            float* std);

    void
    swap_normfacts(
            int position1,
            int position2);

    void
    trim_normfacts(
            int count);

    int
    normalize(
            int seed_position,
            int* other_positions,
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
