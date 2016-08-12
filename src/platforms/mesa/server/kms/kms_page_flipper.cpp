/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "kms_page_flipper.h"
#include "mir/graphics/display_report.h"

#include <stdexcept>
#include <boost/throw_exception.hpp>
#include <boost/exception/errinfo_errno.hpp>

#include <xf86drm.h>
#include <xf86drmMode.h>

namespace mg = mir::graphics;
namespace mgm = mir::graphics::mesa;

namespace
{

void page_flip_handler(int /*fd*/, unsigned int seq,
                       unsigned int sec, unsigned int usec,
                       void* data)
{
    auto page_flip_data = static_cast<mgm::PageFlipEventData*>(data);
    page_flip_data->flipper->notify_page_flip(page_flip_data->crtc_id,
                                              seq, sec*1000000ULL + usec);
}

}

mgm::KMSPageFlipper::KMSPageFlipper(
    int drm_fd,
    std::shared_ptr<DisplayReport> const& report) :
    drm_fd{drm_fd},
    report{report},
    pending_page_flips(),
    worker_tid()
{
    uint64_t mono = 0;
    if (drmGetCap(drm_fd, DRM_CAP_TIMESTAMP_MONOTONIC, &mono) || !mono)
        clock_id = CLOCK_REALTIME;
    else
        clock_id = CLOCK_MONOTONIC;
}

bool mgm::KMSPageFlipper::schedule_flip(uint32_t crtc_id, uint32_t fb_id)
{
    std::unique_lock<std::mutex> lock{pf_mutex};

    if (pending_page_flips.find(crtc_id) != pending_page_flips.end())
        BOOST_THROW_EXCEPTION(std::logic_error("Page flip for crtc_id is already scheduled"));

    pending_page_flips[crtc_id] = PageFlipEventData{crtc_id, this, clock_id};

    auto ret = drmModePageFlip(drm_fd, crtc_id, fb_id,
                               DRM_MODE_PAGE_FLIP_EVENT,
                               &pending_page_flips[crtc_id]);

    if (ret)
        pending_page_flips.erase(crtc_id);

    return (ret == 0);
}

mg::Frame mgm::KMSPageFlipper::wait_for_flip(uint32_t crtc_id)
{
    static drmEventContext evctx =
    {
        DRM_EVENT_CONTEXT_VERSION,  /* .version */
        0,  /* .vblank_handler */
        page_flip_handler  /* .page_flip_handler */
    };
    static std::thread::id const invalid_tid;

    {
        std::unique_lock<std::mutex> lock{pf_mutex};

        /*
         * While another thread is the worker (it is controlling the
         * page flip event loop) and our event has not arrived, wait.
         */
        while (worker_tid != invalid_tid && !page_flip_is_done(crtc_id))
            pf_cv.wait(lock);

        /* If the page flip we are waiting for has arrived we are done. */
        if (page_flip_is_done(crtc_id))
            return completed_page_flips[crtc_id];

        /* ...otherwise we become the worker */
        worker_tid = std::this_thread::get_id();
    }

    /* Only the worker thread reaches this point */
    bool done{false};

    while (!done)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(drm_fd, &fds);

        /*
         * Wait for a page flip event. When we get a page flip event,
         * page_flip_handler(), called through drmHandleEvent(), will update
         * the pending_page_flips map.
         */
        auto ret = select(drm_fd + 1, &fds, nullptr, nullptr, nullptr);

        {
            std::unique_lock<std::mutex> lock{pf_mutex};

            if (ret > 0)
            {
                drmHandleEvent(drm_fd, &evctx);
            }
            else if (ret < 0 && errno != EINTR)
            {
                std::string const msg("Error while waiting for page-flip event");
                BOOST_THROW_EXCEPTION(
                    boost::enable_error_info(
                        std::runtime_error(msg)) << boost::errinfo_errno(errno));
            }

            done = page_flip_is_done(crtc_id);
            /* Give up loop control if we are done */
            if (done)
                worker_tid = invalid_tid;
        }

        /*
         * Wake up other (non-worker) threads, so they can check whether
         * their page-flip events have arrived, or whether they can become
         * the worker (see pf_cv.wait(lock) above).
         */
        pf_cv.notify_all();
    }
    return completed_page_flips[crtc_id];
}

std::thread::id mgm::KMSPageFlipper::debug_get_worker_tid()
{
    std::unique_lock<std::mutex> lock{pf_mutex};

    return worker_tid;
}

/* This method should be called with the 'pf_mutex' locked */
bool mgm::KMSPageFlipper::page_flip_is_done(uint32_t crtc_id)
{
    return pending_page_flips.find(crtc_id) == pending_page_flips.end();
}

void mgm::KMSPageFlipper::notify_page_flip(uint32_t crtc_id, uint64_t msc,
                                           uint64_t ust)
{
    report->report_vsync(crtc_id);
    Frame& frame = completed_page_flips[crtc_id];
    /*
     * Estimate min_ust_interval based on previous flip. The KMSOutput
     * class must improve on this estimate using its knowledge of the current
     * display mode, so that we don't accidentally stop adaptive frame rate
     * technologies like GSync/FreeSync from working.
     */
    if (frame.ust && msc > frame.msc)
        frame.min_ust_interval = (ust - frame.ust)/(msc - frame.msc);
    frame.msc = msc;
    frame.ust = ust;
    frame.clock_id = clock_id;
    pending_page_flips.erase(crtc_id);
}
