#include "Migration.h"
#include "AccountMgr.h"
#include "AchievementMgr.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "Chat.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DB2Structure.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "GameEventMgr.h"
#include "GossipDef.h"
#include "GroupMgr.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "Language.h"
#include "LFGMgr.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "PoolMgr.h"
#include "ReputationMgr.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Transport.h"
#include "UpdateMask.h"
#include "Util.h"
#include "Vehicle.h"
#include "WaypointManager.h"
#include "World.h"
#include "InfoMgr.h"

#define BOTH_FACT 42

Migration::Migration()
{
    TC_LOG_INFO("loading", "Loading Recup packs...");
    uint32 oldMSTime = getMSTime();
    QueryResult result = WorldDatabase.Query("SELECT pack_id, item_entry, faction, spe, level, guild, class FROM pack_item_recup");
    if (!result)
        TC_LOG_INFO("loading", ">> Loaded 0 pack item recup. DB table `pack_item_recup` is empty.");
    else
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();
            eMigration tempMigr;
            tempMigr.packId = fields[0].GetUInt32();
            tempMigr.itemEntry = fields[1].GetUInt32();
            tempMigr.faction = fields[2].GetUInt32();
            tempMigr.specialisation = fields[3].GetUInt32();
            tempMigr.level = fields[4].GetUInt32();
            tempMigr.guild = fields[5].GetUInt32();
            tempMigr.classes = fields[6].GetUInt32();
            std::pair<eMigration, uint32 > res;
            res.first = tempMigr;
            res.second = tempMigr.packId;
            ItemTemplate const *item = sObjectMgr->GetItemTemplate(tempMigr.itemEntry);
            if (item == NULL)
            {
                //                sLog->outError(LOG_FILTER_GENERAL, "MIGRATION :  item entry %u, pack %u doesnt exist, item skip", tempMigr.itemEntry, tempMigr.packId);
                continue;
            }
            //            else if (item->RequiredReputationFaction)
            //                sLog->outError(LOG_FILTER_GENERAL, "MIGRATION :  item entry %u, pack %u need faction reputation to be equip", tempMigr.itemEntry, tempMigr.packId);
            migrationInfos.push_back(res);
            count++;
        }
        while (result->NextRow());
        //        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pack item recup in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    //    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "Loading Migration List...");
    m_migrationRefreshTimer = MIGRATION_REFRESH_TIMER;
    result = WebDatabase.PQuery("SELECT perso, account, id, level, spe, guild, skills, reputs FROM recups_cata WHERE status = 2 ORDER BY id ASC");
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 charGUID = fields[0].GetUInt32();
            eMigrationChar tempCharMigr(charGUID, fields[1].GetUInt32(), fields[2].GetUInt32(), fields[3].GetUInt32(), fields[4].GetUInt32(),
                                        fields[5].GetUInt32(), fields[6].GetString(), fields[7].GetString());
            std::pair<uint32, eMigrationChar > res;
            res.second = tempCharMigr;
            res.first = charGUID;
            migrationsChar.push_back(res);
            count++;
        }
        while (result->NextRow());
        //        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u valide migration in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
    m_migrationExecuteTimer = MIGRATION_EXECUTION_TIMER;

    //    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loading migration spell list");
    result = WorldDatabase.PQuery("SELECT entry, spell, reqlevel FROM npc_trainer WHERE entry in(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) and reqskill = 0 and spell > 0", 20407, 44740, 200007, 200005, 200011, 28474, 200006, 200002, 200102, 200101, 200009, 200008, 200004, 200001, 200003);
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            int32 spellID = fields[1].GetInt32();
            uint32 levelReq = fields[2].GetUInt32();
            uint32 factionID = 0;
            uint32 classID = 0;
            switch (entry)
            {
                case 200003:
                    classID = CLASS_HUNTER;
                    factionID = BOTH_FACT;
                    break;
                case 200001:
                    classID = CLASS_WARRIOR;
                    factionID = BOTH_FACT;
                    break;
                case 200004:
                    classID = CLASS_ROGUE;
                    factionID = BOTH_FACT;
                    break;
                case 200008:
                    classID = CLASS_MAGE;
                    factionID = BOTH_FACT;
                    break;
                case 200009:
                    classID = CLASS_WARLOCK;
                    factionID = BOTH_FACT;
                    break;
                case 200101:
                case 200102:
                case 200002:
                    classID = CLASS_PALADIN;
                    factionID = BOTH_FACT;
                    break;
                case 200006:
                case 28474:
                    classID = CLASS_DEATH_KNIGHT;
                    factionID = BOTH_FACT;
                    break;
                case 200011:
                    classID = CLASS_DRUID;
                    factionID = BOTH_FACT;
                    break;
                case 200005:
                    factionID = BOTH_FACT;
                    classID = CLASS_PRIEST;
                    break;
                case 200007:
                    classID = CLASS_SHAMAN;
                    factionID = BOTH_FACT;
                    break;
                case 44740:
                    classID = CLASS_SHAMAN;
                    factionID = HORDE;
                    break;
                case 20407:
                    factionID = ALLIANCE;
                    classID = CLASS_SHAMAN;
                    break;
                default:
                    continue;
            }
            eMigrationSpells spellCarac;
            spellCarac.spellID = spellID;
            spellCarac.levelReq = levelReq;
            spellCarac.factionID = factionID;
            spellCarac.classID = classID;
            migrationSpells.push_back(spellCarac);
            count++;
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Loaded %u valid migration spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    TC_LOG_INFO("loading", ">> Loading migration special items list");
    result = WorldDatabase.PQuery("SELECT id, quantity, is_guild, profession, faction FROM recups_special_item");
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 quantity = fields[1].GetUInt32();
            uint32 is_guild = fields[2].GetUInt32();
            uint32 profession = fields[3].GetUInt32();
            uint32 faction = fields[4].GetUInt32();
            eMigrationSpecialItems spItems;
            spItems.entry = entry;
            spItems.quantity = quantity;
            spItems.is_guild = is_guild;
            spItems.profession = profession;
            spItems.faction = faction;
            ItemTemplate const *item = sObjectMgr->GetItemTemplate(entry);
            if (item == NULL)
            {
                //                sLog->outError(LOG_FILTER_GENERAL, "MIGRATION :  special item entry %u doesnt exist, item skip", entry);
                continue;
            }
            if (profession != 0)
                migrationProfesionItems[profession].push_back(spItems);
            else
                migrationSpecialItems.push_back(spItems);
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Loaded %u valid migration special items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    TC_LOG_INFO("loading", ">> Loading migration profession basic spells list");
    result = WorldDatabase.PQuery("select spell, reqskill, reqskillvalue, reqlevel from npc_trainer where reqskill in (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", 171 ,197 ,165 ,333 ,164 ,773 ,202 ,755, 393, 182, 186, 185, 129, 356);
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 reqskill = fields[1].GetUInt32();
            uint32 reqskillvalue = fields[2].GetUInt32();
            uint32 reqlevel = fields[3].GetUInt32();
            eMigrationProfessionSpells spItems;
            spItems.entry = entry;
            spItems.reqskill = reqskill;
            spItems.reqskillvalue = reqskillvalue;
            spItems.reqlevel = reqlevel;
            migrationProfessionSpells.push_back(spItems);
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Loaded %u valid rofession basic spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
}

Migration::~Migration() {}

uint32 Migration::findPack(uint32 faction, uint32 specialization, uint32 level, uint32 guild, uint32 classId)
{
    uint32 Nlevel = 0;
    if (level < 20)
        Nlevel = 10;
    else if (level < 30)
        Nlevel = 20;
    else if (level < 40)
        Nlevel = 30;
    else if (level < 50)
        Nlevel = 40;
    else if (level < 60)
        Nlevel = 50;
    else if (level < 70)
        Nlevel = 60;
    else if (level < 80)
        Nlevel = 70;
    else if (level < 85)
        Nlevel = 80;
    else
        Nlevel = 85;
    for (std::list<std::pair<eMigration, uint32 > >::iterator itr = migrationInfos.begin(); itr != migrationInfos.end(); itr++)
    {
        eMigration ps = itr->first;
        if ((ps.faction == faction || ps.faction == 0) && ps.specialisation == specialization && ps.level == Nlevel && (ps.guild == guild || ps.guild == BOTH) && ps.classes == classId)
            return (itr->second);
    }
    return 0;
}

void Migration::GetMigrationItemList(std::list<uint32 > &itemList, uint32 packId, Player *player, uint32 specialisation, bool isGuild)
{
    if (player->getLevel() < 85)
    {
        for (std::list<std::pair<eMigration, uint32 > >::iterator itr = migrationInfos.begin(); itr != migrationInfos.end(); itr++)
            if (itr->second == packId)
                itemList.push_back(itr->first.itemEntry);
    }
    else
    {
        if (isGuild)
            for (int i = 0; i < 151; i++) // pvp set
                itemList.push_back(44115);
        switch (player->getClass())
        {
            case CLASS_PALADIN:
            {
                switch (specialisation)
                {
                    case SP_HEAL:
                        itemList.push_back(60362);
                        itemList.push_back(60361);
                        itemList.push_back(60363);
                        itemList.push_back(59216);
                        itemList.push_back(59497);
                        itemList.push_back(59465);
                        itemList.push_back(60359);
                        itemList.push_back(60360);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(59501);
                        itemList.push_back(58184);
                        itemList.push_back(58183);
                        itemList.push_back(58189);
                        itemList.push_back(59463);
                        itemList.push_back(64673);
                        itemList.push_back(59327);
                        break;
                    case SP_TANK:
                        itemList.push_back(60358);
                        itemList.push_back(60357);
                        itemList.push_back(60355);
                        itemList.push_back(59328);
                        itemList.push_back(59470);
                        itemList.push_back(59117);
                        itemList.push_back(60356);
                        itemList.push_back(60354);
                        itemList.push_back(59466);
                        itemList.push_back(59319);
                        itemList.push_back(59233);
                        itemList.push_back(59332);
                        itemList.push_back(58182);
                        itemList.push_back(58187);
                        itemList.push_back(59347);
                        itemList.push_back(64676);
                        itemList.push_back(59444);
                        break;
                    case SP_DPS_CAC:
                    case SP_DPS_CAST:
                        itemList.push_back(60348);
                        itemList.push_back(60347);
                        itemList.push_back(60345);
                        itemList.push_back(59221);
                        itemList.push_back(59118);
                        itemList.push_back(59342);
                        itemList.push_back(60346);
                        itemList.push_back(60344);
                        itemList.push_back(59507);
                        itemList.push_back(59442);
                        itemList.push_back(59518);
                        itemList.push_back(58181);
                        itemList.push_back(59506);
                        itemList.push_back(58185);
                        itemList.push_back(64674);
                        itemList.push_back(59330);
                        break;
                }
                break;
            }
            case CLASS_SHAMAN:
            {
                switch (specialisation)
                {
                    case SP_HEAL:
                        itemList.push_back(60311);
                        itemList.push_back(60310);
                        itemList.push_back(60312);
                        itemList.push_back(58481);
                        itemList.push_back(59310);
                        itemList.push_back(63535);
                        itemList.push_back(60308);
                        itemList.push_back(60309);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(59501);
                        itemList.push_back(58184);
                        itemList.push_back(58183);
                        itemList.push_back(58189);
                        itemList.push_back(59459);
                        itemList.push_back(59327);
                        itemList.push_back(64673);
                        break;
                    case SP_TANK:
                    case SP_DPS_CAC:
                        itemList.push_back(60322);
                        itemList.push_back(60321);
                        itemList.push_back(60319);
                        itemList.push_back(58199);
                        itemList.push_back(59355);
                        itemList.push_back(59485);
                        itemList.push_back(60320);
                        itemList.push_back(60318);
                        itemList.push_back(59348);
                        itemList.push_back(59517);
                        itemList.push_back(59121);
                        itemList.push_back(59520);
                        itemList.push_back(58181);
                        itemList.push_back(67136);
                        itemList.push_back(59443);
                        itemList.push_back(59443);
                        itemList.push_back(64671);
                        break;
                    case SP_DPS_CAST:
                        itemList.push_back(60317);
                        itemList.push_back(60316);
                        itemList.push_back(60314);
                        itemList.push_back(58481);
                        itemList.push_back(59310);
                        itemList.push_back(63535);
                        itemList.push_back(60315);
                        itemList.push_back(60313);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(59501);
                        itemList.push_back(59519);
                        itemList.push_back(58183);
                        itemList.push_back(67129);
                        itemList.push_back(63680);
                        itemList.push_back(59327);
                        itemList.push_back(64672);
                        break;
                }
                break;
            }
            case CLASS_ROGUE:
            {
                switch (specialisation)
                {
                    case SP_HEAL:
                    case SP_DPS_CAC:
                    case SP_TANK:
                    case SP_DPS_CAST:
                        itemList.push_back(60302);
                        itemList.push_back(60300);
                        itemList.push_back(60298);
                        itemList.push_back(59469);
                        itemList.push_back(59329);
                        itemList.push_back(59502);
                        itemList.push_back(60299);
                        itemList.push_back(60301);
                        itemList.push_back(59348);
                        itemList.push_back(59517);
                        itemList.push_back(59121);
                        itemList.push_back(58181);
                        itemList.push_back(59520);
                        itemList.push_back(67136);
                        itemList.push_back(59122);
                        itemList.push_back(59122);
                        itemList.push_back(59320);
                        break;
                }
                break;
            }
            case CLASS_PRIEST:
            {
                switch (specialisation)
                {
                    case SP_TANK:
                    case SP_DPS_CAC:
                    case SP_HEAL:
                        itemList.push_back(60262);
                        itemList.push_back(60261);
                        itemList.push_back(60275);
                        itemList.push_back(59234);
                        itemList.push_back(59475);
                        itemList.push_back(59349);
                        itemList.push_back(60258);
                        itemList.push_back(60259);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(59501);
                        itemList.push_back(58184);
                        itemList.push_back(58183);
                        itemList.push_back(58189);
                        itemList.push_back(59525);
                        itemList.push_back(69607);
                        break;
                    case SP_DPS_CAST:
                        itemList.push_back(60253);
                        itemList.push_back(60255);
                        itemList.push_back(60257);
                        itemList.push_back(58485);
                        itemList.push_back(59475);
                        itemList.push_back(59349);
                        itemList.push_back(60256);
                        itemList.push_back(60254);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(58188);
                        itemList.push_back(59519);
                        itemList.push_back(58183);
                        itemList.push_back(67129);
                        itemList.push_back(59525);
                        itemList.push_back(69631);
                        break;
                }
                break;
            }
            case CLASS_MAGE:
            {
                switch (specialisation)
                {
                    case SP_HEAL:
                    case SP_DPS_CAC:
                    case SP_TANK:
                    case SP_DPS_CAST:
                        itemList.push_back(60246);
                        itemList.push_back(60245);
                        itemList.push_back(60247);
                        itemList.push_back(58485);
                        itemList.push_back(59475);
                        itemList.push_back(59349);
                        itemList.push_back(60243);
                        itemList.push_back(60244);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(58188);
                        itemList.push_back(59519);
                        itemList.push_back(58183);
                        itemList.push_back(67129);
                        itemList.push_back(59463);
                        itemList.push_back(69631);
                        break;
                }
                break;
            }
            case CLASS_WARLOCK:
            {
                switch (specialisation)
                {
                    case SP_DPS_CAC:
                    case SP_TANK:
                    case SP_DPS_CAST:
                    case SP_HEAL:
                        itemList.push_back(60252);
                        itemList.push_back(60250);
                        itemList.push_back(60248);
                        itemList.push_back(58485);
                        itemList.push_back(59475);
                        itemList.push_back(59349);
                        itemList.push_back(60249);
                        itemList.push_back(60251);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(58188);
                        itemList.push_back(59519);
                        itemList.push_back(58183);
                        itemList.push_back(67129);
                        itemList.push_back(59463);
                        itemList.push_back(69631);
                        break;
                }
                break;
            }
            case CLASS_DRUID:
            {
                switch (specialisation)
                {
                    case SP_HEAL:
                        itemList.push_back(60280);
                        itemList.push_back(60278);
                        itemList.push_back(60279);
                        itemList.push_back(59495);
                        itemList.push_back(59451);
                        itemList.push_back(59321);
                        itemList.push_back(60277);
                        itemList.push_back(60276);
                        itemList.push_back(58194);
                        itemList.push_back(59483);
                        itemList.push_back(58189);
                        itemList.push_back(58184);
                        itemList.push_back(58183);
                        itemList.push_back(59220);
                        itemList.push_back(64673);
                        itemList.push_back(59525);
                        break;
                    case SP_DPS_CAC:
                        itemList.push_back(60289);
                        itemList.push_back(60288);
                        itemList.push_back(60290);
                        itemList.push_back(59469);
                        itemList.push_back(59329);
                        itemList.push_back(59502);
                        itemList.push_back(60286);
                        itemList.push_back(60287);
                        itemList.push_back(59348);
                        itemList.push_back(59517);
                        itemList.push_back(59121);
                        itemList.push_back(58181);
                        itemList.push_back(59520);
                        itemList.push_back(67136);
                        itemList.push_back(64671);
                        itemList.push_back(59474);
                        break;
                    case SP_TANK:
                        itemList.push_back(60289);
                        itemList.push_back(60288);
                        itemList.push_back(60290);
                        itemList.push_back(59469);
                        itemList.push_back(59329);
                        itemList.push_back(59502);
                        itemList.push_back(60286);
                        itemList.push_back(60287);
                        itemList.push_back(59348);
                        itemList.push_back(59517);
                        itemList.push_back(59121);
                        itemList.push_back(58181);
                        itemList.push_back(59520);
                        itemList.push_back(67136);
                        itemList.push_back(64671);
                        itemList.push_back(59474);
                        break;
                    case SP_DPS_CAST:
                        itemList.push_back(60284);
                        itemList.push_back(60283);
                        itemList.push_back(60285);
                        itemList.push_back(59495);
                        itemList.push_back(59451);
                        itemList.push_back(59321);
                        itemList.push_back(60282);
                        itemList.push_back(60281);
                        itemList.push_back(59457);
                        itemList.push_back(59512);
                        itemList.push_back(59501);
                        itemList.push_back(59519);
                        itemList.push_back(58183);
                        itemList.push_back(67129);
                        itemList.push_back(64672);
                        itemList.push_back(59525);
                        break;
                }
                break;
            }
            case CLASS_DEATH_KNIGHT:
            {
                switch (specialisation)
                {
                    case SP_TANK:
                        itemList.push_back(60353);
                        itemList.push_back(60352);
                        itemList.push_back(60350);
                        itemList.push_back(59328);
                        itemList.push_back(59470);
                        itemList.push_back(59117);
                        itemList.push_back(60351);
                        itemList.push_back(60349);
                        itemList.push_back(59466);
                        itemList.push_back(59319);
                        itemList.push_back(59233);
                        itemList.push_back(59332);
                        itemList.push_back(58182);
                        itemList.push_back(58187);
                        itemList.push_back(64676);
                        itemList.push_back(59330);
                        break;
                    case SP_HEAL:
                    case SP_DPS_CAC:
                    case SP_DPS_CAST:
                        itemList.push_back(60343);
                        itemList.push_back(60342);
                        itemList.push_back(60340);
                        itemList.push_back(59221);
                        itemList.push_back(59118);
                        itemList.push_back(59342);
                        itemList.push_back(60341);
                        itemList.push_back(60339);
                        itemList.push_back(59507);
                        itemList.push_back(67138);
                        itemList.push_back(59518);
                        itemList.push_back(59224);
                        itemList.push_back(58180);
                        itemList.push_back(58185);
                        itemList.push_back(64674);
                        itemList.push_back(59330);
                        break;
                }
                break;
            }
            case CLASS_WARRIOR:
            {
                switch (specialisation)
                {
                    case SP_DPS_CAST:
                    case SP_HEAL:
                    case SP_DPS_CAC:
                        itemList.push_back(60327);
                        itemList.push_back(60324);
                        itemList.push_back(60326);
                        itemList.push_back(59221);
                        itemList.push_back(59118);
                        itemList.push_back(59342);
                        itemList.push_back(60325);
                        itemList.push_back(60323);
                        itemList.push_back(59507);
                        itemList.push_back(59442);
                        itemList.push_back(59518);
                        itemList.push_back(58181);
                        itemList.push_back(59506);
                        itemList.push_back(58185);
                        itemList.push_back(59330);
                        itemList.push_back(59320);
                        itemList.push_back(59330);
                        break;
                    case SP_TANK:
                        itemList.push_back(60331);
                        itemList.push_back(60330);
                        itemList.push_back(60332);
                        itemList.push_back(59328);
                        itemList.push_back(59470);
                        itemList.push_back(59117);
                        itemList.push_back(60328);
                        itemList.push_back(60329);
                        itemList.push_back(59466);
                        itemList.push_back(59319);
                        itemList.push_back(59233);
                        itemList.push_back(59332);
                        itemList.push_back(58182);
                        itemList.push_back(58187);
                        itemList.push_back(59347);
                        itemList.push_back(64676);
                        itemList.push_back(59444);
                        itemList.push_back(59320);
                        itemList.push_back(59319);
                        break;
                }
                break;
            }
            case CLASS_HUNTER:
            {
                switch (specialisation)
                {
                    case SP_HEAL:
                    case SP_DPS_CAC:
                    case SP_TANK:
                    case SP_DPS_CAST:
                        itemList.push_back(60306);
                        itemList.push_back(60305);
                        itemList.push_back(60307);
                        itemList.push_back(58199);
                        itemList.push_back(59355);
                        itemList.push_back(59485);
                        itemList.push_back(60303);
                        itemList.push_back(60304);
                        itemList.push_back(59348);
                        itemList.push_back(59517);
                        itemList.push_back(59121);
                        itemList.push_back(59473);
                        itemList.push_back(58181);
                        itemList.push_back(67136);
                        itemList.push_back(69843);
                        itemList.push_back(59320);
                        break;
                }
                break;
            }
        }
    }
}


bool Migration::LearnPrimaryProfession(Player *newChar, uint32 profId, uint32 newValue, bool isGuild)
{
    uint32 spellList[7] = { 0 };
    switch (profId)
    {
        case 171:
            spellList[0] = 2275; spellList[1] = 2280; spellList[2] = 3465;
            spellList[3] = 11612; spellList[4] = 28597; spellList[5] = 51303;
            spellList[6] = 80731;
            break;
        case 164:
            spellList[0] = 2020; spellList[1] = 2021; spellList[2] = 3539;
            spellList[3] = 9786; spellList[4] = 29845; spellList[5] = 51298;
            spellList[6] = 76666;
            break;
        case 333:
            spellList[0] = 7414; spellList[1] = 7415; spellList[2] = 7416;
            spellList[3] = 13921; spellList[4] = 28030; spellList[5] = 51312;
            spellList[6] = 74258;
            break;
        case 202:
            spellList[0] = 4039; spellList[1] = 4040; spellList[2] = 4041;
            spellList[3] = 12657; spellList[4] = 30351; spellList[5] = 51305;
            spellList[6] = 82774;
            break;
        case 182:
            spellList[0] = 2372; spellList[1] = 2373; spellList[2] = 3571;
            spellList[3] = 11994; spellList[4] = 28696; spellList[5] = 50301;
            spellList[6] = 74519;
            break;
        case 773:
            spellList[0] = 45375; spellList[1] = 45376; spellList[2] = 45377;
            spellList[3] = 45378; spellList[4] = 45379; spellList[5] = 45380;
            spellList[6] = 86008;
            break;
        case 755:
            spellList[0] = 25245; spellList[1] = 25246; spellList[2] = 28896;
            spellList[3] = 28899; spellList[4] = 28901; spellList[5] = 51310;
            spellList[6] = 73318;
            break;
        case 165:
            spellList[0] = 2155; spellList[1] = 2154; spellList[2] = 3812;
            spellList[3] = 10663; spellList[4] = 32550; spellList[5] = 51301;
            spellList[6] = 81199;
            break;
        case 186:
            spellList[0] = 2581; spellList[1] = 2582; spellList[2] = 3568;
            spellList[3] = 10249; spellList[4] = 29355; spellList[5] = 50309;
            spellList[6] = 74517;
            break;
        case 393:
            spellList[0] = 8615; spellList[1] = 8619; spellList[2] = 8620;
            spellList[3] = 10769; spellList[4] = 32679; spellList[5] = 50307;
            spellList[6] = 74522;
            break;
        case 197:
            spellList[0] = 3911; spellList[1] = 3912; spellList[2] = 3913;
            spellList[3] = 12181; spellList[4] = 26791; spellList[5] = 51308;
            spellList[6] = 75156;
            break;
        default:
            return false;
    }
    uint16 currValue = newChar->GetPureSkillValue(profId);
    if (!currValue)
        if (!(newChar->GetFreePrimaryProfessionPoints() > 0))
            return false;
    if (newValue > 525) // Max value
        newValue = 525;

    uint8 step = uint8(ceil(float(newValue) / 75.0f));
    for (uint8 i = 0; i < step; i++)
    {
        SpellInfo const* spellinfo = sSpellMgr->GetSpellInfo(spellList[i]);
        if (!spellinfo)
            continue;

        bool cast = false;
        for (uint8 eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
        {
            if (spellinfo->Effects[eff].Effect != SPELL_EFFECT_LEARN_SPELL)
                continue;

            cast = true;
        }

        if (cast)
            newChar->CastSpell(newChar, spellList[i], true);
        else
            newChar->learnSpell(spellList[i], false);
    }

    newValue = (newValue > newChar->GetPureMaxSkillValue(profId)) ? newChar->GetPureMaxSkillValue(profId) : newValue;

    AddProfesionItems(profId, newChar, isGuild);
    LearnAdditionalProfessionSpells(newChar, isGuild, profId, newValue);

    if (newValue < newChar->GetPureSkillValue(profId))
        return true;

    newValue -= newChar->GetPureSkillValue(profId);
    newChar->UpdateSkillPro(profId, 1001, newValue);
    return true;
}

bool Migration::LearnSecondaryProfession(Player *newChar, uint32 profId, uint32 newValue, bool isGuild)
{
    uint32 spellList[7] = { 0 };
    switch (profId)
    {
    case 1:
        if (LearnSecondaryProfession(newChar, 185, newValue, isGuild) && LearnSecondaryProfession(newChar, 356, newValue, isGuild)
            && LearnSecondaryProfession(newChar, 129, newValue, isGuild) && LearnSecondaryProfession(newChar, 794, newValue, isGuild))
            return true;

        return false;
    case 185:
        spellList[0] = 2550; spellList[1] = 3102; spellList[2] = 3413;
        spellList[3] = 18260; spellList[4] = 33359; spellList[5] = 51296;
        spellList[6] = 88053;
        break;
    case 356:
        spellList[0] = 7733; spellList[1] = 7734; spellList[2] = 54083;
        spellList[3] = 18249; spellList[4] = 54084; spellList[5] = 51293;
        spellList[6] = 88869;
        break;
    case 129:
        spellList[0] = 3273; spellList[1] = 3280; spellList[2] = 54254;
        spellList[3] = 10847; spellList[4] = 54255; spellList[5] = 65292;
        spellList[6] = 74559;
        break;
    case 794:
        spellList[0] = 95553; spellList[1] = 95554; spellList[2] = 95555;
        spellList[3] = 95556; spellList[4] = 89720; spellList[5] = 89721;
        spellList[6] = 95546;
        break;
    default: return false;
    }

    if (newValue > 525) // Max value
        newValue = 525;


    uint8 step = uint8(ceil(float(newValue) / 75.0f));
    for (uint8 i = 0; i < step; i++)
    {
        SpellInfo const* spellinfo = sSpellMgr->GetSpellInfo(spellList[i]);
        if (!spellinfo)
            continue;

        bool cast = false;
        for (uint8 eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
        {
            if (spellinfo->Effects[eff].Effect != SPELL_EFFECT_LEARN_SPELL)
                continue;

            cast = true;
        }

        if (cast)
            newChar->CastSpell(newChar, spellList[i], true);
        else
            newChar->learnSpell(spellList[i], false);
    }

    newValue = (newValue > newChar->GetPureMaxSkillValue(profId)) ? newChar->GetPureMaxSkillValue(profId) : newValue;

    AddProfesionItems(profId, newChar, isGuild);
    LearnAdditionalProfessionSpells(newChar, isGuild, profId, newValue);

    if (newValue < newChar->GetPureSkillValue(profId))
        return true;

    newValue -= newChar->GetPureSkillValue(profId);
    newChar->UpdateSkillPro(profId, 1001, newValue);
    return true;
}

char **Migration::my_explode(char *str, char separator)
{
    char **res = NULL;
    int  nbstr = 1;
    int  len;
    int  from = 0;
    int  i;
    int  j;
    res = (char **) malloc(sizeof (char *));
    len = strlen(str);
    for (i = 0; i <= len; ++i)
    {
        if ((i == len) || (str[i] == separator))
        {
            res = (char **) realloc(res, ++nbstr * sizeof (char *));
            res[nbstr - 2] = (char *) malloc((i - from + 1) * sizeof (char));
            for (j = 0; j < (i - from); ++j)
                res[nbstr - 2][j] = str[j + from];
            res[nbstr - 2][i - from] = '\0';
            from = i + 1;
            ++i;
        }
    }
    res[nbstr - 1] =  NULL;
    return res;
}

void Migration::RecupStoreNewItemInBestSlots(uint32 itemEntry, uint32 count, Player *player)
{
    if (!player->StoreNewItemInBestSlots(itemEntry, count))
    {
        if (ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(itemEntry))
            if (Item* item = Item::CreateItem(itemEntry, 1, player))
            {
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                std::string subject = player->GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                MailDraft draft(subject, "Certains items ont eu un problème.");
                item->SaveToDB(trans);
                draft.AddItem(item);
                draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);
                CharacterDatabase.CommitTransaction(trans);
            }
    }
}

void Migration::AddProfesionItems(uint32 skill_id, Player *player, bool isGuild)
{
    std::list<eMigrationSpecialItems > pitm = migrationProfesionItems[skill_id];
    if (pitm.empty())
        return;
    for (std::list<eMigrationSpecialItems >::iterator itr = pitm.begin(); itr != pitm.end(); itr++)
    {
        eMigrationSpecialItems i = *itr;
        ItemTemplate const *item = sObjectMgr->GetItemTemplate(i.entry);
        if (item != NULL)
            if ((i.faction == 0 || (i.faction == player->GetTeam())) && (i.is_guild == 0 || isGuild == true))
                RecupStoreNewItemInBestSlots(i.entry, i.quantity, player);
    }
    if (skill_id == 755 && isGuild)
        player->ModifyCurrency(361, 15);
}

void Migration::AddSpecialItems(Player *player, bool isGuild)
{
    if (migrationSpecialItems.empty())
        return;
    for (std::list<eMigrationSpecialItems >::iterator itr = migrationSpecialItems.begin(); itr != migrationSpecialItems.end(); itr++)
    {
        eMigrationSpecialItems i = *itr;
        ItemTemplate const *item = sObjectMgr->GetItemTemplate(i.entry);
        if (item != NULL)
            if ((i.faction == 0 || (i.faction == player->GetTeam())) && (i.is_guild == 0 || isGuild == true))
                RecupStoreNewItemInBestSlots(i.entry, i.quantity, player);
    }
}

void Migration::LearnAdditionalProfessionSpells(Player *player, bool isGuild, uint32 skillId, uint32 skillLevel)
{
    if (migrationProfessionSpells.empty())
        return;
    for (std::list<eMigrationProfessionSpells >::iterator itr = migrationProfessionSpells.begin(); itr != migrationProfessionSpells.end(); itr++)
    {
        eMigrationProfessionSpells i = *itr;
        if (i.reqskill == skillId && i.reqskillvalue <= skillLevel && player->getLevel() > i.reqlevel)
        {
            SpellInfo const* spellinfo = sSpellMgr->GetSpellInfo(i.entry);
            if (!spellinfo)
                continue;

            bool cast = false;
            for (uint8 eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
            {
                if (spellinfo->Effects[eff].Effect != SPELL_EFFECT_LEARN_SPELL)
                    continue;

                cast = true;
            }

            if (!cast && !player->HasSpell(i.entry))
                player->learnSpell(i.entry, false);
        }
    }
}

void Migration::UpdateProfessions(const char *r_skills, Player *player, bool isGuild)
{
    char **t_skills = my_explode((char*)r_skills, ';');
    char **my_skill;
    for (int i = 0; t_skills[i]; i++)
    {
        my_skill = my_explode(t_skills[i], ' ');
        if (my_skill[0] && my_skill[1])
        {
            uint16 skill_id = static_cast<uint16>(atoi(my_skill[0]));
            uint16 skill_level = static_cast<uint16>(atoi(my_skill[1]));
            uint16 skill_max = player->GetMaxSkillValueForLevel();
            uint16 skill_step = player->GetSkillStep(skill_id);
            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill_id);
            if (pSkill && pSkill->categoryId == SKILL_CATEGORY_PROFESSION)
                LearnPrimaryProfession(player, skill_id, skill_level, isGuild);
            else if (pSkill && (skill_id == SKILL_FISHING || skill_id == SKILL_COOKING || skill_id == SKILL_FIRST_AID || skill_id == SKILL_ARCHAEOLOGY))
                LearnSecondaryProfession(player, skill_id, skill_level, isGuild);

        }
    }
}

void Migration::UpdateReputation(const char *r_reputs, Player *player)
{
    char **t_reputs = my_explode((char*)r_reputs, ';');
    char **my_reput;
    const FactionEntry *factionEntry;
    for (int i = 0; t_reputs[i]; i++)
    {
        my_reput = my_explode(t_reputs[i], ' ');
        if (my_reput[0] && my_reput[1])
        {
            uint32 factionId = (uint32)atoi(my_reput[0]);;
            factionEntry = sFactionStore.LookupEntry(factionId);

            if(!factionEntry || factionEntry->reputationListID < 0)
                ChatHandler(player->GetSession()).PSendSysMessage("Reputation inexistante. Passee.");
            else
            {
                int32 repChange = (int32)atoi(my_reput[1]);
                ReputationRank rank = player->GetReputationMgr().GetRank(factionEntry);
                if (rank == REP_HATED)
                    repChange += 42000;
                else if (rank == REP_HOSTILE)
                    repChange += 6000;
                else if (rank == REP_UNFRIENDLY)
                    repChange += 3000;
                repChange = player->CalculateReputationGain(REPUTATION_SOURCE_SPELL, 0, repChange, factionId);
                player->GetReputationMgr().ModifyReputation(factionEntry, repChange);
            }
        }
    }
}

void Migration::UpdateMountAndDbleSpe(Player *player)
{
    // mount competence
    if (player->getLevel() >= 60)
    {
        player->learnSpell(34091, false);
        player->learnSpell(90267, false);
    }
    if (player->getLevel() >= 68)
    {
        player->learnSpell(54197, false);
    }
    // dble specialization
    if (player->getLevel() >= 80)
    {
        player->CastSpell(player, 63680, true, NULL, NULL, player->GetGUID());
        player->CastSpell(player, 63624, true, NULL, NULL, player->GetGUID());
    }
}

void Migration::UpdateClassSkills(Player *player)
{
    switch (player->getClass())
    {
        case CLASS_PALADIN:
        case CLASS_WARRIOR:
            player->learnSpell(750, false);
            break;
        case CLASS_SHAMAN:
        case CLASS_HUNTER:
            player->learnSpell(8737, false);
            break;
        case CLASS_ROGUE:
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        case CLASS_DRUID:
            break;
        case CLASS_DEATH_KNIGHT:
            player->CastSpell(player, 53431, true);
            break;
    }
    player->UpdateSkillsToMaxSkillsForLevel();
}

void Migration::ValidateMigration(uint32 r_guid)
{
    WebDatabase.PExecute("UPDATE recups_cata SET status=3 WHERE id=%u", r_guid);
}

void Migration::UpdateItemsAndGolds(Player *player, uint32 packId, uint32 specialization, bool isGuild)
{
    // bag
    RecupStoreNewItemInBestSlots(MIGRATION_BAGS, 4, player);

    // unequip items
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        if (Item* offItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            ItemPosCountVec off_dest;
            uint8 off_msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false);
            if (off_msg == EQUIP_ERR_OK)
            {
                player->RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                player->StoreItem(off_dest, offItem, true);
            }
            else
            {
                player->MoveItemFromInventory(INVENTORY_SLOT_BAG_0, i, true);
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                offItem->DeleteFromInventoryDB(trans);
                offItem->SaveToDB(trans);
                std::string subject = player->GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                MailDraft(subject, "There were problems with equipping one or several items").AddItem(offItem).SendMailTo(trans, player,
                                                                                                                          MailSender(player, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);
                CharacterDatabase.CommitTransaction(trans);
            }
        }

    // equip and store pack items
    std::list<uint32 > itemList;
    GetMigrationItemList(itemList, packId, player, specialization, isGuild);
    for (std::list<uint32 >::iterator itr = itemList.begin(); itr != itemList.end(); itr++)
        RecupStoreNewItemInBestSlots(*itr, 1, player);

    // custom xp bonus
    RecupStoreNewItemInBestSlots(CUSTOM_XP_BONUS_ITEM, 1, player);

    // mounts
    RecupStoreNewItemInBestSlots(player->GetTeam() == ALLIANCE ? MOUNT_ALLIANCE : MOUNT_HORDE, 1, player);

    // golds
    player->ModifyMoney(MIGRATION_GOLDS);
}

void Migration::TeleportToCapital(Player *player)
{
    if (player->GetTeam() == ALLIANCE)
        player->TeleportTo(0, -8960.14f, 516.266f, 96.3568f, 0.0f);
    else
        player->TeleportTo(1, 1552.5f, -4420.66f, 19.94802f, 0.0f);
}

void Migration::UpdateRaceSkills(Player *player)
{
    switch (player->getRace())
    {
        case RACE_WORGEN:
            player->CastSpell(player, 95759, true);
            player->CastSpell(player, 72792, true);
            player->learnSpell(68996, false);
            break;
        default:
            break;
    }
}

bool Migration::AbortMigration(MigrationErrors err, uint32 r_guid, Player *player)
{
    switch (err)
    {
        case ERROR_PACKID:
        {
            std::string subject = "Votre recuperation.";
            std::string text = "Staff\nCeci est un message automatique\n votre recuperation n as pu etre traitee en raison d un invalide choix de specialisation\n merci d effectuer les corrections necessaires et de renvoyer votre recuperation a validation.\n Cordialement \n Le Staff Paragon.";
            MailSender sender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM);
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            MailDraft(subject, text).SendMailTo(trans, player, sender, MAIL_CHECK_MASK_COPIED);
            CharacterDatabase.CommitTransaction(trans);
            std::string mess = text;
            WebDatabase.PExecute("UPDATE recups_cata SET status=0 WHERE id=%u", r_guid);
            break;
        }
    }
    return false;
}

void Migration::UpdateLevel(Player *player, uint8 level)
{
    if (level > GetMaxLevelForExpansion(CONTENT_81_85))
        level = GetMaxLevelForExpansion(CONTENT_81_85);
    player->SetLevel(level);
}

void Migration::ForceQuest(Player *player, uint32 questId)
{
    if (Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId))
        if (player->SatisfyQuestRace(qInfo, false) && player->SatisfyQuestClass(qInfo, false) && player->SatisfyQuestReputation(qInfo, false))
        {
            player->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
            if (uint32 talents = qInfo->GetBonusTalents())
            {
                player->learnQuestRewardedSpells(qInfo);
                player->AddQuestRewardedTalentCount(talents);
            }
        }
}

void Migration::CompleteClassQuests(Player *player)
{
    switch (player->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            ForceQuest(player, 12801);
            ForceQuest(player, 12779);
            ForceQuest(player, 12678);
            ForceQuest(player, 12679);
            ForceQuest(player, 12680);
            ForceQuest(player, 12687);
            ForceQuest(player, 12698);
            ForceQuest(player, 12701);
            ForceQuest(player, 12706);
            ForceQuest(player, 12716);
            ForceQuest(player, 12719);
            ForceQuest(player, 12720);
            ForceQuest(player, 12722);
            ForceQuest(player, 12724);
            ForceQuest(player, 12725);
            ForceQuest(player, 12727);
            ForceQuest(player, 12733);
            ForceQuest(player, 12739);
            ForceQuest(player, 12742);
            ForceQuest(player, 12743);
            ForceQuest(player, 12744);
            ForceQuest(player, 12745);
            ForceQuest(player, 12746);
            ForceQuest(player, 12747);
            ForceQuest(player, 12748);
            ForceQuest(player, 12749);
            ForceQuest(player, 12750);
            ForceQuest(player, 12751);
            ForceQuest(player, 12754);
            ForceQuest(player, 12755);
            ForceQuest(player, 12756);
            ForceQuest(player, 12757);
            ForceQuest(player, 28649);
            ForceQuest(player, 28650);
            break;
        default:
            break;
    }
}

void Migration::RollCustomMount(Player *player)
{
    int r = rand() % 100;
    if (r == 42)
    {

    }
    switch (rand() % 3)
    {
    case 0:
        RecupStoreNewItemInBestSlots(76755, 1, player);
        break;
    case 1:
        RecupStoreNewItemInBestSlots(37676, 1, player);
        break;
    case 2:
        RecupStoreNewItemInBestSlots(43954, 1, player);
        break;
    case 3:
        RecupStoreNewItemInBestSlots(49636, 1, player);
        break;
    default:
        RecupStoreNewItemInBestSlots(37676, 1, player);
        break;
    }
}

bool Migration::ExecuteMigration(Player* player, eMigrationChar & migrInfos)
{
    uint32 charguid = player->GetGUIDLow();
    uint32 acctid = player->GetSession()->GetAccountId();
    if (migrInfos.r_charguid == charguid && migrInfos.r_acctid == acctid)
    {
        uint32 r_guid = migrInfos.r_guid;
        uint8 level =  migrInfos.level;
        uint32 classId = player->getClass();
        uint32 specialisation = migrInfos.specialisation;
        uint32 guild = migrInfos.guild;
        std::string r_skills = migrInfos.r_skills;
        std::string r_reputs = migrInfos.r_reputs;

        uint32 packId = 0;
        if (level < 85)
        {
            packId = findPack(player->GetTeam(), specialisation, level, guild, classId);
            if (packId == 0)
                return AbortMigration(ERROR_PACKID, r_guid, player);
        }

        UpdateLevel(player, level);

        if (!r_skills.empty())
            UpdateProfessions(r_skills.c_str(), player,  guild > 0 ? true : false);

        if (!r_reputs.empty())
            UpdateReputation(r_reputs.c_str(), player);

        UpdateMountAndDbleSpe(player);
        UpdateClassSkills(player);
        UpdateRaceSkills(player);
        CompleteClassQuests(player);
        player->InitTalentForLevel();
        UpdateItemsAndGolds(player, packId, specialisation, guild > 0 ? true : false);
        AddSpecialItems(player, guild > 0 ? true : false);
        if (guild > 0)
            RollCustomMount(player);
        LearnAllClassSpells(player);
        TeleportToCapital(player);

        player->UpdateMaxHealth();
        player->SetFullHealth();
        player->ActivateSpec(1); // force the unlearn of talent spells linked.
        player->ActivateSpec(0);
        player->SaveToDB();
        ValidateMigration(r_guid);
        ChatHandler(player->GetSession()).PSendSysMessage("%s, nous vous souhaitons la bienvenue sur Paragon Servers, le Staff se permet de vous offrir un booster xp et réputation que vous trouverez dans votre sac. Bon jeu chez nous");
        return true;
    }
    return false;
}

void Migration::RefreshMigrationList()
{
    {
        TRINITY_GUARD(ACE_Thread_Mutex, LockMigrationList);
        uint32 oldMSTime = getMSTime();
        TC_LOG_INFO("loading", "Refresh Migration List...");
        migrationsChar.clear();
        if (QueryResult result = WebDatabase.PQuery("SELECT perso, account, id, level, spe, guild, skills, reputs FROM recups_cata WHERE status = 2 ORDER BY id ASC"))
        {
            uint32 count = 0;
            uint32 doneMigrCount = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 charGUID = fields[0].GetUInt32();
                eMigrationChar tempCharMigr(charGUID, fields[1].GetUInt32(), fields[2].GetUInt32(), fields[3].GetUInt32(), fields[4].GetUInt32(),
                                            fields[5].GetUInt32(), fields[6].GetString(), fields[7].GetString());
                if (doneMigrCount < MAX_MIGRATION_PER_REFRESH)
                    if (Player *player = sObjectAccessor->FindPlayer(MAKE_NEW_GUID(charGUID, 0, HIGHGUID_PLAYER)))
                        if (player->IsInWorld())
                        {
                            ExecuteMigration(player, tempCharMigr);
                            doneMigrCount++;
                            continue;
                        }
                std::pair<uint32, eMigrationChar > res;
                res.second = tempCharMigr;
                res.first = charGUID;
                migrationsChar.push_back(res);
                count++;
            }
            while (result->NextRow());
            TC_LOG_INFO("loading", ">> Refreash %u valid migrations in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
            TC_LOG_INFO("loading", ">> %u migration have been completed in %u ms", doneMigrCount, GetMSTimeDiffToNow(oldMSTime));
        }
    }
}

class IsExistingMigration
{
public:
    IsExistingMigration(uint32 guid) : _guid(guid) {};
    bool operator () ( const std::pair<uint32 , eMigrationChar > &elem )
    {
        return elem.first == _guid;
    }
private:
    uint32 _guid;
};

void Migration::CheckValidMigration(Player *player)
{
    {
        TRINITY_GUARD(ACE_Thread_Mutex, LockMigrationList);
        std::list<std::pair<uint32 , eMigrationChar > >::iterator itr = std::find_if(migrationsChar.begin(), migrationsChar.end(), IsExistingMigration(player->GetGUIDLow()));
        if (itr !=  migrationsChar.end())
        {
            ExecuteMigration(player, itr->second);
            migrationsChar.erase(itr);
        }
    }
}

void Migration::LearnAllClassSpells(Player *player)
{
    for (std::list< eMigrationSpells >::iterator itr =  migrationSpells.begin(); itr != migrationSpells.end(); itr++)
    {
        eMigrationSpells spellCarac = *itr;
        if (player->getClass() == spellCarac.classID && player->getLevel() >= spellCarac.levelReq && (player->GetTeam() == spellCarac.factionID
                                                                                                      || spellCarac.factionID == BOTH_FACT))
            player->learnSpell(spellCarac.spellID, false);
    }
}

void Migration::Update(const uint32 &diff)
{
    if (m_migrationRefreshTimer <= diff)
    {
        RefreshMigrationList();
        m_migrationRefreshTimer = MIGRATION_REFRESH_TIMER;
    }
    else
        m_migrationRefreshTimer -= diff;
}

std::map<uint32, Bag *> Migration::FillWotlkBags(Player *player)
{
    uint32 wotlkGuid = player->GetWotlkGUID();
    player->SetBankBagSlotCount(7);
    std::map<uint32, Bag *> bag;
    if (QueryResult result = WotlkCharacterDatabase.PQuery("SELECT itemEntry, COUNT, slot, item_instance.guid FROM item_instance JOIN character_inventory ON character_inventory.item = item_instance.`guid` WHERE duration = 0 AND owner_guid = %u AND ((slot >= 67 && slot <= 74) || (slot >= 19 && slot <= 23)) AND bag = 0;", wotlkGuid))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 itemEntry =  fields[0].GetUInt32();
            uint32 itemCount =  fields[1].GetUInt32();
            uint32 itemSlot =  fields[2].GetUInt8();
            uint32 itemGUID =  fields[3].GetUInt32();
            ItemTemplate const *TestItem = sObjectMgr->GetItemTemplate(itemEntry);
            if (TestItem == NULL)
                continue;
            uint16 eDest;
            InventoryResult msg = player->CanEquipNewItem(itemSlot, eDest, itemEntry, false);

            if (msg == EQUIP_ERR_OK)
            {
                player->EquipNewItem(eDest, itemEntry, true);
                continue;
            }

            ItemPosCountVec dest;
            msg = player->CanStoreNewItem(NULL_BAG, itemSlot, dest, itemEntry, itemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item *item = player->StoreNewItem(dest, itemEntry, true);
                player->SendNewItem(item, itemCount, true, false);
                bag[itemGUID] = item->ToBag();
                continue;
            }

            if (player->IsBankPos(INVENTORY_SLOT_BAG_0, itemSlot))
            {
                ItemPosCountVec destBank;
                Item* pItem = Item::CreateItem(itemEntry, itemCount, player);
                if (pItem)
                {
                    msg = player->CanBankItem(NULL_BAG, itemSlot, destBank, pItem, false);
                    if (msg == EQUIP_ERR_OK)
                    {
                        player->BankItem(destBank, pItem, true);
                        continue;
                    }
                }
            }


            RecupStoreNewItemInBestSlots(itemEntry, itemCount, player);
            count++;
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Synchronise %u bags success", count);
    }
    return bag;
}

void Migration::FillWotlkItems(Player *player)
{
    uint32 wotlkGuid = player->GetWotlkGUID();
    std::map<uint32, Bag *> bags = FillWotlkBags(player);
    if (QueryResult result = WotlkCharacterDatabase.PQuery("SELECT itemEntry, COUNT, slot, bag FROM item_instance JOIN character_inventory ON character_inventory.item = item_instance.`guid` WHERE duration = 0 AND owner_guid = %u AND (bag != 0 || (slot < 19 || (slot > 23 && slot < 67) || slot > 74));", wotlkGuid))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 itemEntry =  fields[0].GetUInt32();
            uint32 itemCount =  fields[1].GetUInt32();
            uint32 itemSlot =  fields[2].GetUInt8();
            uint32 bag =  fields[3].GetUInt32();
            ItemTemplate const *TestItem = sObjectMgr->GetItemTemplate(itemEntry);
            if (TestItem == NULL || itemEntry >= 1000000)
                continue;
            ItemPosCountVec dest;
            uint8 bagSlot = NULL_BAG;
            if (Bag *currentBag = bags[bag])
                bagSlot = currentBag->GetSlot();
            InventoryResult msg = player->CanStoreNewItem(bagSlot, itemSlot, dest, itemEntry, itemCount);
            if (msg != EQUIP_ERR_OK)
            {
                RecupStoreNewItemInBestSlots(itemEntry, itemCount, player);
                continue;
            }
            Item *item = player->StoreNewItem(dest, itemEntry, true);
            player->SendNewItem(item, itemCount, true, false);
            count++;
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Synchronise %u bags success", count);
    }
}

void Migration::FillWotlkAchievements(Player *player)
{
    uint32 wotlkGuid = player->GetWotlkGUID();
    if (QueryResult result = WotlkCharacterDatabase.PQuery("SELECT achievement, date FROM character_achievement WHERE guid = %u", wotlkGuid))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 achievement =  fields[0].GetUInt32();
            uint32 date =  fields[1].GetUInt32();
            if (AchievementEntry const* achievementEntry = sAchievementMgr->GetAchievement(achievement))
                if (!player->HasAchieved(achievement))
                {
                    player->CompletedAchievement(achievementEntry);
                    count++;
                }
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Synchronise %u achievement", count);
    }

}

void Migration::FillWotlkSpells(Player *player)
{
    uint32 wotlkGuid = player->GetWotlkGUID();
    if (QueryResult result = WotlkCharacterDatabase.PQuery("SELECT spell FROM character_spell WHERE guid = %u", wotlkGuid))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 spellId =  fields[0].GetUInt32();
            SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);
            if (!spell)
                continue;
            if (spell->HasEffect(SPELL_EFFECT_LEARN_SPELL))
                continue;

            if (spell->HasEffect(SPELL_EFFECT_SUMMON))
            {
                player->learnSpell(spellId, false);
                continue;
            }

            for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (spell->Effects[j].ApplyAuraName == SPELL_AURA_MOUNTED)
                {
                    player->learnSpell(spellId, false);
                    break;
                }
            }
        }
        while (result->NextRow());
    }
}

void Migration::FillProfessions(Player *player)
{
    uint32 wotlkGuid = player->GetWotlkGUID();
    if (QueryResult result = WotlkCharacterDatabase.PQuery("SELECT skill, value FROM character_skills WHERE guid = %u", wotlkGuid))
    {
        uint32 count = 0;
        uint32 countPrimaryProfessionCount = 0;
        do
        {
            Field* fields = result->Fetch();
            uint16 skill_id = fields[0].GetUInt32();
            uint16 skill_level = fields[1].GetUInt32();
            uint16 skill_max = player->GetMaxSkillValueForLevel();
            uint16 skill_step = player->GetSkillStep(skill_id);
            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill_id);
            if (pSkill && pSkill->categoryId == SKILL_CATEGORY_PROFESSION && countPrimaryProfessionCount < 2)
            {
                LearnPrimaryProfession(player, skill_id, skill_level, true);
                countPrimaryProfessionCount++;
            }
            else if (pSkill && (skill_id == SKILL_FISHING || skill_id == SKILL_COOKING || skill_id == SKILL_FIRST_AID || skill_id == SKILL_ARCHAEOLOGY))
                LearnSecondaryProfession(player, skill_id, skill_level, true);
        }
        while (result->NextRow());
    }


}

void Migration::FillReputations(Player *player)
{
    uint32 wotlkGuid = player->GetWotlkGUID();
    if (QueryResult result = WotlkCharacterDatabase.PQuery("SELECT faction, standing FROM character_reputation WHERE guid = %u AND standing > 0", wotlkGuid))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 factionId = fields[0].GetUInt32();
            uint32 standing = fields[1].GetUInt32();
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionId);

            if(!factionEntry || factionEntry->reputationListID < 0)
                ChatHandler(player->GetSession()).PSendSysMessage("Reputation inexistante. Passee.");
            else
            {
                int32 repChange = standing;
                ReputationRank rank = player->GetReputationMgr().GetRank(factionEntry);
                if (rank == REP_HATED)
                    repChange += 42000;
                else if (rank == REP_HOSTILE)
                    repChange += 6000;
                else if (rank == REP_UNFRIENDLY)
                    repChange += 3000;

                repChange = player->CalculateReputationGain(REPUTATION_SOURCE_SPELL, 0, repChange, factionId);
                player->GetReputationMgr().ModifyReputation(factionEntry, repChange);
            }
        }
        while (result->NextRow());
    }


}

void Migration::ValidateSynchronisation(uint32 guid)
{
    CharacterDatabase.PExecute("UPDATE characters SET NeedSynchronisation=0 WHERE guid=%u", guid);
}

void Migration::FillSynchronisation(Player *player)
{
    if (!player->NeedSynchronisation())
        return;
    if (player->GetWotlkGUID() <= 0)
        return;

    UpdateMountAndDbleSpe(player);
    UpdateClassSkills(player);
    UpdateRaceSkills(player);
    CompleteClassQuests(player);
    player->InitTalentForLevel();
    LearnAllClassSpells(player);

    FillReputations(player);
    FillWotlkSpells(player);
    FillWotlkItems(player);
    FillWotlkAchievements(player);
    FillProfessions(player);

    player->UpdateMaxHealth();
    player->SetFullHealth();
    player->ActivateSpec(1); // force the unlearn of talent spells linked.
    player->ActivateSpec(0);
    player->SaveToDB();
    TeleportToCapital(player);
    ValidateSynchronisation(player->GetGUIDLow());
}
