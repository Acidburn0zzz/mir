/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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

#include "mir/glib_main_loop.h"
#include "mir_test/signal.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <thread>

namespace mt = mir::test;

struct GLibMainLoopTest : ::testing::Test
{
    mir::GLibMainLoop ml;
};

TEST_F(GLibMainLoopTest, stops_from_within_handler)
{
    int const owner{0};

    ml.enqueue(&owner, [&] { ml.stop(); });
    ml.run();
}

TEST_F(GLibMainLoopTest, stops_from_outside_handler)
{
    int const owner{0};
    mt::Signal loop_running;

    std::thread t{[&] { loop_running.wait(); ml.stop(); }};

    ml.enqueue(&owner, [&] { loop_running.raise(); });
    ml.run();

    t.join();
}

TEST_F(GLibMainLoopTest, ignores_handler_added_after_stop)
{
    int const owner{0};
    bool handler_called{false};
    mt::Signal loop_running;

    std::thread t{
        [&]
        {
            loop_running.wait();
            ml.stop();
            int const owner1{0};
            ml.enqueue(&owner1, [&] { handler_called = true; });
        }};

    ml.enqueue(&owner, [&] { loop_running.raise(); });
    ml.run();

    t.join();

    EXPECT_FALSE(handler_called);
}
