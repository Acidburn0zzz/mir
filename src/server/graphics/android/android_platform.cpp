/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "android_platform.h"
#include "android_buffer_allocator.h"
#include "android_display.h"
#include "mir/graphics/platform_ipc_package.h"
#include "android_display_selector.h"
#include "android_fb_factory.h"
#include "mir/compositor/buffer_id.h"


#include <boost/throw_exception.hpp>

#include <stdexcept>

namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace mc=mir::compositor;

//todo: the server open/closes the platform a few times, android can't handle this
namespace
{
std::shared_ptr<mg::Platform> global_platform;
}

mga::AndroidPlatform::AndroidPlatform(std::shared_ptr<mga::DisplaySelector> const& selector)
 : display_selector(selector)
{
}

std::shared_ptr<mc::GraphicBufferAllocator> mga::AndroidPlatform::create_buffer_allocator(
        const std::shared_ptr<mg::BufferInitializer>& /*buffer_initializer*/)
{
    return std::make_shared<mga::AndroidBufferAllocator>();
}

std::shared_ptr<mg::Display> mga::AndroidPlatform::create_display()
{
    return display_selector->primary_display();
}

std::shared_ptr<mg::PlatformIPCPackage> mga::AndroidPlatform::get_ipc_package()
{
    return std::make_shared<mg::PlatformIPCPackage>();
}

std::shared_ptr<mg::Platform> mg::create_platform(std::shared_ptr<DisplayReport> const& /*TODO*/)
{
    auto fb_factory = std::make_shared<mga::AndroidFBFactory>();
    auto selector = std::make_shared<mga::AndroidDisplaySelector>(fb_factory);
    if(!global_platform)
    {
        global_platform = std::make_shared<mga::AndroidPlatform>(selector);
    }
    return global_platform;
}
