/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_COMPOSITOR_MOCK_DISPLAY_H_
#define MIR_COMPOSITOR_MOCK_DISPLAY_H_

#include "mir/graphics/display.h"
#include <gmock/gmock.h>

namespace mir
{
namespace graphics
{

struct MockDisplay : public Display
{
public:
    MOCK_METHOD0(view_area, geometry::Rectangle ());
    MOCK_METHOD0(post_update, bool ());
};

}
}
#endif /* MIR_COMPOSITOR_MOCK_DISPLAY_H_ */
