#ifndef __MIGRATION_H__
#define __MIGRATION_H__

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



enum specialisationRecup
{
    SP_HEAL = 0,
    SP_DPS_CAC = 1,
    SP_TANK = 2,
    SP_DPS_CAST = 3,
};

enum guildRecup
{
    NOGUILD = 0,
    ISGUILD = 1,
    BOTH = 2
};

enum MigrationErrors
{
    ERROR_PACKID,
    ERROR_LEVEL,
    NO_ERROR
};

enum Divers
{
    MIGRATION_GOLDS =  20000000,
    MOUNT_HORDE = 25532,
    MOUNT_ALLIANCE = 25473,
    CUSTOM_XP_BONUS_ITEM = 200000,
    MIGRATION_BAGS = 41599,
    MIGRATION_REFRESH_TIMER = 30000,
    MIGRATION_EXECUTION_TIMER = 5000,
    MAX_MIGRATION_PER_REFRESH = 50,
};

struct eMigration
{
    eMigration(uint32 _packId, uint32 _itemEntry, uint32 _faction, uint32 _specialisation, uint32 _level, uint32 _guild, uint32 _class)
    {
        packId = _packId;
        itemEntry = _itemEntry;
        faction = _faction;
        specialisation = _specialisation;
        level = _level;
        guild = _guild;
        classes = _class;
    }
    eMigration() {}

    uint32 packId;
    uint32 itemEntry;
    uint32 faction;
    uint32 specialisation;
    uint32 level;
    uint32 guild;
    uint32 classes;

};

struct eMigrationChar
{
    eMigrationChar(uint32 _r_charguid, uint32 _r_acctid, uint32 _r_guid, uint32 _level, uint32 _specialisation, uint32 _guild, std::string _r_skills, std::string _r_reputs)
    {
        r_charguid = _r_charguid;
        r_acctid = _r_acctid;
        r_guid = _r_guid;
        level = _level;
        specialisation = _specialisation;
        guild = _guild;
        r_skills = _r_skills;
        r_reputs = _r_reputs;
    }
    eMigrationChar() {}

    uint32 r_charguid;
    uint32 r_acctid;
    uint32 r_guid;
    uint32 level;
    uint32 specialisation;
    uint32 guild;
    std::string r_skills;
    std::string r_reputs;
};

struct eMigrationSpells
{
    eMigrationSpells() {}

    uint32 spellID;
    uint32 levelReq;
    uint32 factionID;
    uint32 classID;
};

struct eMigrationSpecialItems
{
    eMigrationSpecialItems() {}

    uint32 entry;
    uint32 quantity;
    uint32 is_guild;
    uint32 profession;
    uint32 faction;
};

struct eMigrationProfessionSpells
{
    eMigrationProfessionSpells() {}

    uint32 entry;
    uint32 reqskill;
    uint32 reqskillvalue;
    uint32 reqlevel;
};

class Migration
{
 public:
    Migration();
    ~Migration();
    void Update(const uint32 &diff);
    void CheckValidMigration(Player *player);
    void FillSynchronisation(Player *player);
    void FillWotlkAchievements(Player *player);

 private:
    uint32 findPack(uint32 faction, uint32 specialization, uint32 level, uint32 guild, uint32 classId);
    void GetMigrationItemList(std::list<uint32 > &itemList, uint32 packId, Player *player, uint32 specialisation, bool isGuild);
    bool LearnPrimaryProfession(Player *newChar, uint32 profId, uint32 newValue, bool);
    bool LearnSecondaryProfession(Player *newChar, uint32 profId, uint32 newValue, bool);
    char **my_explode(char *str, char separator);
    void RecupStoreNewItemInBestSlots(uint32 itemEntry, uint32 count, Player *player);
    void UpdateProfessions(const char *r_skills, Player *player, bool );
    void UpdateReputation(const char *r_reputs, Player *player);
    void UpdateMountAndDbleSpe(Player *player);
    void UpdateClassSkills(Player *player);
    void ValidateMigration(uint32 r_guid);
    void UpdateItemsAndGolds(Player *player, uint32 packId, uint32 specialization, bool isGuild);
    void TeleportToCapital(Player *player);
    void UpdateRaceSkills(Player *player);
    bool AbortMigration(MigrationErrors err, uint32 migrationGUID, Player *player);
    void UpdateLevel(Player *player, uint8 level);
    void LearnAllClassSpells(Player *player);
    void ForceQuest(Player *player, uint32 questId);
    void CompleteClassQuests(Player *player);
    void RefreshMigrationList();
    bool ExecuteMigration(Player* player, eMigrationChar & migrInfos);

    void AddSpecialItems(Player *player, bool isGuild);
    void AddProfesionItems(uint32 skill_id, Player *player, bool isGuild);
    void RollCustomMount(Player *player);
    void LearnAdditionalProfessionSpells(Player *player, bool isGuild, uint32 skillId, uint32 skillLevel);

    std::map<uint32, Bag *> FillWotlkBags(Player *player);
    void FillWotlkItems(Player *player);
    void FillWotlkSpells(Player *player);
    void FillProfessions(Player *player);
    void ValidateSynchronisation(uint32 guid);
    void FillReputations(Player *player);

 private:
    std::list<std::pair<eMigration, uint32 > > migrationInfos;
    std::list<std::pair<uint32 , eMigrationChar > > migrationsChar;
    std::list<eMigrationSpells > migrationSpells;
    std::map<uint32, std::list<eMigrationSpecialItems > > migrationProfesionItems;
    std::list<eMigrationSpecialItems > migrationSpecialItems;
    std::list<eMigrationProfessionSpells > migrationProfessionSpells;
    uint32 m_migrationRefreshTimer;
    uint32 m_migrationExecuteTimer;
    ACE_Thread_Mutex LockMigrationList;
};

#endif
