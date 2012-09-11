/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/graphics/android/android_buffer.h"
#include "mir/graphics/android/android_alloc_adaptor.h"

#include "mir_test/mock_android_alloc_device.h"

#include <gtest/gtest.h>

class AndroidBufferIntegration : public ::testing::Test
{
public:
    virtual void SetUp()
    {

    }
};


TEST_F(AndroidBufferIntegration, alloc_does_not_throw)
{
    pf = mc::PixelFormat::rgba_8888;
    width = geom::Width(300);
    usage = mga::BufferUsage::use_hardware;
    height = geom::Height(200);
    
    auto mock_alloc_device = std::shared_ptr<MockAllocDevice> (new MockAllocDevice(&native_handle));
    auto alloc_adaptor = std::make_shared<mga::AndroidAllocAdaptor>(mock_alloc_device);

    mc::BufferAllocationStrategy = std::make_shared<mc::DoubleBufferAllocationStrategy(alloc_adaptor);
}
