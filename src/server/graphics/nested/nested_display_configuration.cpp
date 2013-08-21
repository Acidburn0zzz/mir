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
 *
 * Authored by: Eleni Maria Stea <elenimaria.stea@canonical.com>
 */

#include "nested_display_configuration.h"

#include <algorithm>

namespace mgn = mir::graphics::nested;

mgn::NestedDisplayConfiguration::NestedDisplayConfiguration(MirConnection* connection) :
display_config{connection}
{
}

mgn::NestedDisplayConfiguration::~NestedDisplayConfiguration() noexcept
{
}

void mgn::NestedDisplayConfiguration::for_each_card(std::function<void(DisplayConfigurationCard const&)> f) const
{
    std::for_each(
        display_config->cards,
        display_config->cards+display_config->num_cards,
        [&f](MirDisplayCard const& mir_card)
        {
            f({
                static_cast<DisplayConfigurationCardId>(mir_card.card_id),
                mir_card.max_simultaneous_outputs});
        });
}

void mgn::NestedDisplayConfiguration::for_each_output(std::function<void(DisplayConfigurationOutput const&)> f) const
{
    std::for_each(
        display_config->outputs,
        display_config->outputs+display_config->num_outputs,
        [&f](MirDisplayOutput const& mir_output)
        {
            std::vector<geometry::PixelFormat> formats;
            formats.reserve(mir_output.num_output_formats);
            for (auto p = mir_output.output_formats; p != mir_output.output_formats+mir_output.num_output_formats; ++p)
                formats.push_back(geometry::PixelFormat(*p));

            std::vector<DisplayConfigurationMode> modes;
            modes.reserve(mir_output.num_modes);
            for (auto p = mir_output.modes; p != mir_output.modes+mir_output.num_modes; ++p)
                modes.push_back(DisplayConfigurationMode{{p->horizontal_resolution, p->vertical_resolution}, p->refresh_rate});

            DisplayConfigurationOutput const output{
                DisplayConfigurationOutputId(mir_output.output_id),
                DisplayConfigurationCardId(mir_output.card_id),
                DisplayConfigurationOutputType(mir_output.type),
                std::move(formats),
                std::move(modes),
                mir_output.preferred_mode,
                geometry::Size{mir_output.physical_width_mm, mir_output.physical_height_mm},
                !!mir_output.connected,
                !!mir_output.used,
                geometry::Point{mir_output.position_x, mir_output.position_y},
                mir_output.current_mode,
                mir_output.current_output_format
            };

            f(output);
        });
}

void mgn::NestedDisplayConfiguration::configure_output(DisplayConfigurationOutputId /*id*/, bool /*used*/, geometry::Point /*top_left*/, size_t /*mode_index*/)
{
    // TODO
}
