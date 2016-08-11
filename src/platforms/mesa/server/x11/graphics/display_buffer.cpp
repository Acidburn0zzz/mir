/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Cemil Azizoglu <cemil.azizoglu@canonical.com>
 *
 */

#include "mir/graphics/egl_error.h"
#include "display_buffer.h"
#include <cstring>
#include <boost/throw_exception.hpp>

namespace mg=mir::graphics;
namespace mgx=mg::X;
namespace geom=mir::geometry;

mgx::DisplayBuffer::DisplayBuffer(geom::Size const sz,
                                  EGLDisplay const d,
                                  EGLSurface const s,
                                  EGLContext const c,
                                  MirOrientation const o)
                                  : size{sz},
                                    egl_dpy{d},
                                    egl_surf{s},
                                    egl_ctx{c},
                                    orientation_{o}
{
#if 0
    // TODO: Replace this
    set_frame_callback([](Frame const& frame)
    {
        unsigned long long frame_seq = frame.msc;
        unsigned long long frame_usec = frame.ust;
        static unsigned long long prev = frame.ust;
        struct timespec now;
        clock_gettime(frame.clock_id, &now);
        unsigned long long now_usec = now.tv_sec*1000000ULL +
                                      now.tv_nsec/1000;
        long long age_usec = now_usec - frame_usec;
        fprintf(stderr, "TODO - Frame #%llu at %llu.%06llus (%lldus ago) delta %lldus\n",
                        frame_seq,
                        frame_usec/1000000ULL, frame_usec%1000000ULL,
                        age_usec, frame_usec - prev);
        prev = frame_usec;
    });
#endif

    /*
     * EGL_CHROMIUM_sync_control is not an official standard, but Google
     * invented it as a way to free Chromium from dependency on GLX
     * (GLX_OML_sync_control). And Mesa/Intel implemented the backend for it
     * (EGL on X11 only).
     */
    auto extensions = eglQueryString(egl_dpy, EGL_EXTENSIONS);
    eglGetSyncValues =
        reinterpret_cast<EglGetSyncValuesCHROMIUM*>(
            strstr(extensions, "EGL_CHROMIUM_sync_control") ?
            eglGetProcAddress("eglGetSyncValuesCHROMIUM") : NULL
            );
}

geom::Rectangle mgx::DisplayBuffer::view_area() const
{
    switch (orientation_)
    {
    case mir_orientation_left:
    case mir_orientation_right:
        return {{0,0}, {size.height.as_int(), size.width.as_int()}};
    default:
        return {{0,0}, size};
    }
}

void mgx::DisplayBuffer::make_current()
{
    if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx))
        BOOST_THROW_EXCEPTION(mg::egl_error("Cannot make current"));
}

void mgx::DisplayBuffer::release_current()
{
    if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
        BOOST_THROW_EXCEPTION(mg::egl_error("Cannot make uncurrent"));
}

bool mgx::DisplayBuffer::post_renderables_if_optimizable(RenderableList const& /*renderlist*/)
{
    return false;
}

void mgx::DisplayBuffer::swap_buffers()
{
    if (!eglSwapBuffers(egl_dpy, egl_surf))
        BOOST_THROW_EXCEPTION(mg::egl_error("Cannot swap"));

    if (eglGetSyncValues) // We allow for this to be missing because calling
    {                     // it may also fail, which needs handling too...
        uint64_t ust, msc, sbc;
        if (eglGetSyncValues(egl_dpy, egl_surf, &ust, &msc, &sbc))
        {
            auto delta = msc - last_frame_.msc;
            if (delta)
            {
                last_frame_.msc = msc;

                // Always monotonic? The Chromium source suggests no. But the
                // libdrm source says you can only find out with drmGetCap :(
                // This appears to be correct for all modern systems though...
                last_frame_.clock_id = CLOCK_MONOTONIC;

                // We might not see every physical frame here since we're at
                // a high level, so need to approximate prev_ust in the case
                // we missed the previous one or more frames...
                last_frame_.prev_ust = last_frame_.ust +
                                       (ust - last_frame_.ust) / delta;
                last_frame_.ust = ust;
            }
        }
    }
}

mg::Frame mgx::DisplayBuffer::last_frame() const
{
    return last_frame_;
}

void mgx::DisplayBuffer::bind()
{
}

MirOrientation mgx::DisplayBuffer::orientation() const
{
    return orientation_;
}

MirMirrorMode mgx::DisplayBuffer::mirror_mode() const
{
    return mir_mirror_mode_none;
}

void mgx::DisplayBuffer::set_orientation(MirOrientation const new_orientation)
{
    orientation_ = new_orientation;
}

mg::NativeDisplayBuffer* mgx::DisplayBuffer::native_display_buffer()
{
    return this;
}
