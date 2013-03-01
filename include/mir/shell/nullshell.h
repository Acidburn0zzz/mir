/*
 * Null Shell
 *
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#ifndef __MIR_NULLSHELL_H__
#define __MIR_NULLSHELL_H__

#include "mir/shell/shell.h"

namespace mir
{
namespace shell
{

class NullShell : public mir::Shell
{
public:
    ~NullShell();

    bool start();
    void stop();
    bool supports(MirSurfaceAttrib attrib) const;
    bool supports(MirSurfaceAttrib attrib, int value) const;
};

} // namespace shell
} // namespace mir

#endif
