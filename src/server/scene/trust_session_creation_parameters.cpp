/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored By: Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "mir/scene/trust_session_creation_parameters.h"

namespace ms = mir::scene;

bool ms::operator==(
    const TrustSessionCreationParameters& lhs,
    const TrustSessionCreationParameters& rhs)
{
    return lhs.base_process_id == rhs.base_process_id;
}

bool ms::operator!=(
    const TrustSessionCreationParameters& lhs,
    const TrustSessionCreationParameters& rhs)
{
    return !(lhs == rhs);
}
