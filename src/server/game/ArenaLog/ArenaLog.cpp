#include "ArenaLog.h"
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

ArenaLog::ArenaLog()
{
    m_arenaLogTimer = 60000;
    //    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "Loading Recup packs...");
    //  uint32 oldMSTime = getMSTime();
    //  sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u valide migration in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

ArenaLog::~ArenaLog() {}

void ArenaLog::saveMatch(arenaInfos *saveInfos)
{
    {
        TRINITY_GUARD(ACE_Thread_Mutex, LockArenaLogList);
        arenaInfosList.push_back(saveInfos);
    }
}

void ArenaLog::Update(const uint32 &diff)
{
    if (m_arenaLogTimer <= diff)
    {
        TRINITY_GUARD(ACE_Thread_Mutex, LockArenaLogList);
        // LockArenaLogList
        int count = 0;
        std::string winplayername[5];
        std::string winplayerguildname[5];
        uint32 windamages[5];
        uint32 winheals[5];
        uint32 winkillingBlows[5];
        uint32 winnersGuids[5];

        std::string losplayername[5];
        std::string losplayerguildname[5];
        uint32 losdamages[5];
        uint32 losheals[5];
        uint32 loskillingBlows[5];
        uint32 looserGuids[5];

        for (int i = 0; i < 5; i++)
        {
            winplayername[i] = "";
            winplayerguildname[i] = "";
            windamages[i] = 0;
            winheals[i] = 0;
            winkillingBlows[i] = 0;
            winnersGuids[i] = 0;

            losplayername[i] = "";
            losplayerguildname[i] = "";
            losdamages[i] = 0;
            losheals[i] = 0;
            loskillingBlows[i] = 0;
            looserGuids[i] = 0;
        }

        for (std::list<arenaInfos *>::iterator iter = arenaInfosList.begin(); iter != arenaInfosList.end(); iter++)
        {
            arenaInfos *data = *iter;

            count = 0;
            for (std::list<uint32 >::iterator itr = data->looserGuildId.begin(); itr != data->looserGuildId.end(); itr++)
                losplayerguildname[count++] = sGuildMgr->GetGuildNameById(*itr);
            count = 0;
            for (std::list<uint64 >::iterator itr = data->looserDamages.begin(); itr != data->looserDamages.end(); itr++)
                losdamages[count++] = *itr;
            count = 0;
            for (std::list<uint64 >::iterator itr = data->looserHeals.begin(); itr != data->looserHeals.end(); itr++)
                losheals[count++] = *itr;
            count = 0;
            for (std::list<uint64 >::iterator itr = data->looserKillingBlows.begin(); itr != data->looserKillingBlows.end(); itr++)
                loskillingBlows[count++] = *itr;
            count = 0;
            for (std::list<uint64 >::iterator itr = data->looserGUID.begin(); itr != data->looserGUID.end(); itr++)
                if (Player* player = ObjectAccessor::FindPlayer(*itr))
                {
                    losplayername[count] = player->GetName();
                    looserGuids[count++] = player->GetGUIDLow();
                }

            count = 0;
            for (std::list<uint32 >::iterator itr = data->winnerGuildId.begin(); itr != data->winnerGuildId.end(); itr++)
                winplayerguildname[count++] = sGuildMgr->GetGuildNameById(*itr);
            count = 0;
            for (std::list<uint64 >::iterator itr = data->winnerDamages.begin(); itr != data->winnerDamages.end(); itr++)
                windamages[count++] = *itr;
            count = 0;
            for (std::list<uint64 >::iterator itr = data->winnerHeals.begin(); itr != data->winnerHeals.end(); itr++)
                   winheals[count++] = *itr;
            count = 0;
            for (std::list<uint64 >::iterator itr = data->winnerKillingBlows.begin(); itr != data->winnerKillingBlows.end(); itr++)
                winkillingBlows[count++] = *itr;
            count = 0;
            for (std::list<uint64 >::iterator itr = data->winnerGUID.begin(); itr != data->winnerGUID.end(); itr++)
                if (Player* player = ObjectAccessor::FindPlayer(*itr))
                {
                    winnersGuids[count] = player->GetGUIDLow();
                    winplayername[count++] = player->GetName();
                }
            std::string timee = data->timeEnd;
            CharacterDatabase.PExecute("insert into arena_log Values('%s', '%s', '%s', %u, %u, %u, %u, '%s', '%s', %u, %u, %u, %u,'%s', '%s', %u, %u, %u, %u,'%s', '%s', %u, %u, %u, %u, '%s', '%s', %u, %u, %u, %u, '%s', '%s', %u, %u, %u, %u, '%s', '%s', %u, %u, %u, %u,'%s', '%s', %u, %u, %u, %u,'%s', '%s', %u, %u, %u, %u, '%s', '%s', %u, %u, %u, %u)",
                                       timee.c_str(),
                                       winplayername[0].c_str(), winplayerguildname[0].c_str(), windamages[0], winheals[0], winkillingBlows[0], winnersGuids[0],
                                       winplayername[1].c_str(), winplayerguildname[1].c_str(), windamages[1], winheals[1], winkillingBlows[1], winnersGuids[1],
                                       winplayername[2].c_str(), winplayerguildname[2].c_str(), windamages[2], winheals[2], winkillingBlows[2], winnersGuids[2],
                                       winplayername[3].c_str(), winplayerguildname[3].c_str(), windamages[3], winheals[3], winkillingBlows[3], winnersGuids[3],
                                       winplayername[4].c_str(), winplayerguildname[4].c_str(), windamages[4], winheals[4], winkillingBlows[4], winnersGuids[4],

                                       losplayername[0].c_str(), losplayerguildname[0].c_str(), losdamages[0], losheals[0], loskillingBlows[0], looserGuids[0],
                                       losplayername[1].c_str(), losplayerguildname[1].c_str(), losdamages[1], losheals[1], loskillingBlows[1], looserGuids[1],
                                       losplayername[2].c_str(), losplayerguildname[2].c_str(), losdamages[2], losheals[2], loskillingBlows[2], looserGuids[2],
                                       losplayername[3].c_str(), losplayerguildname[3].c_str(), losdamages[3], losheals[3], loskillingBlows[3], looserGuids[3],
                                       losplayername[4].c_str(), losplayerguildname[4].c_str(), losdamages[4], losheals[4], loskillingBlows[4], looserGuids[4]
                                       );


            arenaInfosList.erase(iter);
            delete data;
            break;
        }
        m_arenaLogTimer = ARENALOG_REFRESH_TIMER;
    }
    else
        m_arenaLogTimer -= diff;
}
