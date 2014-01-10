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

#include <cmath>

#include "minilog.h"
#include "discretecosinetransform.h"

namespace musly {


discretecosinetransform::discretecosinetransform(int in_bins, int out_bins) :
        m(in_bins, out_bins)
{
    // DCT-II
    Eigen::VectorXf coeff1 =
            Eigen::ArrayXf::LinSpaced(out_bins, 0, out_bins-1.0f);
    Eigen::VectorXf coeff2 =
            (2.0f*Eigen::ArrayXf::LinSpaced(in_bins, 0, in_bins-1.0f) + 1.0f);
    m = 1.0f/std::sqrt(in_bins/2.0f) *
            ((coeff1 * coeff2.transpose()) * M_PI_2/in_bins).array().cos();

    // special scaling for first row
    m.row(0) = m.row(0) * std::sqrt(2.0f)/2.0f;

    MINILOG(logTRACE) << "DCT-II filterbank: " << m;
}

Eigen::MatrixXf discretecosinetransform::compress(const Eigen::MatrixXf& in)
{
    MINILOG(logTRACE) << "Computing DCT, input=" << in.rows()
            << "x" << in.cols();

    Eigen::MatrixXf out = (in.transpose() * m.transpose()).transpose();

    MINILOG(logTRACE) << "Finished DCT, output=" << out.rows()
            << "x" << out.cols();
    return out;
}


} /* namespace musly */
