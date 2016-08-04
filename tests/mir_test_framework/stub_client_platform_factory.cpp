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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "mir_test_framework/stub_client_platform_factory.h"
#include "mir/test/doubles/stub_client_buffer_factory.h"
#include "mir/client_buffer_factory.h"
#include "mir/client_buffer.h"
#include "mir/client_context.h"
#if defined(MESA_KMS) || defined(MESA_X11)
#include "src/platforms/mesa/include/native_buffer.h"
#else
#include "src/platforms/android/include/native_buffer.h"
#endif

#include <unistd.h>
#include <string.h>

namespace mcl = mir::client;
namespace geom = mir::geometry;
namespace mtf = mir_test_framework;
namespace mtd = mir::test::doubles;

mtf::StubClientPlatform::StubClientPlatform(mir::client::ClientContext* context)
        : context{context}
{
}

MirPlatformType mtf::StubClientPlatform::platform_type() const
{
    return mir_platform_type_gbm;
}

void mtf::StubClientPlatform::populate(MirPlatformPackage& package) const
{
    context->populate_server_package(package);
}

MirPlatformMessage* mtf::StubClientPlatform::platform_operation(MirPlatformMessage const*)
{
    return nullptr;
}

std::shared_ptr<mir::client::ClientBufferFactory> mtf::StubClientPlatform::create_buffer_factory()
{
    return std::make_shared<mtd::StubClientBufferFactory>();
}

std::shared_ptr<void> mtf::StubClientPlatform::create_egl_native_window(mir::client::EGLNativeSurface* surface)
{
    return std::shared_ptr<void>{surface, [](void*){}};
}

std::shared_ptr<EGLNativeDisplayType> mtf::StubClientPlatform::create_egl_native_display()
{
    auto fake_display = reinterpret_cast<EGLNativeDisplayType>(0x12345678lu);
    return std::make_shared<EGLNativeDisplayType>(fake_display);
}

MirNativeBuffer* mtf::StubClientPlatform::convert_native_buffer(mir::graphics::NativeBuffer* buf) const
{
    static_cast<void>(buf);
#if defined(MESA_KMS) || defined(MESA_X11)
    return buf;
#else
    return nullptr;
#endif
}

MirPixelFormat mtf::StubClientPlatform::get_egl_pixel_format(EGLDisplay, EGLConfig) const
{
    return mir_pixel_format_argb_8888;
}


std::shared_ptr<mcl::ClientPlatform>
mtf::StubClientPlatformFactory::create_client_platform(mcl::ClientContext* context)
{
    return std::make_shared<StubClientPlatform>(context);
}
