/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#ifndef MIR_FRONTEND_WAYLAND_CONNECTOR_H_
#define MIR_FRONTEND_WAYLAND_CONNECTOR_H_

#include "mir/frontend/connector.h"
#include "mir/fd.h"

#include <wayland-server-core.h>
#include <thread>

namespace mir
{


namespace graphics
{
class RenderingPlatform;
class WaylandAllocator;
}

namespace frontend
{
class WlCompositor;
class WlApplication;
class WlShell;
class WlSeat;
class OutputManager;

class Shell;
class EventSink;
class DisplayChanger;

class WaylandConnector : public Connector
{
public:
    WaylandConnector(
        std::shared_ptr<Shell> const& shell,
        std::shared_ptr<EventSink> const& global_sink,
        DisplayChanger& display_config,
        std::shared_ptr<graphics::RenderingPlatform> const& platform);

    ~WaylandConnector() override;

    void start() override;
    void stop() override;

    int client_socket_fd() const override;

    int client_socket_fd(
        std::function<void(std::shared_ptr<Session> const& session)> const& connect_handler) const override;

private:
    std::unique_ptr<wl_display, void(*)(wl_display*)> const display;
    mir::Fd const pause_signal;
    std::unique_ptr<WlCompositor> compositor_global;
    std::unique_ptr<WlSeat> seat_global;
    std::unique_ptr<OutputManager> output_manager;
    std::shared_ptr<graphics::WaylandAllocator> const allocator;
    std::unique_ptr<WlShell> shell_global;
    std::thread dispatch_thread;
};


}
}

#endif // MIR_FRONTEND_WAYLAND_CONNECTOR_H_
