/*
 * Copyright © 2013 Canonical Ltd.
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
 */

#ifndef MIR_GRAPHICS_ANDROID_ANDROID_DISPLAY_CONFIGURATION_H_
#define MIR_GRAPHICS_ANDROID_ANDROID_DISPLAY_CONFIGURATION_H_

#include "mir/graphics/display_configuration.h"

namespace mir
{
namespace graphics
{
namespace android
{

class AndroidDisplayConfiguration : public graphics::DisplayConfiguration
{
public:
    AndroidDisplayConfiguration(geometry::Size const& display_size);
    AndroidDisplayConfiguration(AndroidDisplayConfiguration const& other);
    AndroidDisplayConfiguration& operator=(AndroidDisplayConfiguration const& other);

    virtual ~AndroidDisplayConfiguration() = default;

    void for_each_card(std::function<void(DisplayConfigurationCard const&)> f) const;
    void for_each_output(std::function<void(DisplayConfigurationOutput const&)> f) const;
    void configure_output(DisplayConfigurationOutputId, bool, geometry::Point, size_t, MirPowerMode power_mode);

private:
    DisplayConfigurationOutput configuration;
    DisplayConfigurationCard card;
};


}
}
}
#endif /* MIR_GRAPHICS_ANDROID_ANDROID_DISPLAY_CONFIGURATION_H_ */
