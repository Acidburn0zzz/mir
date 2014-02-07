/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "compositing_screencast.h"
#include "screencast_display_buffer.h"
#include "mir/graphics/buffer.h"
#include "mir/graphics/buffer_properties.h"
#include "mir/graphics/display.h"
#include "mir/graphics/display_buffer.h"
#include "mir/graphics/gl_context.h"
#include "mir/graphics/graphic_buffer_allocator.h"
#include "mir/compositor/display_buffer_compositor_factory.h"
#include "mir/compositor/display_buffer_compositor.h"
#include "mir/raii.h"

#include <boost/throw_exception.hpp>

namespace mc = mir::compositor;
namespace mf = mir::frontend;
namespace mg = mir::graphics;
namespace geom = mir::geometry;

namespace
{
uint32_t const max_screencast_sessions{100};
}

struct mc::detail::ScreencastSessionContext
{
    std::shared_ptr<graphics::Buffer> buffer;
    std::unique_ptr<graphics::GLContext> gl_context;
    std::unique_ptr<graphics::DisplayBuffer> display_buffer;
    std::unique_ptr<compositor::DisplayBufferCompositor> display_buffer_compositor;
};


mc::CompositingScreencast::CompositingScreencast(
    std::shared_ptr<mg::Display> const& display,
    std::shared_ptr<mg::GraphicBufferAllocator> const& buffer_allocator,
    std::shared_ptr<DisplayBufferCompositorFactory> const& db_compositor_factory)
    : display{display},
      buffer_allocator{buffer_allocator},
      db_compositor_factory{db_compositor_factory}
{
}

mf::ScreencastSessionId mc::CompositingScreencast::create_session(
    graphics::DisplayConfigurationOutputId output_id)
{
    geom::Rectangle extents;
    MirPixelFormat pixel_format;
    std::tie(extents,pixel_format) = output_info_for(output_id);

    std::lock_guard<decltype(session_mutex)> lock{session_mutex};
    auto const id = next_available_session_id();
    session_contexts[id] = create_session_context(extents, pixel_format);

    return id;
}

void mc::CompositingScreencast::destroy_session(mf::ScreencastSessionId id)
{
    std::lock_guard<decltype(session_mutex)> lock{session_mutex};
    auto gl_context = std::move(session_contexts.at(id)->gl_context);

    auto using_gl_context = mir::raii::paired_calls(
        [&] { gl_context->make_current(); },
        [&] { gl_context->release_current(); });

    session_contexts.erase(id);
}

std::shared_ptr<mg::Buffer> mc::CompositingScreencast::capture(mf::ScreencastSessionId id)
{
    std::shared_ptr<detail::ScreencastSessionContext> session_context;

    {
        std::lock_guard<decltype(session_mutex)> lock{session_mutex};
        session_context = session_contexts.at(id);
    }

    auto using_gl_context = mir::raii::paired_calls(
        [&] { session_context->gl_context->make_current(); },
        [&] { session_context->gl_context->release_current(); });

    session_context->display_buffer_compositor->composite();

    return session_context->buffer;
}

mf::ScreencastSessionId mc::CompositingScreencast::next_available_session_id()
{
    for (uint32_t i = 1; i <= max_screencast_sessions; ++i)
    {
        mf::ScreencastSessionId const id{i};
        if (session_contexts.find(id) == session_contexts.end())
            return id;
    }

    BOOST_THROW_EXCEPTION(std::runtime_error("Too many screencast sessions!"));
}

std::pair<geom::Rectangle,MirPixelFormat>
mc::CompositingScreencast::output_info_for(
    graphics::DisplayConfigurationOutputId output_id)
{
    auto const conf = display->configuration();
    geom::Rectangle extents;
    MirPixelFormat pixel_format{mir_pixel_format_invalid};

    conf->for_each_output(
        [&](mg::DisplayConfigurationOutput const& output)
        {
            if (output.id == output_id &&
                output.connected && output.used &&
                output.current_mode_index < output.modes.size())
            {
                extents = output.extents();
                pixel_format = output.current_format;
            }
        });

    if (extents == geom::Rectangle() ||
        pixel_format == mir_pixel_format_invalid)
    {
        BOOST_THROW_EXCEPTION(
            std::runtime_error("Invalid output for screen capture"));
    }

    return {extents, pixel_format};
}

std::shared_ptr<mc::detail::ScreencastSessionContext>
mc::CompositingScreencast::create_session_context(
    geometry::Rectangle const& rect,
    MirPixelFormat pixel_format)
{
    mg::BufferProperties buffer_properties{
        rect.size,
        pixel_format,
        mg::BufferUsage::hardware};

    auto gl_context = display->create_gl_context();
    auto gl_context_raw = gl_context.get();

    auto using_gl_context = mir::raii::paired_calls(
        [&] { gl_context_raw->make_current(); },
        [&] { gl_context_raw->release_current(); });

    auto buffer = buffer_allocator->alloc_buffer(buffer_properties);
    auto display_buffer = std::unique_ptr<ScreencastDisplayBuffer>(
        new ScreencastDisplayBuffer{rect, *buffer});
    auto db_compositor = db_compositor_factory->create_compositor_for(*display_buffer);

    return std::shared_ptr<detail::ScreencastSessionContext>(
        new detail::ScreencastSessionContext{
            buffer,
            std::move(gl_context),
            std::move(display_buffer),
            std::move(db_compositor)});
}
