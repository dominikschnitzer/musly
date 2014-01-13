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

#include <Eigen/Core>

#include "musly/musly_types.h"
#include "mutualproximity.h"

namespace musly {

mutualproximity::mutualproximity(method* m) :
        m(m)
{
}

mutualproximity::~mutualproximity()
{
    // clear cache
    new_cache(0);
}

void
mutualproximity::new_cache(int size)
{
    for (int i = 0; i < (int)norm_tracks.size(); i++) {
        delete[] norm_tracks[i];
    }

    norm_tracks.clear();
    for (int i = 0; i < size; i++) {
        norm_tracks.push_back(m->track_alloc());
    }
}

int
mutualproximity::set_normtracks(
        musly_track** tracks,
        int length)
{
    new_cache(length);

    int track_size = m->track_getsize();
    for (int i = 0; i < length; i++) {
        std::copy(tracks[i], tracks[i]+track_size, norm_tracks[i]);
    }

    return 0;
}

std::vector<musly_track*>*
mutualproximity::get_normtracks()
{
    return &norm_tracks;
}

void
mutualproximity::set_normfacts(
        musly_trackid trackid,
        Eigen::VectorXf& sim)
{
    double mu = sim.mean();
    Eigen::VectorXd sim_mu = sim.cast<double>().array() - mu;
    double std = (sim_mu.transpose() * sim_mu);
    std /= (static_cast<double>(sim.size()) - 1.0);

    // allocate space
    if (trackid >= (int)norm_facts.size()) {
        norm_facts.resize(trackid+1);
        norm_facts[trackid].mu = mu;
        norm_facts[trackid].std = sqrt(std);
    }
}

double
mutualproximity::normcdf(double x)
{
    // constants
    const double a1 =  0.254829592;
    const double a2 = -0.284496736;
    const double a3 =  1.421413741;
    const double a4 = -1.453152027;
    const double a5 =  1.061405429;
    const double p  =  0.3275911;

    // Save the sign of x
    int sign = 1;
    if (x < 0) {
        sign = -1;
    }
    x = fabs(x)/M_SQRT2;

    // A&S formula 7.1.26
    double t = 1.0/(1.0 + p*x);
    double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

    return 0.5*(1.0 + sign*y);
}

int
mutualproximity::normalize(
        musly_trackid seed_trackid,
        musly_trackid* trackids,
        int length,
        float* sim)
{
    if (seed_trackid >= (int)norm_facts.size()) {
        return -1;
    }
    float seed_mu = norm_facts[seed_trackid].mu;
    float seed_std = norm_facts[seed_trackid].std;
    for (int i = 0; i < length; i++) {
        int tid = trackids[i];
        if (tid >= (int)norm_facts.size()) {
            return -1;
        }
        if (tid == seed_trackid) {
            sim[i] = 0;
            continue;
        }

        float d = sim[i];
        if (isnan(d)) {
           continue;
        }

        double p1 = 1 - normcdf((d - seed_mu)/seed_std);
        double p2 = 1 - normcdf((d - norm_facts[tid].mu)/norm_facts[tid].std);
        sim[i] = 1 - p1*p2;
    }
    return 0;
}



} /* namespace musly */
