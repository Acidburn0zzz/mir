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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "android_hwc_factory.h"
#include "hwc11_device.h"
#include "hwc_layerlist.h"
#include "default_fb_device.h"

namespace mga=mir::graphics::android;

std::shared_ptr<mga::HWCDevice> mga::AndroidHWCFactory::create_hwc_1_1(
    std::shared_ptr<hwc_composer_device_1> const& hwc_device,
    std::shared_ptr<mga::FBDevice> const& fb_device) const
{
    auto layer_list = std::make_shared<mga::HWCLayerList>();
//    auto fb = std::make_shared<mga::DefaultFBDevice>(fb_device);
    return std::make_shared<mga::HWC11Device>(hwc_device, layer_list, fb_device);
}
