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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_FRONTEND_UNAUTHORIZED_DISPLAY_CHANGER_H_
#define MIR_FRONTEND_UNAUTHORIZED_DISPLAY_CHANGER_H_

#include "mir/frontend/display_changer.h"

namespace mir
{
namespace frontend
{

class AuthorizingDisplayChanger : public frontend::DisplayChanger
{
public:
    AuthorizingDisplayChanger(
        std::shared_ptr<frontend::DisplayChanger> const& changer,
        bool configuration_is_authorized,
        bool base_configuration_modification_is_authorized);

    std::shared_ptr<graphics::DisplayConfiguration> base_configuration() override;
    void configure(
        std::shared_ptr<frontend::Session> const&,
        std::shared_ptr<graphics::DisplayConfiguration> const&) override;
    void set_base_configuration(
        std::shared_ptr<graphics::DisplayConfiguration> const&) override;

private:
    std::shared_ptr<frontend::DisplayChanger> const changer;

    bool const configure_display_is_allowed;
    bool const set_base_configuration_is_allowed;
};

}
}

#endif /* MIR_FRONTEND_UNAUTHORIZED_DISPLAY_CHANGER_H_ */
