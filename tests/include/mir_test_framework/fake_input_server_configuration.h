/*
 * Copyright © 2013-2014 Canonical Ltd.
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
 * Authored by: Robert Carr <robert.carr@canonical.com>
 *              Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef MIR_TEST_FAKE_INPUT_SERVER_CONFIGURATION_H_
#define MIR_TEST_FAKE_INPUT_SERVER_CONFIGURATION_H_

#include "mir_test_framework/testing_server_configuration.h"
#include "mir_test_framework/temporary_environment_value.h"
#include "mir_test_framework/executable_path.h"

namespace mir_test_framework
{

class FakeInputServerConfiguration : public TestingServerConfiguration
{
public:
    FakeInputServerConfiguration();
    FakeInputServerConfiguration(std::vector<mir::geometry::Rectangle> const& display_rects);

    std::shared_ptr<mir::input::InputManager> the_input_manager() override;
    std::shared_ptr<mir::shell::InputTargeter> the_input_targeter() override;

    // TODO remove reliance on legacy window management
    auto the_window_manager_builder() -> shell::WindowManagerBuilder override;

private:
    TemporaryEnvironmentValue input_lib{"MIR_SERVER_PLATFORM_INPUT_LIB", server_platform("input-stub.so").c_str()};
    TemporaryEnvironmentValue real_input{"MIR_SERVER_TESTS_USE_REAL_INPUT", "1"};
};

}

#endif /* MIR_TEST_FAKE_INPUT_SERVER_CONFIGURATION_H_ */
