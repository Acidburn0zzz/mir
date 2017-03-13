/*
 * Copyright © 2016 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "platform.h"
#include "host_connection.h"
#include "display.h"
#include "buffer.h"
#include "native_buffer.h"
#include "ipc_operations.h"
#include "mir/shared_library.h"
#include "mir/graphics/graphic_buffer_allocator.h"
#include "mir/graphics/display.h"
#include "mir/graphics/platform_ipc_operations.h"
#include "mir/options/option.h"
#include "mir/options/configuration.h"
#include "mir/graphics/buffer_ipc_message.h"
#include "mir/graphics/platform_ipc_operations.h"
#include "mir/graphics/platform_operation_message.h"

namespace mg = mir::graphics;
namespace mgn = mir::graphics::nested;
namespace mo = mir::options;

namespace
{
mgn::PassthroughOption passthrough_from_options(mo::Option const& options)
{
    auto enabled = options.is_set(mo::nested_passthrough_opt) &&
        options.get<bool>(mo::nested_passthrough_opt);
    return enabled ? mgn::PassthroughOption::enabled : mgn::PassthroughOption::disabled;
}
}

mgn::NestedBufferPlatform::NestedBufferPlatform(
    std::shared_ptr<mir::SharedLibrary> const& library, 
    std::shared_ptr<mgn::HostConnection> const& connection, 
    std::shared_ptr<mg::DisplayReport> const& display_report,
    std::shared_ptr<mo::Option> const& options) :
    library(library),
    connection(connection),
    display_report(display_report),
    rendering_platform(library->load_function<mg::CreateRenderingPlatform>(
        "create_rendering_platform", MIR_SERVER_GRAPHICS_PLATFORM_VERSION)(options, connection)),
    passthrough_option(passthrough_from_options(*options))
{
}

namespace
{
class BufferAllocator : public mg::GraphicBufferAllocator
{
public:
    BufferAllocator(
        std::shared_ptr<mgn::HostConnection> const& connection,
        std::shared_ptr<mg::GraphicBufferAllocator> const& guest_allocator) :
        connection(connection),
        guest_allocator(guest_allocator)
    {
    }

    std::shared_ptr<mg::Buffer> alloc_buffer(mg::BufferProperties const& properties) override
    {
        return guest_allocator->alloc_buffer(properties);
    }

    std::shared_ptr<mg::Buffer> alloc_buffer(
        mir::geometry::Size size, uint32_t native_format, uint32_t native_flags) override
    {
        if (passthrough_candidate(size, mg::BufferUsage::hardware))
            return std::make_shared<mgn::Buffer>(connection, size, native_format, native_flags);
        else
            return guest_allocator->alloc_buffer(size, native_format, native_flags);
    }

    std::shared_ptr<mg::Buffer> alloc_software_buffer(mir::geometry::Size size, MirPixelFormat format) override
    {
        if (passthrough_candidate(size, mg::BufferUsage::software))
            return std::make_shared<mgn::Buffer>(connection, size, format);
        else
            return guest_allocator->alloc_software_buffer(size, format);
    }

    std::vector<MirPixelFormat> supported_pixel_formats() override
    {
        return guest_allocator->supported_pixel_formats();
    }

private:
    bool passthrough_candidate(mir::geometry::Size size, mg::BufferUsage usage)
    {
        return connection->supports_passthrough(usage) &&
            (size.width >= mir::geometry::Width{480}) && (size.height >= mir::geometry::Height{480});
    }
    std::shared_ptr<mgn::HostConnection> const connection;
    std::shared_ptr<mg::GraphicBufferAllocator> const guest_allocator;
};
}

mir::UniqueModulePtr<mg::GraphicBufferAllocator> mgn::NestedBufferPlatform::create_buffer_allocator()
{
    if (connection->supports_passthrough(mg::BufferUsage::software) ||
        connection->supports_passthrough(mg::BufferUsage::hardware))
    {
        return mir::make_module_ptr<BufferAllocator>(connection, rendering_platform->create_buffer_allocator());
    }
    else
    {
        return rendering_platform->create_buffer_allocator();
    }
}

mir::UniqueModulePtr<mg::PlatformIpcOperations> mgn::NestedBufferPlatform::make_ipc_operations() const
{
    return mir::make_module_ptr<mgn::IpcOperations>(rendering_platform->make_ipc_operations());
}

mgn::NestedDisplayPlatform::NestedDisplayPlatform(
    std::shared_ptr<mgn::HostConnection> const& connection, 
    std::shared_ptr<mg::DisplayReport> const& display_report,
    mo::Option const& options) :
    connection(connection),
    display_report(display_report),
    passthrough_option(passthrough_from_options(options))
{
}

mir::UniqueModulePtr<mg::Display> mgn::NestedDisplayPlatform::create_display(
    std::shared_ptr<mg::DisplayConfigurationPolicy> const& policy,
    std::shared_ptr<mg::GLConfig> const& config)
{
    return mir::make_module_ptr<mgn::Display>(
        connection,
        display_report,
        policy,
        config,
        passthrough_option);
}

mgn::Platform::Platform(
    std::unique_ptr<mgn::NestedBufferPlatform> buffer_platform,
    std::unique_ptr<mgn::NestedDisplayPlatform> display_platform) :
    buffer_platform{std::move(buffer_platform)},
    display_platform{std::move(display_platform)}
{
}

mir::UniqueModulePtr<mg::Display> mgn::Platform::create_display(
    std::shared_ptr<mg::DisplayConfigurationPolicy> const& initial_conf_policy,
    std::shared_ptr<mg::GLConfig> const& gl_config)
{
    return display_platform->create_display(initial_conf_policy, gl_config);
}
 
mir::UniqueModulePtr<mg::GraphicBufferAllocator> mgn::Platform::create_buffer_allocator()
{
    return buffer_platform->create_buffer_allocator();
}

mir::UniqueModulePtr<mg::PlatformIpcOperations> mgn::Platform::make_ipc_operations() const
{
    return buffer_platform->make_ipc_operations();
}
