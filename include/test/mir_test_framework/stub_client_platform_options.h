/*
 * Copyright © 2017 Canonical Ltd.
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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#ifndef MIR_TEST_FRAMEWORK_STUB_CLIENT_PLATFORM_OPTIONS_H_
#define MIR_TEST_FRAMEWORK_STUB_CLIENT_PLATFORM_OPTIONS_H_

#include <exception>

namespace mir_test_framework
{
enum class FailurePoint
{
    create_client_platform,
    create_egl_native_window,
    create_buffer_factory
};

/**
 *
 *
 * \param [in] where
 * \param [in] what
 */
void add_client_platform_error(FailurePoint where, std::exception_ptr what);
}

#endif //MIR_TEST_FRAMEWORK_STUB_CLIENT_PLATFORM_OPTIONS_H_
