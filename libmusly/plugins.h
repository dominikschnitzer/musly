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

#ifndef MUSLY_PLUGINS_H_
#define MUSLY_PLUGINS_H_

#include <map>
#include <string>

namespace musly {

class plugin {
};

class plugin_creator
{
public:
    plugin_creator(
            const std::string& classname);

    virtual ~plugin_creator() { };

    virtual int get_type() = 0;
    virtual int get_priority() = 0;
    virtual plugin* create() = 0;
};

template <class T>
class plugin_creator_impl : public plugin_creator
{
private:
    int type;
    int priority;

public:
    plugin_creator_impl(
            const std::string& classname,
            int type,
            int priority) :
        plugin_creator(classname),
        type(type),
        priority(priority) { };

    virtual int get_type() {
        return type;
    }
    virtual int get_priority() {
        return priority;
    }
    virtual plugin* create() { return new T; };
};

class plugins
{
public:
    static plugin*
    instantiate_plugin(
            int type,
            std::string& classname);

    static void
    register_plugin(
            const std::string& classname,
            plugin_creator* c);

    static const std::string&
    get_plugins(int type);

    static const int METHOD_TYPE;
    static const int DECODER_TYPE;

private:
    static std::map<std::string, plugin_creator*>&
    get_plugin_table();
};

} /* namespace musly */
#endif /* MUSLY_PLUGINS_H_ */
