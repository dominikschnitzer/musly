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

#include <algorithm>
#include <Eigen/Core>

#include "minilog.h"
#include "windowfunction.h"
#include "timbre.h"


namespace musly {
namespace methods {

/** Register timbre with musly with piority (1)
 */
MUSLY_METHOD_REGIMPL(timbre, 1);



timbre::timbre() :

        // initialize method configuration parameters
        sample_rate(22050),
        window_size(1024),
        hop(0.5f),
        max_pcmlength(60*sample_rate),
        ps_bins(window_size/2+1),
        mel_bins(36),
        mfcc_bins(25),

        // spectra and filters
        ps(windowfunction::hann(window_size), hop),
        mel(ps_bins, mel_bins, sample_rate),
        mfccs(mel_bins, mfcc_bins),
        gs(mfcc_bins),
        mp(this)
{
    // Configure the musly_track features and save the musly_track offsets

    // the feature mean
    track_mu = track_addfield_floats("gaussian.mu", gs.get_dim());
    // add the covariance (symmetric matrix)
    track_covar = track_addfield_floats("gaussian.covar", gs.get_covarelems());
    // add the log(det(covar)) of the covariance for performance reasons
    track_logdet = track_addfield_floats("gaussian.covar_logdet", 1);

    // React on changes to the trackid mapping in the ordered_idpool
    idpool.set_observer(this);
}

timbre::~timbre()
{
}

const char*
timbre::about()
{
    return
        "A timbre only music similarity measure based 'mandelellis'. It\n"
        "improves the basic measure in multiple ways to achieve superior\n"
        "results:\n"
        "We compute a single Gaussian representation from the songs\n"
        "using 25 MFCCs. The similarity between two tracks is computed\n"
        "with the Jensen-Shannon divergence. The Similarities are\n"
        "normalized with Mutual Proximity:\n"
        "D. Schnitzer et al.: Using mutual proximity to improve\n"
        "content-based audio similarity. In the proceedings of the 12th\n"
        "International Society for Music Information Retrieval\n"
        "Conference, ISMIR, 2011.";
}

int
timbre::analyze_track(
        float* pcm,
        int length,
        musly_track* track)
{
    MINILOG(logTRACE) << "T analysis started. samples=" << length;

    // select the central max_pcmlength (usually 60s) of the piece
    int start = 0;
    if (length > max_pcmlength) {
        start = (length - max_pcmlength) / 2;
        length = max_pcmlength;
    }

    // PCM --> powerspectrum
    Eigen::Map<Eigen::VectorXf> pcm_vector(pcm+start, length);
    Eigen::MatrixXf power_spectrum = ps.from_pcm(pcm_vector);

    // powerspectrum -> Mel
    Eigen::MatrixXf mel_spectrum = mel.from_powerspectrum(power_spectrum);

    // Mel -> MFCC representation
    Eigen::MatrixXf mfcc_representation =
            mfccs.from_melspectrum(mel_spectrum);

    // estimate the Gaussian from the MFCC representation
    gaussian g = {0, 0, 0, 0};
    g.mu = &track[track_mu];
    g.covar = &track[track_covar];
    g.covar_logdet = &track[track_logdet];
    if (gs.estimate_gaussian(mfcc_representation, g) == false) {
        MINILOG(logTRACE) << "T Gaussian model estimation failed.";
        return 2;
    }

    MINILOG(logTRACE) << "T analysis finished!";

    return 0;
}


void
timbre::similarity_raw(
        musly_track* track,
        musly_track** tracks,
        int length,
        float* similarities)
{
    // map seed track to gaussian structure
    gaussian g0;
    g0.mu = &track[track_mu];
    g0.covar = &track[track_covar];
    g0.covar_logdet = &track[track_logdet];

    // create the temporary buffer required for the Jensen-Shannon divergence
    musly_track* tmp_t = track_alloc();
    gaussian tmp;
    tmp.mu = &tmp_t[track_mu];
    tmp.covar = &tmp_t[track_covar];
    tmp.covar_logdet = &tmp_t[track_logdet];

    // iterate over all musly_tracks to compute the Jensen-Shannon divergence
    for (int i = 0; i < length; i++) {
        gaussian gi;
        musly_track* track1 = tracks[i];
        gi.mu = &track1[track_mu];
        gi.covar = &track1[track_covar];
        gi.covar_logdet = &track1[track_logdet];

        similarities[i] = gs.jensenshannon(g0, gi, tmp);
    }

    delete[] tmp_t;
}



int
timbre::similarity(
        musly_track* track,
        musly_trackid seed_trackid,
        musly_track** tracks,
        musly_trackid* trackids,
        int length,
        float* similarities)
{
    if ((length <= 0) || !track || ! tracks || !similarities) {
        return -1;
    }

    // compute raw similarities
    similarity_raw(track, tracks, length, similarities);

    // normalize with mp
    // - lookup positions of trackids in the ordered_idpool
    int seed_position = idpool.position_of(seed_trackid);
    int* other_positions = new int[length];
    for (int i = 0; i < length; i++) {
        other_positions[i] = idpool.position_of(trackids[i]);
    }
    // - call mp.normalize with these positions
    int res = mp.normalize(seed_position, other_positions, length, similarities);
    delete[] other_positions;

    return res;
}

int
timbre::set_musicstyle(
            musly_track** tracks,
            int length)
{
    MINILOG(logTRACE) << "T initializing mutual proximity!";

    // save the mp normalization tracks
    return mp.set_normtracks(tracks, length);
}

int
timbre::add_tracks(
        musly_track** tracks,
        musly_trackid* trackids,
        int length,
        bool generate_ids) {
    if (mp.get_normtracks()->size() == 0) {
        return -1;  // not initialized, cannot add tracks
    }
    int num_new;
    if (generate_ids) {
        idpool.generate_ids(trackids, length);
        num_new = length;
    }
    else {
        num_new = idpool.add_ids(trackids, length);
    }

    Eigen::VectorXf sim(mp.get_normtracks()->size());
    mp.append_normfacts(num_new);
    int pos = idpool.get_size() - length;
    for (int i = 0; i < length; i++) {
        similarity_raw(tracks[i], mp.get_normtracks()->data(),
                static_cast<int>(mp.get_normtracks()->size()), sim.data());

        mp.set_normfacts(pos + i, sim);
    }
    return 0;
}

void
timbre::remove_tracks(
        musly_trackid* trackids,
        int length) {
    length = idpool.move_to_end(trackids, length);
    mp.trim_normfacts(length);
    idpool.remove_last(length);
}

int
timbre::get_trackcount() {
    return idpool.get_size();
}

int
timbre::get_maxtrackid() {
    return idpool.get_max_seen();
}

int
timbre::get_trackids(
        musly_trackid* trackids) {
    std::copy(idpool.idlist().begin(), idpool.idlist().end(), trackids);
    return idpool.get_size();
}

void
timbre::swapped_positions(
        int pos_a,
        int pos_b) {
    // positions in idpool have changed; update mp index accordingly
    mp.swap_normfacts(pos_a, pos_b);
}

int
timbre::serialize_metadata(
        unsigned char* buffer) {
    if (buffer) {
        // number of registered tracks
        *(int*)(buffer) = idpool.get_size();
        buffer += sizeof(int);

        // largest seen track id
        *(musly_trackid*)(buffer) = idpool.get_max_seen();
        buffer += sizeof(musly_trackid);

        // mutual proximity tracks
        std::vector<musly_track*> &mptracks = *mp.get_normtracks();
        *(int*)(buffer) = static_cast<int>(mptracks.size());
        buffer += sizeof(int);
        for (int i = 0; i < (int)mptracks.size(); i++) {
            std::copy(mptracks[i], mptracks[i] + track_getsize(), (musly_track*)buffer);
            buffer += track_getsize() * sizeof(musly_track);
        }
    }
    return static_cast<int>(sizeof(int) + sizeof(musly_trackid) + sizeof(int)
            + mp.get_normtracks()->size() * track_getsize() * sizeof(musly_track));
}

int
timbre::deserialize_metadata(
        unsigned char* buffer) {
    // number of registered tracks
    int expected_tracks = *(int*)(buffer);
    buffer += sizeof(int);

    // largest seen track id
    musly_trackid max_seen = *(musly_trackid*)(buffer);
    buffer += sizeof(musly_trackid);
    idpool.add_ids(&max_seen, 1);
    idpool.remove_ids(&max_seen, 1);

    // mutual proximity tracks
    int num_mptracks = *(int*)(buffer);
    buffer += sizeof(int);
    musly_track** mptracks = new musly_track*[num_mptracks];
    for (int i = 0; i < num_mptracks; i++) {
        mptracks[i] = (musly_track*)buffer;
        buffer += track_getsize() * sizeof(musly_track);
    }
    mp.set_normtracks(mptracks, num_mptracks);
    delete[] mptracks;
    mp.append_normfacts(expected_tracks);

    return expected_tracks;
}

int
timbre::serialize_trackdata(
        unsigned char* buffer,
        int num_tracks,
        int skip_tracks) {
    if ((num_tracks < 0) || (skip_tracks < 0)) {
        return -1;
    }
    if (buffer) {
        if (num_tracks + skip_tracks > idpool.get_size()) {
            return -1;
        }
        for (int i = skip_tracks; i < skip_tracks + num_tracks; i++) {
            *(musly_trackid*)(buffer) = idpool[i];
            buffer += sizeof(musly_trackid);
            mp.get_normfacts(i,
                    (float*)(buffer),
                    (float*)(buffer + sizeof(float)));
            buffer += 2 * sizeof(float);
        }
    }
    return num_tracks * (sizeof(musly_trackid) + 2 * sizeof(float));
}

int
timbre::deserialize_trackdata(
        unsigned char* buffer,
        int num_tracks) {
    if (num_tracks < 0) {
        return -1;
    }
    int had_tracks = idpool.get_size();
    for (int i = 0; i < num_tracks; i++) {
        idpool.add_ids((musly_trackid*)buffer, 1);
        buffer += sizeof(musly_trackid);
        mp.set_normfacts(had_tracks + i,
                *(float*)(buffer),
                *(float*)(buffer + sizeof(float)));
        buffer += 2 * sizeof(float);
    }
    return num_tracks;
}

} /* namespace methods */
} /* namespace musly */
