#include "ArenaReward.h"
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

enum TitleDistribution
{
    CURRENT_SEASON = 2,

    RANK_ONE_CUTOFF          = 1,
    V2_GLADIATOR_CUTOFF      = 4,
    V2_DUEL_CUTOFF           = 11,
    V2_RIVAL_CUTOFF          = 21,
    V2_CHALLANGER_CUTOFF     = 41,


    V3_GLADIATOR_CUTOFF      = 2,
    V3_DUEL_CUTOFF           = 6,

    // misc
    MAX_RATING_DIFFERENCE    = 150,
    MIN_GAMES                = 10,

    // ranks
    RANK_ONE_TITLE           = 0,
    GLADIATOR_TITLE          = 1,
    DUEL_TITLE               = 2,
    RIVAL_TITLE              = 3,
    CHALLANGER_TITLE         = 4
};


ArenaReward::ArenaReward()
{
    Initialize();
}

ArenaReward::~ArenaReward() {}

void ArenaReward::Initialize()
{
    rewardsInfos.clear();
    arenaRewardsChar.clear();
    TC_LOG_INFO("loading", "Loading Arena Rewards...");
    uint32 oldMSTime = getMSTime();
    QueryResult result = WorldDatabase.Query("SELECT seasonID, achievement, itemEntry, rank, title FROM arena_reward");
    if (!result)
        TC_LOG_INFO("loading", ">> Loaded 0 arena reward. DB table `arena_reward` is empty.");
    else
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();
            eArenaReward tempRwd;
            tempRwd.seasonID = fields[0].GetUInt32();
            tempRwd.achievement = fields[1].GetUInt32();
            tempRwd.itemEntry = fields[2].GetUInt32();
            tempRwd.rank = fields[3].GetUInt32();
            tempRwd.title = fields[4].GetUInt32();

            std::pair<eArenaReward, uint32 > res;
            res.first = tempRwd;
            res.second = tempRwd.seasonID;
            if (tempRwd.itemEntry)
            {
                ItemTemplate const *item = sObjectMgr->GetItemTemplate(tempRwd.itemEntry);
                if (item == NULL)
                {
                    tempRwd.itemEntry = 0;
                    TC_LOG_INFO("loading", "invalid item entry %u for season %u rank %u (skiped)", tempRwd.itemEntry,  tempRwd.seasonID,  tempRwd.rank);
                }
            }
            rewardsInfos.push_back(res);
            count++;
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Loaded %u arena reward in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    TC_LOG_INFO("loading", ">> Loading rewarding list...");

    //    m_arenaRewardRefreshTimer = ARENA_REWARD_REFRESH_TIMER;
    result = CharacterDatabase.PQuery("SELECT guid, rank, currentSeason, type FROM character_arena_reward WHERE isRewarded = 0");
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 charGUID = fields[0].GetUInt32();
            uint32 rank = fields[1].GetUInt32();
            uint32 currentSeason = fields[2].GetUInt32();
            uint32 type = fields[3].GetUInt32();
            eArenaRewardChar tempCharRwd(charGUID, rank, currentSeason, type);
            std::pair<uint32, eArenaRewardChar > res;
            res.second = tempCharRwd;
            res.first = charGUID;
            arenaRewardsChar.push_back(res);
            count++;
        }
        while (result->NextRow());
        TC_LOG_INFO("loading", ">> Loaded %u valide migration in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }
}

class IsExistingRewardArena
{
public:
    IsExistingRewardArena(uint32 guid) : _guid(guid) {};
    bool operator () ( const std::pair<uint32 , eArenaRewardChar > &elem )
    {
        return elem.first == _guid;
    }
private:
    uint32 _guid;
};

void ArenaReward::CheckValidArenaReward(Player *player)
{
    {
        TRINITY_GUARD(ACE_Thread_Mutex, LockMigrationList);
        std::list<std::pair<uint32 , eArenaRewardChar > >::iterator itr = std::find_if(arenaRewardsChar.begin(), arenaRewardsChar.end(), IsExistingRewardArena(player->GetGUIDLow()));
        if (itr != arenaRewardsChar.end())
        {
            eArenaRewardChar & rwdCharInfos = itr->second;
            for (std::list<std::pair<eArenaReward, uint32 > >::iterator itr = rewardsInfos.begin(); itr != rewardsInfos.end(); itr++)
            {
                if (itr->second == rwdCharInfos.seasonID)
                    if (itr->first.rank == rwdCharInfos.rank)
                    {
                        eArenaReward &rwd = itr->first;
                        if (rwd.itemEntry)
                        {
                            if (!player->StoreNewItemInBestSlots(rwd.itemEntry, 1))
                            {
                                if (ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(rwd.itemEntry))
                                    if (Item* item = Item::CreateItem(rwd.itemEntry, 1, player))
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

                        if (rwd.title)
                            if (CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(rwd.title))
                                player->SetTitle(titleInfo);

                        if (rwd.achievement)
                            if (AchievementEntry const* achievementEntry = sAchievementMgr->GetAchievement(rwd.achievement))
                                player->CompletedAchievement(achievementEntry);
                        std::cout << "UPDATE character_arena_reward SET isRewarded = 1 WHERE guid = " << rwdCharInfos.guid << std::endl;
                        CharacterDatabase.PQuery("UPDATE character_arena_reward SET isRewarded = 1 WHERE currentSeason = %u AND guid = %u AND rank = %u AND type = %u", rwdCharInfos.seasonID, rwdCharInfos.guid, rwdCharInfos.rank,  rwdCharInfos.type);

                    }
            }
            arenaRewardsChar.erase(itr);
        }
    }
}

void ArenaReward::reward3C3Season()
{
    // Get 3v3 team top 8
    QueryResult result = CharacterDatabase.PQuery("select name, arenaTeamId, rating from arena_team where type = 3 and seasonGames > 9 order by rating desc limit %u", V3_DUEL_CUTOFF);
    if (result)
    {
        uint32 count = 0;
        uint32 rank = 0;
        do
        {
            ++rank;
            Field* fields = result->Fetch();

            std::string teamname = fields[0].GetString();
            CharacterDatabase.EscapeString(teamname);
            uint32 teamid = fields[1].GetUInt32();
            uint32 teamrating = fields[2].GetUInt32();

            uint32 titleRank = 0;
            if (rank <= RANK_ONE_CUTOFF)
            {
                titleRank = RANK_ONE_TITLE;
            }
            else if (rank <= V3_GLADIATOR_CUTOFF)
            {
                titleRank = GLADIATOR_TITLE;
            }
            else if (rank <= V3_DUEL_CUTOFF)
            {
                titleRank = DUEL_TITLE;
            }

            // Get team members for each 3v3 team
            QueryResult res2 = CharacterDatabase.PQuery("select arena_team_member.guid, arena_team_member.personalRating from arena_team_member join character_pvp_stats on arena_team_member.guid = character_pvp_stats.guid where arena_team_member.arenateamid = '%u' and character_pvp_stats.slot = 0", teamid);
            if (res2)
            {
                do
                {
                    Field* fld2 = res2->Fetch();
                    uint32 playerid = fld2[0].GetUInt32();
                    uint32 playerrating = fld2[1].GetUInt32();

                    if (playerrating + MAX_RATING_DIFFERENCE < teamrating)
                        continue;
                    else
                    {
                        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ARENA_SEASON_CHAR_REWARD);
                        stmt->setUInt32(0, playerid);
                        stmt->setUInt32(1, titleRank);
                        stmt->setUInt32(2, CURRENT_SEASON);
                        stmt->setUInt32(3, 3);
                        CharacterDatabase.Execute(stmt);
                        ++count;
                    }
                }
                while (res2->NextRow());
            }
        }
        while (result->NextRow());
    }
}

void ArenaReward::reward2C2Season()
{
    QueryResult result = CharacterDatabase.PQuery("select name, arenaTeamId, rating from arena_team where type = 2 and seasonGames > 9 order by rating desc limit %u", V2_CHALLANGER_CUTOFF);
    if (result)
    {
        uint32 count = 0;
        uint32 rank = 0;
        do
        {
            ++rank;
            Field* fields = result->Fetch();

            std::string teamname = fields[0].GetString();
            CharacterDatabase.EscapeString(teamname);
            uint32 teamid = fields[1].GetUInt32();
            uint32 teamrating = fields[2].GetUInt32();

            uint32 titleRank = 0;
            if (rank <= RANK_ONE_CUTOFF)
            {
                titleRank = RANK_ONE_TITLE;
            }
            else if (rank <= V2_GLADIATOR_CUTOFF)
            {
                titleRank = GLADIATOR_TITLE;
            }
            else if (rank <= V2_DUEL_CUTOFF)
            {
                titleRank = DUEL_TITLE;
            }
            else if (rank <= V2_RIVAL_CUTOFF)
            {
                titleRank = RIVAL_TITLE;
            }
            else if (rank <= V2_CHALLANGER_CUTOFF)
            {
                titleRank = CHALLANGER_TITLE;
            }

            // Get team members for each 2v2 team
            QueryResult res2 = CharacterDatabase.PQuery("select arena_team_member.guid, arena_team_member.personalRating from arena_team_member join character_pvp_stats on arena_team_member.guid = character_pvp_stats.guid where arena_team_member.arenateamid = '%u' and character_pvp_stats.slot = 0", teamid);
            if (res2)
            {
                do
                {
                    Field* fld2 = res2->Fetch();
                    uint32 playerid = fld2[0].GetUInt32();
                    uint32 playerrating = fld2[1].GetUInt32();

                    if (playerrating + MAX_RATING_DIFFERENCE < teamrating)
                        continue;
                    else
                    {
                        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ARENA_SEASON_CHAR_REWARD);
                        stmt->setUInt32(0, playerid);
                        stmt->setUInt32(1, titleRank);
                        stmt->setUInt32(2, CURRENT_SEASON);
                        stmt->setUInt32(3, 2);
                        CharacterDatabase.Execute(stmt);
                        ++count;
                    }
                }
                while (res2->NextRow());
            }
        }
        while (result->NextRow());
    }
}

void ArenaReward::resetSeason()
{
    CharacterDatabase.PQuery("UPDATE character_pvp_stats SET matchMakerRating = 0");
    CharacterDatabase.PQuery("UPDATE character_battleground_stats SET rating = 0");
    CharacterDatabase.PQuery("UPDATE arena_team SET rating = 0, seasonGames = 0, seasonWins = 0, rank = 0");
    CharacterDatabase.PQuery("UPDATE arena_team_member SET personalRating = 0, seasonGames = 0, seasonWins = 0");
    CharacterDatabase.PQuery("UPDATE character_currency SET season_cap = 0");
}
