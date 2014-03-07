/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Nick Dedekind <nick.dedekind@gmail.com>
 */

#include "trust_session_control.h"

namespace mcl = mir::client;

mcl::TrustSessionControl::TrustSessionControl() :
    next_fn_id(0)
{
}

mcl::TrustSessionControl::~TrustSessionControl()
{
}

int mcl::TrustSessionControl::add_trust_session_event_handler(std::function<void(MirTrustSessionState)> const& fn)
{
    std::unique_lock<std::mutex> lk(guard);

    int id = next_id();
    handle_trust_session_events[id] = fn;
    return id;
}

void mcl::TrustSessionControl::remove_trust_session_event_handler(int id)
{
    std::unique_lock<std::mutex> lk(guard);

    handle_trust_session_events.erase(id);
}

void mcl::TrustSessionControl::call_trust_session_event_handler(uint32_t state)
{
    std::unique_lock<std::mutex> lk(guard);

    for (auto const& fn : handle_trust_session_events)
    {
        fn.second(static_cast<MirTrustSessionState>(state));
    }
}

int mcl::TrustSessionControl::next_id()
{
    return ++next_fn_id;
}