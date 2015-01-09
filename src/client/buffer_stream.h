/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
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

#ifndef MIR_CLIENT_BUFFER_STREAM_H
#define MIR_CLIENT_BUFFER_STREAM_H

#include "mir_protobuf.pb.h"

#include "mir_wait_handle.h"
#include "client_buffer_depository.h"

#include "mir_toolkit/client_types.h"

#include <EGL/eglplatform.h>

#include <memory>

namespace mir
{
namespace client
{
class ClientBufferFactory;
class ClientBuffer;
class EGLNativeWindowFactory;
struct MemoryRegion;

enum BufferStreamMode
{
Producer, // Like surfaces
Consumer // Like screencasts
};

class BufferStream;
typedef void (*mir_buffer_stream_callback)(
    BufferStream*, void*);

// TODO: Inherits from client surface
class BufferStream
{
public:
    BufferStream(mir::protobuf::DisplayServer& server,
        BufferStreamMode mode,
        std::shared_ptr<ClientBufferFactory> const& buffer_factory,
        std::shared_ptr<EGLNativeWindowFactory> const& native_window_factory,
                 protobuf::BufferStream const& protobuf_bs);
    virtual ~BufferStream();
    
    MirWaitHandle* next_buffer(mir_buffer_stream_callback callback, void* context);
    MirSurfaceParameters get_parameters();
    std::shared_ptr<mir::client::ClientBuffer> get_current_buffer();

    EGLNativeWindowType egl_native_window();
    std::shared_ptr<MemoryRegion> secure_for_cpu_write();

protected:
    BufferStream(BufferStream const&) = delete;
    BufferStream& operator=(BufferStream const&) = delete;

private:
    void process_buffer(protobuf::Buffer const& buffer);

    mir::protobuf::DisplayServer& display_server;

    BufferStreamMode const mode;
    std::shared_ptr<EGLNativeWindowFactory> const native_window_factory;
    
    mir::protobuf::BufferStream protobuf_bs;
    mir::client::ClientBufferDepository buffer_depository;
};

}
}

#endif // MIR_CLIENT_BUFFER_STREAM_H
