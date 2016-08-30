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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois<kevin.dubois@canonical.com>
 */

#include "mir_native_window.h"
#include "android_format_conversion-inl.h"
#include "mir/client_context.h"
#include "android_client_platform.h"
#include "gralloc_registrar.h"
#include "android_client_buffer_factory.h"
#include "egl_native_surface_interpreter.h"
#include "native_window_report.h"
#include "mir/logging/dumb_console_logger.h"

#include "mir/weak_egl.h"
#include <EGL/egl.h>

#include <boost/throw_exception.hpp>

namespace mcl=mir::client;
namespace mcla=mir::client::android;
namespace mga=mir::graphics::android;

mcla::AndroidClientPlatform::AndroidClientPlatform(
    ClientContext* const context)
    : context{context}
{
}

std::shared_ptr<mcl::ClientBufferFactory> mcla::AndroidClientPlatform::create_buffer_factory()
{
    const hw_module_t *hw_module;
    int error = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &hw_module);
    if (error < 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Could not open hardware module"));
    }

    gralloc_module_t* gr_dev = (gralloc_module_t*) hw_module;
    /* we use an empty deleter because hw_get_module does not give us the ownership of the ptr */
    auto gralloc_dev = std::shared_ptr<gralloc_module_t>(gr_dev, [](auto){});
    auto registrar = std::make_shared<mcla::GrallocRegistrar>(gralloc_dev);
    return std::make_shared<mcla::AndroidClientBufferFactory>(registrar);
}

std::shared_ptr<void> mcla::AndroidClientPlatform::create_egl_native_window(EGLNativeSurface *surface)
{
    auto log = getenv("MIR_CLIENT_ANDROID_WINDOW_REPORT");
    std::shared_ptr<mga::NativeWindowReport> report;
    char const* on_val = "log";
    if (log && !strncmp(log, on_val, strlen(on_val)))
        report = std::make_shared<mga::ConsoleNativeWindowReport>(
            std::make_shared<mir::logging::DumbConsoleLogger>());
    else
        report = std::make_shared<mga::NullNativeWindowReport>();
 
    return std::make_shared<mga::MirNativeWindow>(
        std::make_shared<mcla::EGLNativeSurfaceInterpreter>(*surface), report);
}

std::shared_ptr<EGLNativeDisplayType>
mcla::AndroidClientPlatform::create_egl_native_display()
{
    auto native_display = std::make_shared<EGLNativeDisplayType>();
    *native_display = EGL_DEFAULT_DISPLAY;
    return native_display;
}

MirPlatformType mcla::AndroidClientPlatform::platform_type() const
{
    return mir_platform_type_android;
}

void mcla::AndroidClientPlatform::populate(MirPlatformPackage& package) const
{
    context->populate_server_package(package);
}

MirPlatformMessage* mcla::AndroidClientPlatform::platform_operation(
    MirPlatformMessage const*)
{
    return nullptr;
}

MirNativeBuffer* mcla::AndroidClientPlatform::convert_native_buffer(graphics::NativeBuffer* buf) const
{
    return mga::to_native_buffer_checked(buf)->anwb();
}

MirPixelFormat mcla::AndroidClientPlatform::get_egl_pixel_format(
    EGLDisplay disp, EGLConfig conf) const
{
    MirPixelFormat mir_format = mir_pixel_format_invalid;

    // EGL_KHR_platform_android says this will always work...
    EGLint vis = 0;
    mcl::WeakEGL weak;
    if (weak.eglGetConfigAttrib(disp, conf, EGL_NATIVE_VISUAL_ID, &vis))
        mir_format = mir::graphics::android::to_mir_format(vis);

    return mir_format;
}
