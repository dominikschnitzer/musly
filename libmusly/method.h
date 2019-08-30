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
     *   float zc = t[zc_offset];
     * \endcode
     *
     * \param[in] name The name of the features.
     * \param[in] num_floats The number of elements (float values) that will be
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
     * \returns A null terminated string describing the music similarity method.
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
            int length,
            musly_trackid* limit_to,
            int num_limit_to);

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
    virtual int
    add_tracks(
            musly_track** tracks,
            musly_trackid* trackids,
            int length,
            bool generate_ids) = 0;

    /**
     *
     */
    virtual void
    remove_tracks(
            musly_trackid* trackids,
            int length) = 0;

    /**
     *
     */
    virtual int
    get_trackcount() = 0;

    /**
     *
     */
    virtual int
    get_maxtrackid() = 0;

    /**
     *
     */
    virtual int
    get_trackids(
            musly_trackid* trackids) = 0;

    /**
     * Writes metadata about the jukebox state into a binary buffer.
     *
     * \param buffer The buffer to write to, or <tt>NULL</tt> to query the
     * required buffer size.
     * \returns The number of bytes written to the buffer, or the number of
     * bytes that would have been written if \p buffer is <tt>NULL</tt>, or
     * -1 in case of an error.
     */
    virtual int
    serialize_metadata(
            unsigned char* buffer);

    /**
     * Writes the jukebox state for registered tracks into a binary buffer.
     *
     * \param buffer The buffer to write to, or <tt>NULL</tt> to query the
     * required buffer size assuming that the state for \p num_tracks tracks
     * will be written.
     * \param num_tracks The number of registered tracks to write the jukebox
     * state for.
     * \param skip_tracks The number of tracks to skip.
     * \returns The number of bytes written to the buffer, or the number of
     * bytes that would be needed to export the state for \o num_tracks
     * tracks if \p buffer is <tt>NULL</tt>, or -1 in case of an error.
     *
     * \note If buffer is not <tt>NULL</tt>, <tt>num_tracks + skip_tracks</tt>
     * must not be greater than get_trackcount().
     */
    virtual int
    serialize_trackdata(
            unsigned char* buffer,
            int num_tracks,
            int skip_tracks = 0);

    /**
     * Initiates restoring the jukebox state from a binary buffer.
     *
     * \param buffer The buffer to read from.
     * \returns The total number of track states expected to be restored via
     * deserialize_trackdata() for this jukebox, or -1 in case of an error.
     *
     * \note The number of bytes read is the same that was written by
     * serialize_metadata(). It depends on the internal state of the jukebox
     * that was serialized.
     */
    virtual int
    deserialize_metadata(
            unsigned char* buffer);

    /**
     * Restores the jukebox state for registered tracks from a binary buffer.
     *
     * \param buffer The buffer to read from.
     * \param num_tracks The number of track states to read and restore.
     * \returns The number of track states restored, or -1 in case of an error.
     *
     * \note The number of bytes read is <tt>serialize_trackdata(NULL, num_tracks, 0)</tt>
     */
    virtual int
    deserialize_trackdata(
            unsigned char* buffer,
            int num_tracks);

};

/** A macro to facilitating registering a method class with musly. This macro
 * has to be used in the header and class declaration. Call it with the class
 * name as parameter.
 */
#ifndef BUILD_STATIC
#define MUSLY_METHOD_REGCLASS(classname) \
private: \
    static const plugin_creator_impl<classname> creator
#else
#define MUSLY_METHOD_REGCLASS(classname)
#endif

/** A macro to facilitate registering a method class with musly. This macro has
 * to be used in the source file, it has two parameters. Call it with the name
 * of your class and the priority. The priority value is used if the user
 * does not request a special musly::method when calling \sa
 * musly_jukebox_poweron(). The method with the highest priority value is used.
 */
#ifndef BUILD_STATIC
#define MUSLY_METHOD_REGIMPL(classname, priority) \
    const plugin_creator_impl<classname> classname::creator(#classname, \
            plugins::METHOD_TYPE, priority)
#else
#define MUSLY_METHOD_REGIMPL(classname, priority)
#endif

/** Alternative form for MUSLY_METHOD_REGIMPL to be used for static builds of
 * the library. This form has to be placed in global scope in lib.cpp to make
 * the plugin available in static builds.
 */
#ifdef BUILD_STATIC
#define MUSLY_METHOD_REGSTATIC(classname, priority) \
    static const musly::plugin_creator_impl<musly::methods::classname> \
            create ## _ ## classname (#classname, \
            musly::plugins::METHOD_TYPE, priority)
#endif

} /* namespace musly */
#endif /* MUSLY_METHOD_H_ */
