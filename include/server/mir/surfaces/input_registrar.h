/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

#ifndef MIR_SURFACES_INPUT_REGISTRAR_H_
#define MIR_SURFACES_INPUT_REGISTRAR_H_

#include <memory>

namespace mir
{
namespace input
{
class SessionTarget;
class SurfaceTarget;
}

namespace surfaces
{

/// An interface used to register input targets and take care of input assosciation (i.e.
/// create input channels).
class InputRegistrar
{
public:
    virtual ~InputRegistrar() = default;
    
    virtual void input_surface_opened(std::shared_ptr<input::SurfaceTarget> const& opened_surface) = 0;
    virtual void input_surface_closed(std::shared_ptr<input::SurfaceTarget> const& closed_surface) = 0;

protected:
    InputRegistrar() = default;
    InputRegistrar(InputRegistrar const&) = delete;
    InputRegistrar& operator=(InputRegistrar const&) = delete;
};

}
} // namespace mir

#endif // MIR_SURFACES_INPUT_REGISTRAR_H_
