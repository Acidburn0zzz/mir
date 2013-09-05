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

#ifndef MIR_CLIENT_MIR_CONNECTION_FACTORY
#define MIR_CLIENT_MIR_CONNECTION_FACTORY

class MirConnectionFactory
{
public:
    virtual MirConnection* create_mir_connection(std::string const& socket_file) = 0;

protected:
    virtual ~MirConnectionFactory() = default;
    MirConnectionFactory() = default;
    MirConnectionFactory(const MirConnectionFactory&) = delete;
    MirConnectionFactory& operator=(const MirConnectionFactory&) = delete;
};

#endif /* MIR_CLIENT_MIR_CONNECTION_FACTORY */
