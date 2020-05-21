#include "ScriptPCH.h"
#include "Player.h"
#include "ReputationMgr.h"
#include "AchievementMgr.h"

#define SPELL_PALADIN_CRUSADER_STRIKE 35395

class paladin_learn_missing_spell_playerscript : public PlayerScript
{
public:
    paladin_learn_missing_spell_playerscript() : PlayerScript("paladin_learn_missing_spell_playerscript") { }

    void OnLogin(Player* player)
    {

        /*        for (int i = 0; i < ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE; i++)
        {
            AchievementCriteriaEntryList const& achievementCriteriaList = sAchievementMgr->GetAchievementCriteriaByType((AchievementCriteriaTypes)i);
            for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
            {
                AchievementCriteriaEntry const* achievementCriteria = (*i);
                WorldDatabase.DirectPExecute("REPLACE INTO achievement_criteria_dbc values (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u)",
                                             achievementCriteria->ID,
                                             achievementCriteria->achievement,
                                             achievementCriteria->type,
                                             achievementCriteria->kill_creature.creatureID,
                                             achievementCriteria->kill_creature.creatureCount,
                                             achievementCriteria->additionalRequirements[0].additionalRequirement_type,
                                             achievementCriteria->additionalRequirements[0].additionalRequirement_value,
                                             achievementCriteria->additionalRequirements[1].additionalRequirement_type,
                                             achievementCriteria->additionalRequirements[1].additionalRequirement_value,
                                             achievementCriteria->completionFlag,
                                             achievementCriteria->timedCriteriaStartType,
                                             achievementCriteria->timedCriteriaMiscId,
                                             achievementCriteria->timeLimit,
                                             achievementCriteria->showOrder,
                                             achievementCriteria->additionalConditionType[0],
                                             achievementCriteria->additionalConditionType[1],
                                             achievementCriteria->additionalConditionType[2],
                                             achievementCriteria->additionalConditionValue[0],
                                             achievementCriteria->additionalConditionValue[1],
                                             achievementCriteria->additionalConditionValue[2]);
            }
            }*/
        if (player->getRace() == RACE_WORGEN && !player->HasSpell(68996) && player->getLevel() >= 13)
        {
            player->CastSpell(player, 95759, true);
            player->CastSpell(player, 72792, true);
            player->learnSpell(68996, false);
        }
        else if (player->getRace() != RACE_WORGEN && player->HasSpell(68996))
        {
            player->removeSpell(68992);
            player->removeSpell(68975);
            player->removeSpell(68978);
            player->removeSpell(68976);
            player->removeSpell(68996);
        }
        if (player->getClass() != CLASS_PALADIN || !sSpellMgr->GetSpellInfo(SPELL_PALADIN_CRUSADER_STRIKE) || player->HasSpell(SPELL_PALADIN_CRUSADER_STRIKE))
            return;
        player->learnSpell(SPELL_PALADIN_CRUSADER_STRIKE, false);
    }
};

void AddSC_paladin_learn_missing_spell_playerscript()
{
    new paladin_learn_missing_spell_playerscript();
}
