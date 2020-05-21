#ifndef __ARENAREWARD_H__
#define __ARENAREWARD_H__

#include "Log.h"
#include "Object.h"
#include "Bag.h"
#include "AchievementMgr.h"
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



struct eArenaReward
{
    eArenaReward(uint32 _seasonID, uint32 _achievement, uint32 _itemEntry, uint32 _rank, uint32 _title)
    {
        seasonID = _seasonID;
        achievement = _achievement;
        itemEntry = _itemEntry;
        rank = _rank;
        title = _title;
    }
    eArenaReward() {}

    uint32 seasonID;
    uint32 achievement;
    uint32 itemEntry;
    uint32 rank;
    uint32 title;
};

struct eArenaRewardChar
{
    eArenaRewardChar(uint32 _charguid, uint32 _rank, uint32 _seasonID, uint32 _type)
    {
        guid = _charguid;
        rank = _rank;
        seasonID = _seasonID;
        type = _type;
    }
    eArenaRewardChar() {}

    uint32 guid;
    uint32 rank;
    uint32 seasonID;
    uint32 type;
};

class ArenaReward
{
 public:
    ArenaReward();
    ~ArenaReward();
    void CheckValidArenaReward(Player *player);
    void Initialize();

    void reward2C2Season();
    void reward3C3Season();
    void resetSeason();

 private:
    std::list<std::pair<eArenaReward, uint32 > > rewardsInfos;
    std::list<std::pair<uint32 , eArenaRewardChar > > arenaRewardsChar;
    uint32 m_arenaRewardRefreshTimer;
    ACE_Thread_Mutex LockMigrationList;
};


#endif
