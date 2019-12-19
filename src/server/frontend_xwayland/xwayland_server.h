/*
 * Copyright (C) 2018 Marius Gripsgard <marius@ubports.com>
 * Copyright (C) 2019 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3,
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
 */

#ifndef MIR_FRONTEND_XWAYLAND_SERVER_H
#define MIR_FRONTEND_XWAYLAND_SERVER_H

#include <memory>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

namespace mir
{
namespace dispatch
{
class ReadableFd;
class ThreadedDispatcher;
class MultiplexingDispatchable;
} /*dispatch */
namespace frontend
{
class WaylandConnector;
class XWaylandWM;
class XWaylandServer
{
public:
    XWaylandServer(const int xdisp, std::shared_ptr<WaylandConnector> wc, std::string const& xwayland_path);
    ~XWaylandServer();

    enum Status {
      STARTING = 1,
      RUNNING = 2,
      STOPPED = -1,
      FAILED = -2
     };

private:
    /// Forks off the XWayland process
    void spawn();
    /// Called after fork() if we should turn into XWayland
    void execl_xwayland(int wl_client_client_fd, int wm_client_fd);
    /// Called after fork() if we should continue on as Mir
    void connect_to_xwayland(int wl_client_server_fd, int wm_server_fd);
    void new_spawn_thread();

    std::shared_ptr<XWaylandWM> const wm;
    std::shared_ptr<WaylandConnector> const wlc;
    std::shared_ptr<dispatch::MultiplexingDispatchable> const dispatcher;
    std::unique_ptr<dispatch::ThreadedDispatcher> const xserver_thread;
    struct SocketFd
    {
        int const xdisplay;
        int socket_fd;
        int abstract_socket_fd;

        SocketFd(int xdisplay);
        ~SocketFd();
    } const sockets;
    std::shared_ptr<dispatch::ReadableFd> const afd_dispatcher;
    std::shared_ptr<dispatch::ReadableFd> const fd_dispatcher;
    std::string const xwayland_path;

    Status xserver_status = STOPPED;
    std::thread spawn_thread;
    pid_t pid;
    bool terminate = false;
};
} /* frontend */
} /* mir */

#endif /* end of include guard: MIR_FRONTEND_XWAYLAND_SERVER_H */
