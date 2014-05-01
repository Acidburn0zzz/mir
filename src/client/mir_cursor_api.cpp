/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

#include "mir_toolkit/mir_cursor_configuration.h"
#include "cursor_configuration.h"

void mir_cursor_configuration_destroy(MirCursorConfiguration *cursor)
{
    delete cursor;
}

MirCursorConfiguration* mir_cursor_configuration_disabled()
{
    auto c = new MirCursorConfiguration;
    c->name = "";
    
    return c;
}

MirCursorConfiguration* mir_cursor_configuration_default()
{
    auto c = new MirCursorConfiguration;
    c->name = "default";
    
    return c;
}

MirCursorConfiguration* mir_cursor_configuration_from_name(char const* name)
{
    auto c = new MirCursorConfiguration;
    c->name = std::string(name);
    
    return c;
}
