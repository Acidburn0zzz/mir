/*
 * Copyright © 2016 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir_toolkit/mir_presentation_chain.h"
#include "mir_toolkit/mir_buffer.h"
#include "presentation_chain.h"
#include "mir_connection.h"
#include "buffer.h"
#include "mir/require.h"
#include "mir/uncaught.h"
#include "mir/require.h"
#include "mir/client_buffer.h"
#include <stdexcept>
#include <boost/throw_exception.hpp>

namespace mcl = mir::client;

//private NBS api under development
void mir_connection_allocate_buffer(
    MirConnection* connection, 
    int width, int height,
    MirPixelFormat format,
    MirBufferUsage usage,
    mir_buffer_callback cb, void* context)
try
{
    mir::require(connection);
    connection->allocate_buffer(mir::geometry::Size{width, height}, format, usage, cb, context);
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
}

void mir_buffer_release(MirBuffer* b) 
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    auto connection = buffer->allocating_connection();
    connection->release_buffer(buffer);
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
}

MirNativeFence mir_buffer_get_fence(MirBuffer* b)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->get_fence();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return nullptr;
}

void mir_buffer_associate_fence(MirBuffer* b, MirNativeFence fence, MirBufferAccess access)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    buffer->set_fence(fence, access);
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
}

int mir_buffer_wait_for_access(MirBuffer* b, MirBufferAccess access, int timeout)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
     
    return buffer->wait_fence(access, std::chrono::nanoseconds(timeout)) ? 0 : -1;
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return -1;
}

MirNativeBuffer* mir_buffer_get_native_buffer(MirBuffer* b, MirBufferAccess access) 
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    if (!buffer->wait_fence(access, std::chrono::nanoseconds(-1)))
        BOOST_THROW_EXCEPTION(std::runtime_error("error accessing MirNativeBuffer"));
    return buffer->as_mir_native_buffer();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return nullptr;
}

MirGraphicsRegion mir_buffer_get_graphics_region(MirBuffer* b, MirBufferAccess access)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    if (!buffer->wait_fence(access, std::chrono::nanoseconds(-1)))
        BOOST_THROW_EXCEPTION(std::runtime_error("error accessing MirNativeBuffer"));

    return buffer->map_region();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return MirGraphicsRegion { 0, 0, 0, mir_pixel_format_invalid, nullptr };
}

unsigned int mir_buffer_get_width(MirBuffer* b)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->size().width.as_uint32_t();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return 0;
}

unsigned int mir_buffer_get_height(MirBuffer* b)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->size().height.as_uint32_t();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return 0;
}

MirPixelFormat mir_buffer_get_pixel_format(MirBuffer* b)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->pixel_format();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return mir_pixel_format_invalid;
}

MirBufferUsage mir_buffer_get_buffer_usage(MirBuffer* b)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->buffer_usage();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return mir_buffer_usage_hardware;
}

bool mir_buffer_is_valid(MirBuffer* b)
try
{
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->valid();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return false;
}

char const *mir_buffer_get_error_message(MirBuffer* b)
try
{
    auto buffer = reinterpret_cast<mcl::MirBuffer*>(b);
    return buffer->error_message();
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
    return "MirBuffer: unknown error";
}

void mir_buffer_set_callback(
    MirBuffer* b, mir_buffer_callback available_callback, void* available_context)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::Buffer*>(b);
    buffer->set_callback(available_callback, available_context);
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
}

void mir_buffer_get_egl_image(MirBuffer* b, char const* ext, EGLenum* type, EGLClientBuffer* bu, EGLint** attr)
try
{
    mir::require(b);
    auto buffer = reinterpret_cast<mcl::Buffer*>(b);
    buffer->client_buffer()->egl_image(ext, type, bu, attr);
}
catch (std::exception const& ex)
{
    MIR_LOG_UNCAUGHT_EXCEPTION(ex);
}
