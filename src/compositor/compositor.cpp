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

#include "mir/compositor/compositor.h"

#include "mir/geometry/rectangle.h"
#include "mir/graphics/display.h"
#include "mir/graphics/renderable.h"
#include "mir/graphics/renderer.h"
#include "mir/surfaces/scenegraph.h"
#include "mir/surfaces/surface.h"

#include <cassert>
#include <functional>

namespace mc = mir::compositor;
namespace mg = mir::graphics;
namespace ms = mir::surfaces;

mc::Compositor::Compositor(
    Renderview *view,
    const std::shared_ptr<mg::Renderer>& renderer)
    : render_view(view),
      renderer(renderer)
{
    assert(render_view);
    assert(renderer);
}

namespace
{

struct RegionRenderableFilter : public mc::RenderableFilter
{
    RegionRenderableFilter(mir::geometry::Rectangle enclosing_region)
		: enclosing_region(enclosing_region)
	{
	}
	bool operator()(mg::Renderable& /*renderable*/)
	{
		return true;
	}
    mir::geometry::Rectangle& enclosing_region;
};

struct RenderingRenderableOperator : public mc::RenderableOperator
{
	RenderingRenderableOperator(mg::Renderer& renderer)
		: renderer(renderer)
	{
	}
	void operator()(mg::Renderable& renderable)
	{
		renderer.render(renderable);
	}
	mg::Renderer& renderer;
};
}

void mc::Compositor::render(graphics::Display* display)
{
    RegionRenderableFilter filter(display->view_area());
	RenderingRenderableOperator rendering_operator(*renderer);

	render_view->apply(filter, rendering_operator);
    
    display->post_update();
}
