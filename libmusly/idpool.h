/*
 * Copyright 2014, Jan Schlueter <jan.schlueter@ofai.at>
 *
 * This file is part of Musly, a program for high performance music
 * similarity computation: http://www.musly.org/.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Provides two helper classes for music similarity measures to manage a
 * collection of track ids: unordered_idpool and ordered_idpool. The former
 * just keeps a set of track ids to help a music similarity measure implement
 * consistent add_tracks, remove_tracks, get_trackcount and get_maxtrackid
 * methods. The latter also provides a mapping from all currently registered
 * track ids to consecutive indices from <tt>0</tt> to <tt>get_size() - 1</tt>,
 * and guarantees that track ids are always added or deleted from the end.
 * By using this mapping and registering an ordered_idpool_observer with the
 * ordered_idpool to be informed of updates to the mapping, a music similarity
 * measure can store per-track metadata efficiently in an array.
 */

#ifndef MUSLY_IDPOOL_H_
#define MUSLY_IDPOOL_H_

#include <set>
#include <map>
#include <vector>

namespace musly {

template <typename T>
class idpool {

protected:
    /** Largest id ever registered with this instance
     */
    T max_seen;

    idpool(): max_seen(-1) {}

public:
    virtual ~idpool() {}

    /** Return the number of registered ids
     */
    virtual int
    get_size() = 0;

    /** Return the largest id ever registered with this instance
     */
    T
    get_max_seen() {
        return max_seen;
    }

    /** Register a bunch of ids and return how many of them were new
     */
    virtual int
    add_ids(T* ids, int length) = 0;

    /** Generate and register a bunch of ids, starting with get_max_seen() + 1
     */
    virtual void
    generate_ids(T* ids, int length) {
        for (int i = 0; i < length; i++) {
            ids[i] = ++max_seen;
        }
        add_ids(ids, length);
    }

    /** Deregister a bunch of ids and return how many of them were known
     */
    virtual int
    remove_ids(T* ids, int length) = 0;

};


template <typename T>
class unordered_idpool :
        public idpool<T>
{
private:
    std::set<T> registered_ids;

public:
    unordered_idpool() {}

    inline const std::set<T>& idset() const {
        return registered_ids;
    }

    int
    get_size() {
        return static_cast<int>(registered_ids.size());
    }

    int
    add_ids(T* ids, int length) {
        int added = 0;
        for (int i = 0; i < length; i++) {
            if (registered_ids.insert(ids[i]).second) {
                added++;
                if (ids[i] > idpool<T>::max_seen) {
                    idpool<T>::max_seen = ids[i];
                }
            }
        }
        return added;
    }

    int
    remove_ids(T* ids, int length) {
        int deleted = 0;
        for (int i = 0; i < length; i++) {
            if (registered_ids.erase(ids[i])) {
                deleted++;
            }
        }
        return deleted;
    }

    void
    export_ids(int from, int to, T* ids) {
        typename std::set<T>::iterator it = registered_ids.begin();
        std::advance(it, from);
#if __cplusplus > 199711L
        std::copy_n(it, to - from, ids);
#else
        while (from < to) {
            *ids++ = *it++;
            from++;
        }
#endif
    }
};


class ordered_idpool_observer
{
public:
    virtual ~ordered_idpool_observer() {};

    virtual void
    swapped_positions(int pos_a, int pos_b) = 0;
};


template <typename T>
class ordered_idpool :
        public idpool<T>
{
private:
    ordered_idpool_observer* observer;
    std::vector<T> registered_ids;
    std::map<T,int> positions;

    void
    swap_positions(int pos_a, int pos_b, typename std::map<T,int>::iterator map_a) {
        if (pos_a == pos_b) {
            return;
        }
        // gather information
        T id_a = registered_ids[pos_a];
        T id_b = registered_ids[pos_b];
        // swap in `registered_ids`
        registered_ids[pos_a] = id_b;
        registered_ids[pos_b] = id_a;
        // swap in `positions`
        map_a->second = pos_b;
        positions[id_b] = pos_a;
        // notify observer (if any)
        if (observer) {
            observer->swapped_positions(pos_a, pos_b);
        }
    }

public:
    ordered_idpool() : observer(NULL) {};

    void
    set_observer(ordered_idpool_observer* obs) {
        this->observer = obs;
    }

    inline const std::vector<T>& idlist() const {
        return registered_ids;
    }

    inline const std::map<T,int>& posmap() const {
        return positions;
    }

    inline const T& operator[](int const& index) const {
        return registered_ids[index];
    }

    inline int
    position_of(T id) {
        typename std::map<T,int>::iterator it = positions.find(id);
        if (it != positions.end()) {
            return it->second;
        }
        return -1;
    }

    inline int
    get_size() {
        return static_cast<int>(registered_ids.size());
    }

    /** Move a bunch of ids to the end of idlist(), in their given order.
     * Unknown ids are skipped. Returns how many ids were known (and moved).
     */
    int
    move_to_end(T* ids, int length) {
        int start = static_cast<int>(registered_ids.size());
        for (int i = length - 1; i >= 0; i--) {
            typename std::map<T,int>::iterator it = positions.find(ids[i]);
            if (it != positions.end()) {
                start--;
                swap_positions(it->second, start, it);
            }
        }
        return static_cast<int>(registered_ids.size()) - start;
    }

    /** Register a bunch of ids and return how many of them were new
     * After calling, the last \p length items of idlist() equal \p ids
     */
    int
    add_ids(T* ids, int length) {
        // move all known ids to the end
        int num_known = move_to_end(ids, length);
        // make enough room to add unknown ids
        int start = static_cast<int>(registered_ids.size()) - num_known;
        registered_ids.resize(static_cast<size_t>(start + length));
        // overwrite the last `length` elements with the given `ids`
        for (int i = 0; i < length; i++) {
            registered_ids[start + i] = ids[i];
            positions[ids[i]] = start + i;
            if (ids[i] > idpool<T>::max_seen) {
                idpool<T>::max_seen = ids[i];
            }
        }
        return length - num_known;
    }

    /** Generate and register a bunch of ids, starting with get_max_seen() + 1
     * After calling, the last \p length items of idlist() equal \p ids
     */
    void
    generate_ids(T* ids, int length) {
        // generate ids
        for (int i = 0; i < length; i++) {
            ids[i] = ++idpool<T>::max_seen;
        }
        // make enough room to add all ids
        size_t size = registered_ids.size();
        registered_ids.reserve(size + length);
        // append ids to the end
        for (int i = 0; i < length; i++) {
            registered_ids.push_back(ids[i]);
            positions[ids[i]] = static_cast<int>(size++);
        }
    }

    /** Deregister a bunch of ids and return how many of them were known
     */
    int
    remove_ids(T* ids, int length) {
        // move all known ids to the end, then remove them
        int num_known = move_to_end(ids, length);
        remove_last(num_known);
        return num_known;
    }

    /** Deregisters the given number of ids from the end of idlist()
     */
    void
    remove_last(int length) {
        int start = static_cast<int>(registered_ids.size()) - length;
        for (int i = start; i < start + length; i++) {
            positions.erase(registered_ids[i]);
        }
        registered_ids.resize(start);
    }
};

} /* namespace musly */

#endif /* MUSLY_IDPOOL_H_ */
