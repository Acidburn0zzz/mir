/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Cemil Azizoglu <cemil.azizoglu@canonical.com>
 */

#include "input_device.h"

#include "mir/input/pointer_settings.h"
#include "mir/input/input_device_info.h"
#include "mir/input/device_capability.h"

namespace mi = mir::input;
namespace mix = mi::X;

mix::XInputDevice::XInputDevice(InputDeviceInfo const& device_info)
    : info(device_info)
{
}

void mix::XInputDevice::start(InputSink* input_sink, EventBuilder* event_builder)
{
    sink = input_sink;
    builder = event_builder;
}

void mix::XInputDevice::stop()
{
    sink = nullptr;
    builder = nullptr;
}

mi::InputDeviceInfo mix::XInputDevice::get_device_info()
{
    return info;
}

mir::UniqueModulePtr<mi::PointerSettings> mix::XInputDevice::get_pointer_settings() const
{
    UniqueModulePtr<PointerSettings> ret;
    if (!contains(info.capabilities, DeviceCapability::pointer))
        return ret;

    return make_module_ptr<PointerSettings>();
}

void mix::XInputDevice::apply_settings(PointerSettings const&)
{
}
