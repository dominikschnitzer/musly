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
#include <limits>
#include <algorithm>
#include <Eigen/Core>
#include <Eigen/QR>
#include "minilog.h"
#include "gaussianstatistics.h"


namespace musly {

gaussian_statistics::gaussian_statistics(
        int gaussian_dim) :
                d(gaussian_dim),
                covar_elems((d*(d+1)/2))
{
}

int
gaussian_statistics::get_covarelems()
{
    return covar_elems;
}

int
gaussian_statistics::get_dim()
{
    return d;
}


bool
gaussian_statistics::estimate_gaussian(
        const Eigen::MatrixXf& m,
        gaussian& g)
{
    MINILOG(logTRACE) << "Estimating Gaussian from matrix: " << m.rows()
            << "x" << m.cols();

    if (m.cols() <= d) {
        MINILOG(logTRACE) << "could not estimate Gaussian. "
                << "Too few input samples. m.cols=" << m.cols();
        return false;
    }

    if (m.rows() != d) {
        MINILOG(logTRACE) << "could not estimate Gaussian. "
                << "Wrong dimension (d=" << d << " vs. m.rows="
                << m.rows() << ")";
        return false;
    }

    // always compute sample mean
    Eigen::VectorXf mu = m.rowwise().mean();
    if (g.mu) {
        for (int i = 0; i < d; i++) {
            g.mu[i] = mu(i);
        }
    }

    // always compute sample covariance
    Eigen::MatrixXf covar  = (m.colwise()-mu) * (m.colwise()-mu).transpose()
            / (static_cast<float>(m.cols()) - 1.0f);

    // Add Gaussian noise to the data to avoid singular covariance matrices
    // in case the input data was silence.
    covar.diagonal().array() += 1e-4f;
    if (g.covar) {
        int idx_ij = 0;
        for (int i = 0; i < d; i++) {
            for (int j = i; j < d; j++) {
                g.covar[idx_ij] = covar(i, j);
                idx_ij++;
            }
        }
    }

    // Check if we need to set logdet or inversecovar fields of the Gaussian
    if (g.covar_inverse || g.covar_logdet) {
        Eigen::FullPivHouseholderQR<Eigen::MatrixXf> qr =
                covar.fullPivHouseholderQr();
/*        if (!qr.isInvertible()) {
            MINILOG(logDEBUG1) << "Could not compute inverse Gaussian "
                    << "covariance matrix";
            return false;
        }
*/
        if (g.covar_inverse) {
            Eigen::MatrixXf covar_inverse = qr.inverse();
            int idx_ij = 0;
            for (int i = 0; i < d; i++) {
                for (int j = i; j < d; j++) {
                    g.covar_inverse[idx_ij] = covar_inverse(i, j);
                    idx_ij++;
                }
            }
        }

        if (g.covar_logdet) {
            *(g.covar_logdet) = qr.logAbsDeterminant();
        }
    }


    return true;
}


float
gaussian_statistics::jensenshannon(
        const gaussian& g0,
        const gaussian& g1,
        gaussian& tmp)
{
    // return 0 if the models to compare are the same
    if ((g0.covar == g1.covar) && (g0.mu == g1.mu)) {
        return 0;
    }
    float jsd = -0.25f * (*(g0.covar_logdet) + *(g1.covar_logdet));

    // merge the mean and covariance matrices to get the merged Gaussian
    for (int i = 0; i < d; i++) {
        tmp.mu[i] = static_cast<float>(0.5*(g0.mu[i] - g1.mu[i]));
    }
    int idx_covar = 0;
    for (int i = 0; i < d; i++) {
        for (int j = i; j < d; j++) {
            tmp.covar[idx_covar] = 0.5f*
                    (g0.covar[idx_covar] + g1.covar[idx_covar]) +
                    tmp.mu[i]*tmp.mu[j];
            idx_covar++;
        }
    }

    // Do an inplace cholesky decompositon and compute logdet of the merged
    // Gaussian.
    int idx_ii = 0;
    for (int i = 0; i < d; i++) {
        int idx_k = i;
        for (int k = 0; k < i; k++) {
            tmp.covar[idx_ii] -=
                    tmp.covar[idx_k]*tmp.covar[idx_k];
            idx_k += d - k - 1;
        }

        if (tmp.covar[idx_ii] <= 0) {
            return -1;
        }
        tmp.covar[idx_ii] = std::sqrt(tmp.covar[idx_ii]);
        jsd += std::log(tmp.covar[idx_ii]);

        int idx_ij = idx_ii;
        for (int j = i+1; j < d; j++) {
            idx_ij++;

            idx_k = 0;
            for (int k = 0; k < i; k++) {
                tmp.covar[idx_ij] -=
                        tmp.covar[idx_k+i] * tmp.covar[idx_k+j];
                idx_k += d - k - 1;
            }
            tmp.covar[idx_ij] /= tmp.covar[idx_ii];
        }

        idx_ii += d - i;
    }

    if (std::isnan(jsd) || std::isinf(jsd)) {
        return std::numeric_limits<float>::max();
    }

    return std::sqrt(std::max(0.0f, jsd));
}

float
gaussian_statistics::symmetric_kullbackleibler(
        const gaussian& g0,
        const gaussian& g1,
        gaussian& tmp)
{
    // distance value
    float skld = 0;

    // return 0 if the models to compare are the same
    if ((g0.covar == g1.covar) && (g0.mu == g1.mu)) {
        return skld;
    }


    // add the two inverted covariances
    for (int i = 0; i < covar_elems; i++) {
        tmp.covar_inverse[i] = g0.covar_inverse[i] + g1.covar_inverse[i];
    }

    for (int i = 0; i < d; i++) {
        int idx = i*d - (i*i+i)/2;

        skld += g0.covar[idx+i] * g1.covar_inverse[idx+i] +
                g1.covar[idx+i] * g0.covar_inverse[idx+i];

        for (int k = i+1; k < d; k++) {
            skld += 2*g0.covar[idx+k] * g1.covar_inverse[idx+k] +
                2*g1.covar[idx+k] * g0.covar_inverse[idx+k];
        }
    }

    // compute the difference of the two means
    for (int i = 0; i < d; i++) {
        tmp.mu[i] = g0.mu[i] - g1.mu[i];
    }

    for (int i = 0; i < d; i++) {
        int idx = i - d;
        float tmp1 = 0;

        for (int k = 0; k <= i; k++) {
            idx += d - k;
            tmp1 += tmp.covar_inverse[idx] * tmp.mu[k];
        }

        for (int k = i + 1; k < d; k++) {
            idx++;
            tmp1 += tmp.covar_inverse[idx] * tmp.mu[k];
        }
        skld += tmp1 * tmp.mu[i];
    }

    if (std::isnan(skld) || std::isinf(skld)) {
        return std::numeric_limits<float>::max();
    }

    return std::max(skld/4 - d/2, 0.0f);
}

} /* namespace musly */
