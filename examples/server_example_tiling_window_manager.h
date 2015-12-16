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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_EXAMPLE_TILING_WINDOW_MANAGER_H_
#define MIR_EXAMPLE_TILING_WINDOW_MANAGER_H_

#include "server_example_basic_window_manager.h"

///\example server_example_tiling_window_manager.h
/// Demonstrate implementing a simple tiling algorithm

namespace mir
{
namespace examples
{
// simple tiling algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (two finger drag)
//  o Maximize/restore current window (to tile size): Alt-F11
//  o Maximize/restore current window (to tile height): Shift-F11
//  o Maximize/restore current window (to tile width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class TilingWindowManagerPolicy : public WindowManagementPolicy
{
public:
    explicit TilingWindowManagerPolicy(WindowManagerTools* const tools);

    void click(geometry::Point cursor);

    void handle_session_info_updated(SessionInfoMap& session_info, geometry::Rectangles const& displays);

    void handle_displays_updated(SessionInfoMap& session_info, geometry::Rectangles const& displays);

    void resize(geometry::Point cursor);

    auto handle_place_new_surface(
        std::shared_ptr<scene::Session> const& session,
        scene::SurfaceCreationParameters const& request_parameters)
    -> scene::SurfaceCreationParameters;

    void handle_new_surface(std::shared_ptr<scene::Session> const& session, std::shared_ptr<scene::Surface> const& surface);

    void handle_modify_surface(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        shell::SurfaceSpecification const& modifications);

    void handle_delete_surface(std::shared_ptr<scene::Session> const& session, std::weak_ptr<scene::Surface> const& surface);

    int handle_set_state(std::shared_ptr<scene::Surface> const& surface, MirSurfaceState value);

    void drag(geometry::Point cursor);

    bool handle_keyboard_event(MirKeyboardEvent const* event);

    bool handle_touch_event(MirTouchEvent const* event);

    bool handle_pointer_event(MirPointerEvent const* event);

    void handle_raise_surface(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface);

    void generate_decorations_for(
        std::shared_ptr<scene::Session> const& session, std::shared_ptr<scene::Surface> const& surface,
        SurfaceInfoMap& surface_info,
        std::function<frontend::SurfaceId(std::shared_ptr<scene::Session> const&, scene::SurfaceCreationParameters const&)> const& build);

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    void toggle(MirSurfaceState state);

    std::shared_ptr<scene::Session> session_under(geometry::Point position);

    void update_tiles(
        SessionInfoMap& session_info,
        geometry::Rectangles const& displays);

    void update_surfaces(std::weak_ptr<scene::Session> const& session, geometry::Rectangle const& old_tile, geometry::Rectangle const& new_tile);

    static void clip_to_tile(scene::SurfaceCreationParameters& parameters, geometry::Rectangle const& tile);

    static void fit_to_new_tile(scene::Surface& surface, geometry::Rectangle const& old_tile, geometry::Rectangle const& new_tile);

    void drag(std::shared_ptr<scene::Surface> surface, geometry::Point to, geometry::Point from, geometry::Rectangle bounds);

    static void resize(std::shared_ptr<scene::Surface> surface, geometry::Point cursor, geometry::Point old_cursor, geometry::Rectangle bounds);

    static void constrained_move(std::shared_ptr<scene::Surface> const& surface, geometry::Displacement& movement, geometry::Rectangle const& bounds);

    std::shared_ptr<scene::Surface> select_active_surface(std::shared_ptr<scene::Session> const& session, std::shared_ptr<scene::Surface> const& surface);

    WindowManagerTools* const tools;

    geometry::Point old_cursor{};
};

using TilingWindowManager = WindowManagerBuilder<TilingWindowManagerPolicy>;
}
}

#endif /* MIR_EXAMPLE_TILING_WINDOW_MANAGER_H_ */
