/*
 * Copyright 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MUSLY_TYPES_H_
#define MUSLY_TYPES_H_

/** The Musly base object storing the initialized music similarity method and
 * audio decoder information. A reference to an initialized musly_jukebox
 * object is required for almost any Musly call.
 *
 * \sa musly_jukebox_poweron(), musly_jukebox_poweroff()
 */
typedef struct {
    /** A reference to the initialized music similarity method. Hides a C++
     * musly::method object.
     */
    void* method;

    /** Method name as null terminated string
     */
    char* method_name;

    /** A reference to the initialized audio file decoder. Hides a C++
     * musly::decoder object.
     */
    void* decoder;

    /** Decoder name as null terminated string
     */
    char* decoder_name;
} musly_jukebox;


/** A musly_track object typically represents the features extracted with an
 * music similarity method. The features are stored linearly in a float* array.
 * Each music similarity method may write different features into this
 * structure. Thus the size of the structure will vary too. To allocate a
 * Musly track use  musly_track_alloc(). To get the size (in bytes) of a
 * Musly track use musly_track_size().
 */
typedef float musly_track;


/** An identifier given to a musly track when registering it with a jukebox.
 */
typedef int musly_trackid;


#endif // MUSLY_TYPES_H_
