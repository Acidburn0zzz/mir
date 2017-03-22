/*
 * Copyright © 2017 Canonical Ltd.
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

#include "gbm_platform.h"
#include "mir/graphics/platform_authentication.h"
#include "buffer_allocator.h"
#include "ipc_operations.h"
#include "nested_authentication.h"
#include <boost/throw_exception.hpp>

namespace mg = mir::graphics;
namespace mgm = mir::graphics::mesa;

mgm::GBMPlatform::GBMPlatform(
    BypassOption bypass_option,
    BufferImportMethod import_method,
    std::shared_ptr<mg::PlatformAuthentication> const& platform_authentication) :
    bypass_option(bypass_option),
    import_method(import_method),
    platform_authentication(platform_authentication),
    gbm{std::make_shared<mgm::helpers::GBMHelper>()},
    auth{std::make_shared<mgm::NestedAuthentication>(platform_authentication)}
{
    auto master = platform_authentication->drm_fd();
    auto auth = platform_authentication->auth_extension();
    if (master.is_set())
    {
        gbm->setup(master.value());
    }
    else if (auth.is_set())
    {
        gbm->setup(auth.value()->auth_fd());
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::logic_error("no authentication fd to make gbm buffers"));
    }
}

mgm::GBMPlatform::GBMPlatform(
    BypassOption bypass_option,
    BufferImportMethod import_method,
    std::shared_ptr<mir::udev::Context> const& udev,
    std::shared_ptr<mgm::helpers::DRMHelper> const& drm) :
    bypass_option(bypass_option),
    import_method(import_method),
    udev(udev),
    drm(drm),
    gbm{std::make_shared<mgm::helpers::GBMHelper>()},
    auth{drm}
{
    gbm->setup(*drm);
}

mir::UniqueModulePtr<mg::GraphicBufferAllocator> mgm::GBMPlatform::create_buffer_allocator()
{
    return make_module_ptr<mgm::BufferAllocator>(gbm->device, bypass_option, import_method);
}

mir::UniqueModulePtr<mg::PlatformIpcOperations> mgm::GBMPlatform::make_ipc_operations() const
{
    return make_module_ptr<mg::mesa::IpcOperations>(auth);
}

mg::NativePlatform* mgm::GBMPlatform::native_platform()
{
    return this;
}

EGLNativeDisplayType mgm::GBMPlatform::egl_native_display() const
{
    return gbm->device;
}
