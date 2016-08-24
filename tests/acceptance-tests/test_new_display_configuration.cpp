/*
 * Copyright © 2013-2016 Canonical Ltd.
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
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 *              Kevin DuBois <kevin.dubois@canonical.com>
 *              Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "mir/main_loop.h"
#include "mir/frontend/session_authorizer.h"
#include "mir/graphics/event_handler_register.h"
#include "mir/shell/display_configuration_controller.h"

#include "mir_test_framework/connected_client_with_a_surface.h"
#include "mir/test/doubles/null_platform.h"
#include "mir/test/doubles/fake_display.h"
#include "mir/test/doubles/null_display_sync_group.h"
#include "mir/test/doubles/null_platform.h"
#include "mir/test/display_config_matchers.h"
#include "mir/test/doubles/stub_display_configuration.h"
#include "mir/test/doubles/stub_session_authorizer.h"
#include "mir/test/fake_shared.h"
#include "mir/test/pipe.h"
#include "mir/test/signal.h"
#include "mir/test/signal.h"

#include "mir_toolkit/mir_client_library.h"

#include <atomic>
#include <chrono>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace mg = mir::graphics;
namespace geom = mir::geometry;
namespace mf = mir::frontend;
namespace mtf = mir_test_framework;
namespace mtd = mir::test::doubles;
namespace mt = mir::test;

using namespace testing;
using namespace std::literals::chrono_literals;

namespace
{
mtd::StubDisplayConfig stub_display_config;

mtd::StubDisplayConfig changed_stub_display_config{1};

class MockDisplay : public mtd::FakeDisplay
{
public:
    MockDisplay(): mtd::FakeDisplay()
    {
        using namespace testing;
        ON_CALL(*this, configure(_))
            .WillByDefault(Invoke(
                [this](mg::DisplayConfiguration const& new_config)
                {
                    mtd::FakeDisplay::configure(new_config);
                }));
    }

    MOCK_METHOD1(configure, void(mg::DisplayConfiguration const&));
};

struct StubAuthorizer : mtd::StubSessionAuthorizer
{
    bool configure_display_is_allowed(mf::SessionCredentials const&) override
    {
        return allow_configure_display;
    }

    bool set_base_display_configuration_is_allowed(mf::SessionCredentials const&) override
    {
        return allow_set_base_display_configuration;
    }

    std::atomic<bool> allow_configure_display{true};
    std::atomic<bool> allow_set_base_display_configuration{true};
};

void wait_for_server_actions_to_finish(mir::ServerActionQueue& server_action_queue)
{
    mt::Signal last_action_done;
    server_action_queue.enqueue(
        &last_action_done,
        [&] { last_action_done.raise(); });

    last_action_done.wait_for(std::chrono::seconds{5});
}
}

struct DisplayConfigurationTest : mtf::ConnectedClientWithASurface
{
    void SetUp() override
    {
        server.override_the_session_authorizer([this] { return mt::fake_shared(stub_authorizer); });
        preset_display(mt::fake_shared(mock_display));
        mtf::ConnectedClientWithASurface::SetUp();
    }

    testing::NiceMock<MockDisplay> mock_display;
    StubAuthorizer stub_authorizer;
};

TEST_F(DisplayConfigurationTest, display_configuration_reaches_client)
{
    auto configuration = mir_connection_create_display_configuration(connection);

    EXPECT_THAT(configuration,
        mt::DisplayConfigMatches(std::cref(stub_display_config)));

    mir_display_config_release(configuration);
}

namespace
{
void display_change_handler(MirConnection* connection, void* context)
{
    auto configuration = mir_connection_create_display_configuration(connection);

    EXPECT_THAT(configuration,
                mt::DisplayConfigMatches(std::cref(changed_stub_display_config)));
    mir_display_config_release(configuration);

    auto callback_called = static_cast<mt::Signal*>(context);
    callback_called->raise();
}
}

TEST_F(DisplayConfigurationTest, hw_display_change_notification_reaches_all_clients)
{
    mt::Signal callback_called;

    mir_connection_set_display_config_change_callback(connection, &display_change_handler, &callback_called);

    MirConnection* unsubscribed_connection = mir_connect_sync(new_connection().c_str(), "notifier");

    mock_display.emit_configuration_change_event(
        mt::fake_shared(changed_stub_display_config));

    EXPECT_TRUE(callback_called.wait_for(std::chrono::seconds{10}));

    // At this point, the message has gone out on the wire. since with unsubscribed_connection
    // we're emulating a client that is passively subscribed, we will just wait for the display
    // configuration to change and then will check the new config.

    auto config = mir_connection_create_display_configuration(unsubscribed_connection);
    while((unsigned)mir_display_config_get_num_outputs(config) != changed_stub_display_config.outputs.size())
    {
        mir_display_config_release(config);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        config = mir_connection_create_display_configuration(unsubscribed_connection);
    }

    EXPECT_THAT(config,
        mt::DisplayConfigMatches(std::cref(changed_stub_display_config)));

    mir_display_config_release(config);

    mir_connection_release(unsubscribed_connection);
}

namespace
{
struct SimpleClient
{
    SimpleClient(std::string const& mir_test_socket) :
        mir_test_socket{mir_test_socket} {}

    void connect()
    {
        connection = mir_connect_sync(mir_test_socket.c_str(), __PRETTY_FUNCTION__);

        auto const spec = mir_connection_create_spec_for_normal_surface(connection, 100, 100, mir_pixel_format_abgr_8888);
        surface = mir_surface_create_sync(spec);
        mir_surface_spec_release(spec);
        mir_buffer_stream_swap_buffers_sync(mir_surface_get_buffer_stream(surface));
    }

    void disconnect()
    {
        mir_surface_release_sync(surface);
        mir_connection_release(connection);
    }

    void disconnect_without_releasing_surface()
    {
        mir_connection_release(connection);
    }

    std::string mir_test_socket;
    MirConnection* connection{nullptr};
    MirSurface* surface{nullptr};
};

struct DisplayClient : SimpleClient
{
    using SimpleClient::SimpleClient;

    std::unique_ptr<MirDisplayConfig,void(*)(MirDisplayConfig*)> get_base_config()
    {
        return {mir_connection_create_display_configuration(connection),
            &mir_display_config_release};
    }
};
}

namespace
{
struct DisplayConfigMatchingContext
{
    std::function<void(MirDisplayConfig*)> matcher;
    mt::Signal done;
};

void new_display_config_matches(MirConnection* connection, void* ctx)
{
    auto context = reinterpret_cast<DisplayConfigMatchingContext*>(ctx);

    auto config = mir_connection_create_display_configuration(connection);
    context->matcher(config);
    mir_display_config_release(config);
    context->done.raise();
}
}

TEST_F(DisplayConfigurationTest, shell_initiated_display_configuration_notifies_clients)
{
    using namespace testing;

    // Create a new client for explicit lifetime handling.
    SimpleClient client{new_connection()};

    client.connect();

    std::shared_ptr<mg::DisplayConfiguration> new_conf;
    new_conf = server.the_display()->configuration();

    new_conf->for_each_output([](mg::UserDisplayConfigurationOutput& output)
        {
            if (output.connected)
            {
                output.used = !output.used;
            }
        });

    DisplayConfigMatchingContext context;
    context.matcher = [new_conf](MirDisplayConfig* conf)
    {
        EXPECT_THAT(conf, mt::DisplayConfigMatches(std::cref(*new_conf)));
    };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    server.the_display_configuration_controller()->set_base_configuration(new_conf);

    EXPECT_TRUE(context.done.wait_for(std::chrono::seconds{10}));

    EXPECT_THAT(
        *server.the_display()->configuration(),
        mt::DisplayConfigMatches(std::cref(*new_conf)));

    client.disconnect();
}

struct DisplayPowerSetting : public DisplayConfigurationTest, public ::testing::WithParamInterface<MirPowerMode> {};
struct DisplayOrientationSetting : public DisplayConfigurationTest, public ::testing::WithParamInterface<MirOrientation> {};
struct DisplayFormatSetting : public DisplayConfigurationTest, public ::testing::WithParamInterface<MirPixelFormat> {};
struct DisplaySubpixelSetting : public DisplayConfigurationTest, public ::testing::WithParamInterface<MirSubpixelArrangement> {};

TEST_P(DisplayPowerSetting, can_get_power_mode)
{
    using namespace testing;

    auto mode = GetParam();

    std::shared_ptr<mg::DisplayConfiguration> server_config = server.the_display()->configuration();

    server_config->for_each_output(
        [mode](mg::UserDisplayConfigurationOutput& output)
        {
            output.power_mode = mode;
        });

    server.the_display_configuration_controller()->set_base_configuration(server_config);
    wait_for_server_actions_to_finish(*server.the_main_loop());

    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        EXPECT_THAT(mir_output_get_power_mode(output), Eq(mode));
    }

    client.disconnect();
}

TEST_P(DisplayOrientationSetting, can_get_orientation)
{
    using namespace testing;

    auto orientation = GetParam();

    std::shared_ptr<mg::DisplayConfiguration> server_config = server.the_display()->configuration();

    server_config->for_each_output(
        [orientation](mg::UserDisplayConfigurationOutput& output)
        {
            output.orientation = orientation;
        });

    mock_display.emit_configuration_change_event(server_config);
    mock_display.wait_for_configuration_change_handler();

    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        EXPECT_THAT(mir_output_get_orientation(output), Eq(orientation));
    }

    client.disconnect();
}

namespace
{
std::vector<MirPixelFormat> const formats{
    mir_pixel_format_abgr_8888,
    mir_pixel_format_xbgr_8888,
    mir_pixel_format_argb_8888,
    mir_pixel_format_xrgb_8888,
    mir_pixel_format_bgr_888,
    mir_pixel_format_rgb_888,
    mir_pixel_format_rgb_565,
    mir_pixel_format_rgba_5551,
    mir_pixel_format_rgba_4444,
};
}

TEST_P(DisplayFormatSetting, can_get_all_output_format)
{
    using namespace testing;

    auto format = GetParam();
    mtd::StubDisplayConfig single_format_config(1, {format});

    mock_display.emit_configuration_change_event(mt::fake_shared(single_format_config));
    mock_display.wait_for_configuration_change_handler();

    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        EXPECT_THAT(mir_output_get_current_pixel_format(output), Eq(format));
    }

    client.disconnect();
}

TEST_P(DisplaySubpixelSetting, can_get_all_subpixel_arrangements)
{
    using namespace testing;

    auto subpixel_arrangement = GetParam();
    mtd::StubDisplayConfigurationOutput output{
        {1920, 1200},
        {200, 100},
        mir_pixel_format_abgr_8888,
        60.0,
        true,
        subpixel_arrangement};
    mtd::StubDisplayConfig single_subpixel_config({output});

    mock_display.emit_configuration_change_event(mt::fake_shared(single_subpixel_config));
    mock_display.wait_for_configuration_change_handler();

    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        EXPECT_THAT(mir_output_get_subpixel_arrangement(output), Eq(subpixel_arrangement));
    }

    client.disconnect();
}


INSTANTIATE_TEST_CASE_P(DisplayConfiguration, DisplayPowerSetting,
    Values(mir_power_mode_on, mir_power_mode_standby, mir_power_mode_suspend, mir_power_mode_off));

INSTANTIATE_TEST_CASE_P(DisplayConfiguration, DisplayFormatSetting,
    ValuesIn(formats));

INSTANTIATE_TEST_CASE_P(DisplayConfiguration, DisplaySubpixelSetting,
    Values(
        mir_subpixel_arrangement_unknown,
        mir_subpixel_arrangement_horizontal_rgb,
        mir_subpixel_arrangement_horizontal_bgr,
        mir_subpixel_arrangement_vertical_rgb,
        mir_subpixel_arrangement_vertical_bgr,
        mir_subpixel_arrangement_none));

TEST_F(DisplayConfigurationTest, client_received_configuration_matches_server_config)
{
    mg::DisplayConfigurationMode hd{{1280, 720}, 60.0};
    mg::DisplayConfigurationMode fhd{{1920, 1080}, 60.0};
    mg::DisplayConfigurationMode proper_size{{1920, 1200}, 60.0};
    mg::DisplayConfigurationMode retina{{3210, 2800}, 60.0};

    mtd::StubDisplayConfigurationOutput tv{
        mg::DisplayConfigurationOutputId{1},
        {hd, fhd},
        {mir_pixel_format_abgr_8888}};
    mtd::StubDisplayConfigurationOutput monitor{
        mg::DisplayConfigurationOutputId{2},
        {hd, fhd, proper_size, retina},
        {mir_pixel_format_abgr_8888}};

    std::vector<mg::DisplayConfigurationOutput> outputs{tv, monitor};

    auto config = std::make_shared<mtd::StubDisplayConfig>(outputs);

    mock_display.emit_configuration_change_event(config);
    mock_display.wait_for_configuration_change_handler();

    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();
    auto server_config = server.the_display()->configuration();

    EXPECT_THAT(client_config.get(), mt::DisplayConfigMatches(std::cref(*server_config)));

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, client_receives_correct_mode_information)
{
    mg::DisplayConfigurationMode hd{{1280, 720}, 60.0};
    mg::DisplayConfigurationMode fhd{{1920, 1080}, 60.0};
    mg::DisplayConfigurationMode proper_size{{1920, 1200}, 60.0};
    mg::DisplayConfigurationMode retina{{3210, 2800}, 60.0};

    mg::DisplayConfigurationOutputId const id{2};

    std::vector<mg::DisplayConfigurationMode> modes{hd, fhd, proper_size, retina};

    mtd::StubDisplayConfigurationOutput monitor{
        id,
        modes,
        {mir_pixel_format_abgr_8888}};

    std::vector<mg::DisplayConfigurationOutput> outputs{monitor};

    mock_display.emit_configuration_change_event(std::make_shared<mtd::StubDisplayConfig>(outputs));
    mock_display.wait_for_configuration_change_handler();

    DisplayClient client{new_connection()};

    client.connect();

    auto config = client.get_base_config();

    ASSERT_THAT(mir_display_config_get_num_outputs(config.get()), Eq(1));

    std::vector<mg::DisplayConfigurationMode> received_modes;

    auto output = mir_display_config_get_output(config.get(), 0);

    for (auto i = 0; i < mir_output_get_num_modes(output) ; ++i)
    {
        auto mode = mir_output_get_mode(output, i);
        auto width = mir_output_mode_get_width(mode);
        auto height = mir_output_mode_get_height(mode);
        auto refresh = mir_output_mode_get_refresh_rate(mode);

        received_modes.push_back(mg::DisplayConfigurationMode{{width, height}, refresh});
    }

    EXPECT_THAT(received_modes, ContainerEq(modes));

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, mode_width_and_height_are_independent_of_orientation)
{
    mg::DisplayConfigurationMode hd{{1280, 720}, 60.0};
    mg::DisplayConfigurationMode fhd{{1920, 1080}, 60.0};
    mg::DisplayConfigurationMode proper_size{{1920, 1200}, 60.0};
    mg::DisplayConfigurationMode retina{{3210, 2800}, 60.0};

    mg::DisplayConfigurationOutputId const id{2};

    std::vector<mg::DisplayConfigurationMode> modes{hd, fhd, proper_size, retina};

    mtd::StubDisplayConfigurationOutput monitor{
        id,
        modes,
        {mir_pixel_format_abgr_8888}};

    std::vector<mg::DisplayConfigurationOutput> outputs{monitor};

    std::shared_ptr<mg::DisplayConfiguration> server_config;
    server_config = std::make_shared<mtd::StubDisplayConfig>(outputs);
    mock_display.emit_configuration_change_event(server_config);
    mock_display.wait_for_configuration_change_handler();

    DisplayClient client{new_connection()};
    client.connect();


    DisplayConfigMatchingContext context;
    context.matcher = [&server_config](MirDisplayConfig* conf)
        {
            EXPECT_THAT(conf, mt::DisplayConfigMatches(std::cref(*server_config)));
        };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    for (auto const orientation :
        {mir_orientation_normal, mir_orientation_left, mir_orientation_inverted, mir_orientation_right})
    {
        server_config = server.the_display()->configuration();
        server_config->for_each_output(
            [orientation](mg::UserDisplayConfigurationOutput& output)
            {
                output.orientation = orientation;
            });
        server.the_display_configuration_controller()->set_base_configuration(server_config);

        EXPECT_TRUE(context.done.wait_for(std::chrono::seconds{10}));
        context.done.reset();

        auto config = client.get_base_config();
        ASSERT_THAT(mir_display_config_get_num_outputs(config.get()), Eq(1));

        std::vector<mg::DisplayConfigurationMode> received_modes;

        auto output = mir_display_config_get_output(config.get(), 0);

        for (auto i = 0; i < mir_output_get_num_modes(output) ; ++i)
        {
            auto mode = mir_output_get_mode(output, i);
            auto width = mir_output_mode_get_width(mode);
            auto height = mir_output_mode_get_height(mode);
            auto refresh = mir_output_mode_get_refresh_rate(mode);

            received_modes.push_back(mg::DisplayConfigurationMode{{width, height}, refresh});
        }

        EXPECT_THAT(received_modes, ContainerEq(modes));
    }

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, output_position_is_independent_of_orientation)
{
    std::array<mir::geometry::Point, 3> const positions = {{
        mir::geometry::Point{-100, 10},
        mir::geometry::Point{100, 10000},
        mir::geometry::Point{-100, 10}
    }};

    std::shared_ptr<mg::DisplayConfiguration> server_config = server.the_display()->configuration();
    server_config->for_each_output(
        [position = positions.begin()](mg::UserDisplayConfigurationOutput& output) mutable
        {
            output.top_left = *position;
            ++position;
        });

    DisplayClient client{new_connection()};

    client.connect();

    DisplayConfigMatchingContext context;
    context.matcher = [&server_config](MirDisplayConfig* conf)
        {
            EXPECT_THAT(conf, mt::DisplayConfigMatches(std::cref(*server_config)));
        };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    server.the_display_configuration_controller()->set_base_configuration(server_config);
    context.done.wait_for(std::chrono::seconds{10});

    for (auto const orientation :
        {mir_orientation_normal, mir_orientation_left, mir_orientation_inverted, mir_orientation_right})
    {
        server_config = server.the_display()->configuration();
        server_config->for_each_output(
            [orientation](mg::UserDisplayConfigurationOutput& output)
                {
                    output.orientation = orientation;
                });

        context.done.reset();
        server.the_display_configuration_controller()->set_base_configuration(server_config);

        EXPECT_TRUE(context.done.wait_for(std::chrono::seconds{10}));

        auto config = client.get_base_config();

        auto position = positions.begin();
        for (auto i = 0; i < mir_display_config_get_num_outputs(config.get()); ++i)
        {
            auto output = mir_display_config_get_output(config.get(), i);

            EXPECT_THAT(mir_output_get_position_x(output), Eq(position->x.as_int()));
            EXPECT_THAT(mir_output_get_position_y(output), Eq(position->y.as_int()));

            ++position;
        }
    }

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, client_receives_correct_output_positions)
{
    std::array<mir::geometry::Point, 3> const positions = {{
        mir::geometry::Point{-100, 10},
        mir::geometry::Point{100, 10000},
        mir::geometry::Point{-100, 10}
    }};

    std::shared_ptr<mg::DisplayConfiguration> server_config = server.the_display()->configuration();
    server_config->for_each_output(
    [position = positions.begin()](mg::UserDisplayConfigurationOutput& output) mutable
    {
        output.top_left = *position;
        ++position;
    });

    DisplayClient client{new_connection()};

    client.connect();

    DisplayConfigMatchingContext context;
    context.matcher = [server_config](MirDisplayConfig* conf)
        {
            EXPECT_THAT(conf, mt::DisplayConfigMatches(std::cref(*server_config)));
        };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    server.the_display_configuration_controller()->set_base_configuration(server_config);

    EXPECT_TRUE(context.done.wait_for(std::chrono::seconds{10}));

    auto config = client.get_base_config();

    auto position = positions.begin();
    for (auto i = 0; i < mir_display_config_get_num_outputs(config.get()); ++i)
    {
        auto output = mir_display_config_get_output(config.get(), i);

        EXPECT_THAT(mir_output_get_position_x(output), Eq(position->x.as_int()));
        EXPECT_THAT(mir_output_get_position_y(output), Eq(position->y.as_int()));

        ++position;
    }

    client.disconnect();
}

namespace
{
void signal_when_config_received(MirConnection* /*unused*/, void* ctx)
{
    auto signal = reinterpret_cast<mt::Signal*>(ctx);

    signal->raise();
}
}

TEST_F(DisplayConfigurationTest, client_sees_server_set_scale_factor)
{
    std::shared_ptr<mg::DisplayConfiguration> current_config = server.the_display()->configuration();
    current_config->for_each_output(
        [](mg::UserDisplayConfigurationOutput& output)
        {
            static int output_num{0};

            output.scale = 1 + 0.25f * output_num;
            ++output_num;
        });

    DisplayClient client{new_connection()};

    client.connect();

    mt::Signal configuration_received;
    mir_connection_set_display_config_change_callback(
        client.connection,
        &signal_when_config_received,
        &configuration_received);

    server.the_display_configuration_controller()->set_base_configuration(current_config);

    EXPECT_TRUE(configuration_received.wait_for(std::chrono::seconds{10}));

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        EXPECT_THAT(mir_output_get_scale_factor(output), Eq(1 + 0.25 * i));
    }

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, client_sees_server_set_form_factor)
{
    std::array<MirFormFactor, 3> const form_factors = {{
        mir_form_factor_monitor,
        mir_form_factor_projector,
        mir_form_factor_unknown
    }};

    std::shared_ptr<mg::DisplayConfiguration> current_config = server.the_display()->configuration();
    current_config->for_each_output(
        [&form_factors](mg::UserDisplayConfigurationOutput& output)
            {
                static int output_num{0};

                output.form_factor = form_factors[output_num];
                ++output_num;
            });

    DisplayClient client{new_connection()};

    client.connect();

    mt::Signal configuration_received;
    mir_connection_set_display_config_change_callback(
        client.connection,
        &signal_when_config_received,
        &configuration_received);

    server.the_display_configuration_controller()->set_base_configuration(current_config);

    EXPECT_TRUE(configuration_received.wait_for(std::chrono::seconds{10}));

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        EXPECT_THAT(mir_output_get_form_factor(output), Eq(form_factors[i]));
    }

    client.disconnect();
}

namespace
{
MATCHER_P(IsSameModeAs, mode, "")
{
    return mir_output_mode_get_height(arg) == mir_output_mode_get_height(mode) &&
        mir_output_mode_get_width(arg) == mir_output_mode_get_width(mode) &&
        mir_output_mode_get_refresh_rate(arg) == mir_output_mode_get_refresh_rate(mode);
}
}

TEST_F(DisplayConfigurationTest, get_current_mode_index_invariants)
{
    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        if (auto mode = mir_output_get_current_mode(output))
        {
            auto indexed_mode = mir_output_get_mode(output, mir_output_get_current_mode_index(output));
            EXPECT_THAT(indexed_mode, IsSameModeAs(mode));
        }
        else
        {
            EXPECT_THAT(mir_output_get_current_mode_index(output), Eq(std::numeric_limits<size_t>::max()));
        }
    }

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, get_preferred_mode_index_invariants)
{
    DisplayClient client{new_connection()};

    client.connect();

    auto client_config = client.get_base_config();

    for (int i = 0; i < mir_display_config_get_num_outputs(client_config.get()); ++i)
    {
        auto output = mir_display_config_get_output(client_config.get(), i);

        if (auto mode = mir_output_get_preferred_mode(output))
        {
            auto indexed_mode = mir_output_get_mode(output, mir_output_get_preferred_mode_index(output));
            EXPECT_THAT(indexed_mode, IsSameModeAs(mode));
        }
        else
        {
            EXPECT_THAT(mir_output_get_preferred_mode_index(output), Eq(std::numeric_limits<size_t>::max()));
        }
    }

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, preview_base_display_configuration_sends_config_event)
{
    DisplayClient client{new_connection()};

    client.connect();

    std::shared_ptr<MirDisplayConfig> config = client.get_base_config();

    for (auto i = 0; i < mir_display_config_get_num_outputs(config.get()); ++i)
    {
        auto output = mir_display_config_get_mutable_output(config.get(), i);

        for (auto j = 0; j < mir_output_get_num_modes(output); ++j)
        {
            auto mode = mir_output_get_mode(output, j);

            if (mode != mir_output_get_current_mode(output))
            {
                mir_output_set_current_mode(output, mode);
                break;
            }
        }
    }

    ASSERT_THAT(config.get(), Not(mt::DisplayConfigMatches(client.get_base_config().get())));

    DisplayConfigMatchingContext context;
    context.matcher = [config](MirDisplayConfig* conf)
        {
            EXPECT_THAT(conf, mt::DisplayConfigMatches(config.get()));
        };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    mir_connection_preview_base_display_configuration(client.connection, config.get(), 5);

    EXPECT_TRUE(context.done.wait_for(std::chrono::seconds{10}));

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, preview_base_display_configuration_reverts_after_timeout)
{
    DisplayClient client{new_connection()};

    client.connect();

    std::shared_ptr<MirDisplayConfig> old_config = client.get_base_config();
    std::shared_ptr<MirDisplayConfig> new_config = client.get_base_config();

    for (auto i = 0; i < mir_display_config_get_num_outputs(new_config.get()); ++i)
    {
        auto output = mir_display_config_get_mutable_output(new_config.get(), i);

        for (auto j = 0; j < mir_output_get_num_modes(output); ++j)
        {
            auto mode = mir_output_get_mode(output, j);

            if (mode != mir_output_get_current_mode(output))
            {
                mir_output_set_current_mode(output, mode);
                break;
            }
        }
    }

    ASSERT_THAT(new_config.get(), Not(mt::DisplayConfigMatches(old_config.get())));

    DisplayConfigMatchingContext context;
    auto reverted = std::make_shared<mt::Signal>();
    context.matcher = [old_config, new_config, reverted](MirDisplayConfig* conf)
        {
            static int call_count{0};
            ++call_count;
            if (call_count == 1)
            {
                EXPECT_THAT(conf, mt::DisplayConfigMatches(new_config.get()));
            }
            else if (call_count == 2)
            {
                EXPECT_THAT(conf, mt::DisplayConfigMatches(old_config.get()));
                reverted->raise();
            }
            else
            {
                FAIL() << "Received unexpected configuration event";
            }
        };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    mir_connection_preview_base_display_configuration(client.connection, new_config.get(), 5);

    std::this_thread::sleep_for(std::chrono::seconds{3});
    // Should still have the old config
    EXPECT_TRUE(context.done.raised());
    EXPECT_FALSE(reverted->raised());

    EXPECT_TRUE(reverted->wait_for(std::chrono::seconds{10}));

    client.disconnect();
}

TEST_F(DisplayConfigurationTest, display_configuration_sticks_after_confirmation)
{
    DisplayClient client{new_connection()};

    client.connect();

    std::shared_ptr<MirDisplayConfig> old_config = client.get_base_config();
    std::shared_ptr<MirDisplayConfig> new_config = client.get_base_config();

    for (auto i = 0; i < mir_display_config_get_num_outputs(new_config.get()); ++i)
    {
        auto output = mir_display_config_get_mutable_output(new_config.get(), i);

        for (auto j = 0; j < mir_output_get_num_modes(output); ++j)
        {
            auto mode = mir_output_get_mode(output, j);

            if (mode != mir_output_get_current_mode(output))
            {
                mir_output_set_current_mode(output, mode);
                break;
            }
        }
    }

    ASSERT_THAT(new_config.get(), Not(mt::DisplayConfigMatches(old_config.get())));

    DisplayConfigMatchingContext context;
    auto signalled_twice = std::make_shared<mt::Signal>();
    context.matcher = [new_config, signalled_twice](MirDisplayConfig* conf)
        {
            static int call_count{0};
            ++call_count;
            EXPECT_THAT(conf, mt::DisplayConfigMatches(new_config.get()));
            if (call_count == 2)
            {
                signalled_twice->raise();
            }
        };

    mir_connection_set_display_config_change_callback(
        client.connection,
        &new_display_config_matches,
        &context);

    mir_connection_preview_base_display_configuration(client.connection, new_config.get(), 10);

    EXPECT_TRUE(context.done.wait_for(std::chrono::seconds{5}));

    mir_connection_confirm_base_display_configuration(client.connection, new_config.get());

    EXPECT_TRUE(signalled_twice->wait_for(std::chrono::seconds{10}));

    client.disconnect();
}

namespace
{
struct ErrorValidator
{
    mt::Signal received;
    std::function<void(MirError const*)> validate;
};

void validating_error_handler(MirConnection*, MirError const* error, void* context)
{
    auto& error_validator = *reinterpret_cast<ErrorValidator*>(context);
    error_validator.validate(error);
    error_validator.received.raise();
}
}

TEST_F(DisplayConfigurationTest, unauthorised_client_receives_error)
{
    stub_authorizer.allow_set_base_display_configuration = false;

    DisplayClient client{new_connection()};

    client.connect();

    auto config = client.get_base_config();

    ErrorValidator validator;
    validator.validate = [&config](MirError const* error)
        {
            EXPECT_THAT(mir_error_get_domain(error), Eq(mir_error_domain_display_configuration));
            EXPECT_THAT(mir_error_get_code(error), Eq(mir_display_configuration_error_unauthorized));
        };
    mir_connection_set_error_callback(client.connection, &validating_error_handler, &validator);

    mir_connection_preview_base_display_configuration(client.connection, config.get(), 20);

    EXPECT_TRUE(validator.received.wait_for(std::chrono::seconds{10}));

    client.disconnect();
}
