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

#ifndef MUSLY_METHOD_H_
#define MUSLY_METHOD_H_

#include <string>
#include <vector>
#include "plugins.h"
#include "musly/musly_types.h"

namespace musly {

class method :
        public plugin
{
private:
    /** A list of feature names as initialized with \sa track_addfield_floats().
     */
    std::vector<std::string> track_field_name;

    /** A list of feature sizes as initialized with \sa track_addfield_floats().
     */
    std::vector<int> track_field_size;

    /** The number of elements of a musly_track float array.
     */
    int track_size;

    /** Holds tracks serialized to a string
     */
    std::string trackstr;

    /** Current trackid
     */
    musly_trackid current_tid;

protected:
    /** Add features to the Musly method track model. Each musly::method music
     * similarity method needs to store the features for each music track in a
     * musly_track structure. The structure is a simple array of floats,
     * initially of size 0. To define the structure of your methods features
     * and add space for them in a musly_track, use this function. The function
     * returns the offset where the currently added features start.
     *
     * I.e., if you initialized your features with:
     * \code
     *   int zc_offset = track_addfield_floats("zerocrossings", 1);
     * \endcode
     * you can access the allocated feature with:
     * \code
     *   musly_track* t = ... // allocated, analyzed track
     *   fload zc = t[zc_offset];
     * \endcode
     *
     * \param name The name of the features.
     * \param num_floats The number of elements (float values) that will be
     * used to store the features.
     * \returns The offset of the added feature relative to a musly_track.
     */
    int
    track_addfield_floats(
            const std::string& name,
            int num_floats);

public:
    method();
    virtual ~method();

    /** A short description of the implemented music similarity method. Give a
     * short description or reference the music similarity method implemented
     * by your object.
     *
     * \returns A null terminated string describing th music similarity method.
     */
    virtual const char*
    about() = 0;

    /** Return the number of floats in a musly_track in bytes. The is dependent
     * on how the music similarity method initialized it with \sa
     * track_addfield_floats().
     *
     * \returns the number of elements of a musly_track.
     */
    int
    track_getsize();

    /** Allocate a musly_track.
     */
    musly_track*
    track_alloc();

    /**
     *
     */
    virtual const char*
    track_tostr(
            musly_track* track);

    /**
     *
     */
    virtual int
    analyze_track(
            float* pcm,
            int length,
            musly_track* track) = 0;

    /**
     *
     */
    virtual int
    similarity(
            musly_track* track,
            musly_trackid seed_trackid,
            musly_track** tracks,
            musly_trackid* trackids,
            int length,
            float* similarities) = 0;

    /**
     *
     */
    virtual int
    guess_neighbors(
            musly_trackid seed,
            musly_trackid* neighbors,
            int length);

    /**
     *
     */
    virtual int
    set_musicstyle(
            musly_track** tracks,
            int length);

    /**
     *
     */
    void
    add_tracks(
            musly_track** tracks,
            musly_trackid* trackids,
            int length);

    /**
     *
     */
    virtual void
    init_tracks(
            musly_track** tracks,
            musly_trackid* trackids,
            int length);

};

/** A macro to facilitating registering a method class with musly. This macro
 * has to be used in the header and class declaration. Call it with the class
 * name as parameter.
 */
#define MUSLY_METHOD_REGCLASS(classname) \
private: \
    static const plugin_creator_impl<classname> creator;

/** A macro to facilitate registering a method class with musly. This macro has
 * to be used in the source file, it has two parameters. Call it with the name
 * of your class and the priority. The priority value is used if the user
 * does not request a special musly::method when calling \sa
 * musly_jukebox_poweron(). The method with the lowest priority value is used.
 */
#define MUSLY_METHOD_REGIMPL(classname, priority) \
    const plugin_creator_impl<classname> classname::creator(#classname, \
            plugins::METHOD_TYPE, priority);

} /* namespace musly */
#endif /* MUSLY_METHOD_H_ */
