/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_SURFACES_SCENEGRAPH_H_
#define MIR_SURFACES_SCENEGRAPH_H_

#include "mir/geometry/forward.h"

#include <memory>

namespace mir
{
namespace surfaces
{

class Surface;

class SurfaceEnumerator
{
public:
    virtual ~SurfaceEnumerator() {}

    virtual void operator()(const std::shared_ptr<Surface>& surface) = 0;

protected:
    SurfaceEnumerator() = default;
    SurfaceEnumerator(const SurfaceEnumerator&) = delete;
    SurfaceEnumerator& operator=(const SurfaceEnumerator&) = delete;
};

class SurfaceCollection
{
public:
    virtual ~SurfaceCollection() {}

    virtual void invoke_for_each_surface(SurfaceEnumerator& enumerator) = 0;

  protected:    
    SurfaceCollection() = default;
    SurfaceCollection(const SurfaceCollection&) = delete;
    SurfaceCollection& operator=(const SurfaceCollection&) = delete;

};

// scenegraph is the interface compositor uses onto the surface stack
class Scenegraph
{
public:
    
    virtual ~Scenegraph() {}
    
    virtual std::shared_ptr<SurfaceCollection> get_surfaces_in(geometry::Rectangle const& display_area) = 0;

protected:
    Scenegraph() = default;
    
private:
    Scenegraph(Scenegraph const&) = delete;
    Scenegraph& operator=(Scenegraph const&) = delete;
};
}
}

#endif /* MIR_SURFACES_SCENEGRAPH_H_ */
