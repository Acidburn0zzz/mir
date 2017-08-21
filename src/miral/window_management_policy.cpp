/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY {} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "miral/window_management_policy.h"
#include "both_versions.h"

void miral::WindowManagementPolicy::advise_begin() {}
void miral::WindowManagementPolicy::advise_end() {}
void miral::WindowManagementPolicy::advise_new_app(ApplicationInfo& /*application*/) {}
void miral::WindowManagementPolicy::advise_delete_app(ApplicationInfo const& /*application*/) {}
void miral::WindowManagementPolicy::advise_new_window(WindowInfo const& /*window_info*/) {}
void miral::WindowManagementPolicy::advise_focus_lost(WindowInfo const& /*info*/) {}
void miral::WindowManagementPolicy::advise_focus_gained(WindowInfo const& /*info*/) {}

MIRAL_BOTH_VERSIONS(
    _ZN5miral22WindowManagementPolicy19advise_state_changeERKNS_10WindowInfoE15MirSurfaceState,
    _ZN5miral22WindowManagementPolicy19advise_state_changeERKNS_10WindowInfoE14MirWindowState)
void miral::WindowManagementPolicy::advise_state_change(WindowInfo const& /*window_info*/, MirWindowState /*state*/) {}
void miral::WindowManagementPolicy::advise_move_to(WindowInfo const& /*window_info*/, Point /*top_left*/) {}
void miral::WindowManagementPolicy::advise_resize(WindowInfo const& /*window_info*/, Size const& /*new_size*/) {}
void miral::WindowManagementPolicy::advise_delete_window(WindowInfo const& /*window_info*/) {}
void miral::WindowManagementPolicy::advise_raise(std::vector<Window> const& /*windows*/) {}
