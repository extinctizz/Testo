/* ScriptData
Name: diamond_commandscript
%Complete: 100
Comment: All diamond shops related commands
Category: commandscripts
EndScriptData */

#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "GuildMgr.h"
#include "Guild.h"

class diamond_commandscript : public CommandScript
{
public:
    diamond_commandscript() : CommandScript("diamond_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> guildCommandTable =
            {
                { "level",  SEC_PLAYER,  false, &HandleDiamondGuildLevelCommand,  ""},
                { "rename", SEC_PLAYER,  false, &HandleDiamondGuildRenameCommand, ""},
            };

        static std::vector<ChatCommand> diamondCommandTable =
            {
                { "info",      SEC_PLAYER,  false, &HandleDiamondInfoCommand,      ""},
                { "level",     SEC_PLAYER,  false, &HandleDiamondLevelCommand,     ""},
                { "guild",     SEC_PLAYER,  false, NULL,                           "", guildCommandTable },
                { "deserteur", SEC_PLAYER,  false, &HandleDiamondDeserteurCommand, ""},
            };

        static std::vector<ChatCommand> commandTable =
            {
                { "diamond",        SEC_PLAYER,         false, NULL,                               "", diamondCommandTable },
                { "deserteur",      SEC_MODERATOR,      true,  &HandleDeserteurCommand,            ""}
            };

      return commandTable;
  }

    static bool HandleDiamondInfoCommand(ChatHandler* handler, const char* /*args*/)
    {
        QueryResult result = StoreDatabase.PQuery( "SELECT diamond FROM diamond WHERE acctid = '%u'", handler->GetSession()->GetAccountId());
        if (!result)
        {
            StoreDatabase.PExecute("INSERT INTO diamond(acctid, diamond) VALUES('%d', '0')", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_DIAM_INFO, 0);
            return true;
        }

        Field *fields = result->Fetch();
        handler->PSendSysMessage(LANG_DIAM_INFO, fields[0].GetInt32());

        return true;
    }

    static bool HandleDiamondLevelCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player *chr = handler->GetSession()->GetPlayer();
        if (!chr)
            return false;
        uint32 chrLvl = chr->getLevel();
        int32 diam = 0;

        QueryResult result = StoreDatabase.PQuery("SELECT diamond FROM diamond WHERE acctid='%d'", handler->GetSession()->GetAccountId());
        if (!result)
        {
            StoreDatabase.PExecute("INSERT INTO diamond(acctid, diamond) VALUES('%d', '0')", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }
        Field *fields = result->Fetch();
        diam = fields[0].GetInt32();

        //Is the character has diamond ?
        if (diam == 0)
        {
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        result = StoreDatabase.Query("SELECT level FROM level ORDER BY level DESC");
        fields = result->Fetch();
        uint32 dataLvl = fields[0].GetUInt32();

        if (chrLvl >= dataLvl || chrLvl >= 85)
        {
            handler->PSendSysMessage(LANG_DIAM_LVL_ERR);
            return true;
        }

        result = StoreDatabase.PQuery("SELECT price, nblevel FROM level WHERE level = '%u'", chrLvl);
        fields = result->Fetch();
        int32 dataPrice = fields[0].GetInt32();
        uint32 dataNblvl = fields[1].GetUInt32();

        //Is character has enough diamond ?
        if (diam < dataPrice)
        {
            handler->PSendSysMessage(LANG_DIAM_NOT_ENOUGH, dataPrice);
            return true;
        }

        diam -= dataPrice;
        int32 newlevel = chrLvl + dataNblvl;

        int chrguid = chr->GetGUID();
        int acctid = handler->GetSession()->GetAccountId();

        //Character level up
        chr->GiveLevel(newlevel);
        chr->InitTalentForLevel();
        chr->SetUInt32Value(PLAYER_XP, 0);

        StoreDatabase.PExecute("UPDATE diamond SET diamond='%d' WHERE acctid='%u'", diam, handler->GetSession()->GetAccountId());
        StoreDatabase.PExecute("INSERT INTO lvl_bought(acctid, charid, clvl, elvl, nlvl, price) VALUES('%u', '%u', '%u', '%u', '%u', '%u')", acctid, chrguid, chrLvl, newlevel, dataNblvl, dataPrice);
        chr->SaveToDB();
        handler->PSendSysMessage(LANG_DIAM_LVL_BUY, dataNblvl, dataPrice);
        return true;
    }

    static bool HandleDiamondGuildLevelCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player *chr = handler->GetSession()->GetPlayer();
        if (!chr)
            return false;
        Guild *guild = chr->GetGuild();
        if (!guild)
        {
            handler->PSendSysMessage("Vous devez être en guilde pour utiliser cette commande");
            return false;
        }

        uint32 guildLvl = guild->GetLevel();
        if (guildLvl >= 25)
        {
            handler->PSendSysMessage("Votre guild à déja atteind son niveau maximal");
            return false;
        }

        int32 diam = 0;

        QueryResult result = StoreDatabase.PQuery("SELECT diamond FROM diamond WHERE acctid='%d'", handler->GetSession()->GetAccountId());
        if (!result)
        {
            StoreDatabase.PExecute("INSERT INTO diamond(acctid, diamond) VALUES('%d', '0')", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        Field *fields = result->Fetch();
        diam = fields[0].GetInt32();

        //Is the character has diamond ?
        if (diam == 0)
        {
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        result = StoreDatabase.PQuery("SELECT price, nblevel FROM level WHERE guildLevel = '%u'", guildLvl);
        fields = result->Fetch();
        int32 dataPrice = fields[0].GetInt32();
        uint32 dataNblvl = fields[1].GetUInt32();

        //Is character has enough diamond ?
        if (diam < dataPrice)
        {
            handler->PSendSysMessage(LANG_DIAM_NOT_ENOUGH, dataPrice);
            return true;
        }

        diam -= dataPrice;
        uint32 newLevel = guildLvl + dataNblvl;

        for (int i = guildLvl; i < newLevel; i++)
            guild->LevelUp(guildLvl, chr);

        int chrguid = chr->GetGUID();
        int acctid = handler->GetSession()->GetAccountId();
        uint32 guildId = guild->GetId();
        //Character level up
        StoreDatabase.PExecute("UPDATE diamond SET diamond='%d' WHERE acctid='%u'", diam, handler->GetSession()->GetAccountId());
        StoreDatabase.PExecute("INSERT INTO guild_lvl_bought(acctid, charid, guildid, clvl, elvl, nlvl, price) VALUES('%u', '%u', '%u', '%u', '%u', '%u', '%u')", acctid, chrguid, guildId, guildLvl, newLevel, dataNblvl, dataPrice);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_EXPERIENCE);
        stmt->setUInt32(0, guild->GetLevel());
        stmt->setUInt64(1, guild->GetExperience());
        stmt->setUInt64(2, guild->GetTodayExperience());
        stmt->setUInt32(3, guild->GetId());
        CharacterDatabase.DirectExecute(stmt);
        handler->PSendSysMessage("Votre commande a ete effectuee avec succes : niveau %u, cout %u dismonds", newLevel, dataPrice);
        return true;
    }


    static bool HandleDiamondGuildRenameCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        std::string newGuildNameStr = args;
        if (newGuildNameStr.size() < 3)
        {
            handler->SendSysMessage("Guild name size too low.");
            return false;
        }

        if (newGuildNameStr.size() > 20)
        {
            handler->SendSysMessage("Guild name size too big.");
            return false;
        }

        Player *chr = handler->GetSession()->GetPlayer();
        if (!chr)
            return false;

        Guild *guild = chr->GetGuild();
        if (!guild)
        {
            handler->PSendSysMessage("Vous devez etre en guilde pour utiliser cette commande");
            return false;
        }

        if (guild->GetLeaderGUID() != chr->GetGUID())
        {
            handler->PSendSysMessage("Vous devez etre gm de votre guilde pour effectuer cette action");
            return false;
        }

        int32 diam = 0;

        QueryResult result = StoreDatabase.PQuery("SELECT diamond FROM diamond WHERE acctid='%d'", handler->GetSession()->GetAccountId());
        if (!result)
        {
            StoreDatabase.PExecute("INSERT INTO diamond(acctid, diamond) VALUES('%d', '0')", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        Field *fields = result->Fetch();
        diam = fields[0].GetInt32();

        //Is the character has diamond ?
        if (diam == 0)
        {
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        uint32 dataPrice = 50;

        //Is character has enough diamond ?
        if (diam < dataPrice)
        {
            handler->PSendSysMessage(LANG_DIAM_NOT_ENOUGH, dataPrice);
            return true;
        }

        std::string oldName = guild->GetName();

        newGuildNameStr = my_escape_string(newGuildNameStr);

        if (sGuildMgr->GetGuildByName(newGuildNameStr))
        {
            handler->PSendSysMessage("Guild name %s already exist", newGuildNameStr.c_str());
            return true;
        }

        guild->UpdateName(newGuildNameStr);

        diam -= dataPrice;

        int chrGuid = chr->GetGUID();
        int acctid = handler->GetSession()->GetAccountId();
        uint32 guildId = guild->GetId();
        //Character level up
        StoreDatabase.PExecute("UPDATE diamond SET diamond='%d' WHERE acctid='%u'", diam, handler->GetSession()->GetAccountId());
        StoreDatabase.PExecute("INSERT INTO guild_rename_bought(chrGuid, guildId, name, newName) VALUES('%u', '%u', '%s', '%s')", chrGuid, guildId, oldName.c_str(), newGuildNameStr.c_str());

        handler->PSendSysMessage("Guild %s succesfully rename in %s", oldName.c_str(), newGuildNameStr.c_str());
        return true;
    }

    static bool HandleDiamondDeserteurCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player *chr = handler->GetSession()->GetPlayer();
        if (!chr)
            return false;

        int32 diam = 0;

        QueryResult result = StoreDatabase.PQuery("SELECT diamond FROM diamond WHERE acctid='%d'", handler->GetSession()->GetAccountId());
        if (!result)
        {
            StoreDatabase.PExecute("INSERT INTO diamond(acctid, diamond) VALUES('%d', '0')", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        Field *fields = result->Fetch();
        diam = fields[0].GetInt32();

        //Is the character has diamond ?
        if (diam == 0)
        {
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        uint32 dataPrice = 10;

        //Is character has enough diamond ?
        if (diam < dataPrice)
        {
            handler->PSendSysMessage(LANG_DIAM_NOT_ENOUGH, dataPrice);
            return true;
        }

        diam -= dataPrice;
        StoreDatabase.PExecute("UPDATE diamond SET diamond='%d' WHERE acctid='%u'", diam, handler->GetSession()->GetAccountId());

        chr->RemoveAura(71041);
        chr->RemoveAura(26013);
        handler->PSendSysMessage("command succesfull, deserter have been removed");
        return true;
    }

    static bool HandleDeserteurCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* target;
        uint64 target_guid;
        std::string target_name;
        if (!handler->extractPlayerTarget((char *)args, &target, &target_guid, &target_name))
            return false;

        if (!target)
            return false;

        int32 diam = 0;

        QueryResult result = StoreDatabase.PQuery("SELECT diamond FROM diamond WHERE acctid='%d'", handler->GetSession()->GetAccountId());
        if (!result)
        {
            StoreDatabase.PExecute("INSERT INTO diamond(acctid, diamond) VALUES('%d', '0')", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        Field *fields = result->Fetch();
        diam = fields[0].GetInt32();

        //Is the character has diamond ?
        if (diam == 0)
        {
            handler->PSendSysMessage(LANG_DIAM_ZERO);
            return true;
        }

        uint32 dataPrice = 10;

        //Is character has enough diamond ?
        if (diam < dataPrice)
        {
            handler->PSendSysMessage(LANG_DIAM_NOT_ENOUGH, dataPrice);
            return true;
        }

        diam -= dataPrice;
        StoreDatabase.PExecute("UPDATE diamond SET diamond='%d' WHERE acctid='%u'", diam, handler->GetSession()->GetAccountId());

        target->RemoveAura(71041);
        target->RemoveAura(26013);
        handler->PSendSysMessage("command succesfull, deserter have been removed");
        return true;
    }
};

void AddSC_diamond_commandscript()
{
    new diamond_commandscript();
}
