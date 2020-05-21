#ifndef __ARENALOG_H__
#define __ARENALOG_H__

#include "Log.h"
#include "Object.h"
#include "Bag.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Corpse.h"
#include "QuestDef.h"
#include "ItemPrototype.h"
#include "NPCHandler.h"
#include "DatabaseEnv.h"
#include "Mail.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include <ace/Singleton.h>
#include "VehicleDefines.h"
#include <string>
#include <map>
#include <limits>
#include "ConditionMgr.h"
#include <functional>
#include "PhaseMgr.h"
#include "DB2Stores.h"

#define ARENALOG_REFRESH_TIMER 10000

struct arenaInfos
{
    arenaInfos() {}

    std::list<uint32 > winnerGuildId;
    std::list<uint64 > winnerDamages;
    std::list<uint64 > winnerHeals;
    std::list<uint64 > winnerKillingBlows;
    std::list<uint64 > winnerGUID;

    uint32 winnerTeamId;

    std::list<uint32 > looserGuildId;
    std::list<uint64 > looserDamages;
    std::list<uint64 > looserHeals;
    std::list<uint64 > looserKillingBlows;
    std::list<uint64 > looserGUID;
    uint32 looserTeamId;

    uint32 matchHourStart;
    uint32 matchTime;

    std::string timeEnd;
};

class ArenaLog
{
 public:
    ArenaLog();
    ~ArenaLog();
    void saveMatch(arenaInfos *);
    void Update(const uint32 &diff);


 private:
    std::list<arenaInfos *> arenaInfosList;
    uint32 m_arenaLogTimer;
    ACE_Thread_Mutex LockArenaLogList;
};

#endif
