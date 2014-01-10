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

#include <string>
#include <iostream>
#include <climits>

#include "plugins.h"

namespace musly {

const int plugins::METHOD_TYPE = 0;
const int plugins::DECODER_TYPE = 1;


plugin*
plugins::instantiate_plugin(
        int type,
        std::string& classname)
{
    std::map<std::string, plugin_creator*>::iterator i;

    if (classname.length() > 0) {
        i = get_plugin_table().find(classname);
        if (i != get_plugin_table().end()) {
            if (i->second->get_type() == type) {
                plugin* pl = i->second->create();
                return pl;
            }
        }
    } else {

        // search for the default plugin, i.e. the highest priority.
        i = get_plugin_table().begin();
        int p = INT_MIN;
        plugin_creator* pc = NULL;
        std::string name;
        while (i != get_plugin_table().end()) {
            if (i->second->get_type() == type) {
                int cur_p = i->second->get_priority();
                if (cur_p > p) {
                    p = cur_p;
                    pc = i->second;
                    name = i->first;
                }
            }
            i++;
        }

        // return the plugin with the highest priority
        if (pc) {
            plugin* pl = pc->create();
            classname = name;
            return pl;
        }
    }

    return NULL;
}

void
plugins::register_plugin(
        const std::string& classname,
        plugin_creator* c)
{
    get_plugin_table()[classname] = c;

}

const std::string&
plugins::get_plugins(
        int type)
{
    std::map<std::string, plugin_creator*> p = get_plugin_table();
    std::map<std::string, plugin_creator*>::iterator i = p.begin();

    static std::string pn;
    pn = "";

    int added = 0;
    while (i != p.end()) {
        if (i->second->get_type() == type) {
            if (added != 0) {
                pn.append(",");
            }
            pn.append(i->first);
            added++;
        }
        i++;
    }

    return pn;

}

std::map<std::string, plugin_creator*>&
plugins::get_plugin_table()
{
    static std::map<std::string, plugin_creator*> table;
    return table;
}

plugin_creator::plugin_creator(const std::string& classname)
{
   plugins::register_plugin(classname, this);
}



} /* namespace musly */
