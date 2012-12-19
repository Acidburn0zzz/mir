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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_COMPOSITOR_COMPOSITOR_H_
#define MIR_COMPOSITOR_COMPOSITOR_H_

#include "mir/compositor/drawer.h"
#include "mir/compositor/render_view.h"

#include <memory>

namespace mir
{
namespace graphics
{

class Renderer;

}

namespace compositor
{

class Compositor : public Drawer
{
public:
    explicit Compositor(
        RenderView* render_view,
        const std::shared_ptr<graphics::Renderer>& renderer);

    virtual void render(graphics::Display* display);

private:
    RenderView* const render_view;
    std::shared_ptr<graphics::Renderer> renderer;
};


}
}

#endif /* MIR_COMPOSITOR_COMPOSITOR_H_ */
