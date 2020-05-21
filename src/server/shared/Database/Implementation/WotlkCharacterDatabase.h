/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WOTLKCHARACTERDATABASE_H
#define _WOTLKCHARACTERDATABASE_H

#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"

class WotlkCharacterDatabaseConnection : public MySQLConnection
{
    public:
        //- Constructors for sync and async connections
        WotlkCharacterDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) {}
        WotlkCharacterDatabaseConnection(ACE_Activation_Queue* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) {}

        //- Loads database type specific prepared statements
        void DoPrepareStatements();
};

typedef DatabaseWorkerPool<WotlkCharacterDatabaseConnection> WotlkCharacterDatabaseWorkerPool;

enum WotlkCharacterDatabaseStatements
{
    MAX_WOTLKCHARACTERDATABASE_STATEMENTS
};

#endif
