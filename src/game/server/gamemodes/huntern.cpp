/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "huntern.h"

#include <game/server/entities/character.h>
#include <game/server/weapons.h>

#include <game/server/classes.h>

CGCHunterN::CGCHunterN() :
	IGameController()
{
	m_pGameType = "HunterN";
	m_GameFlags = IGF_SUDDENDEATH;

	// Register Config
	INSTANCE_CONFIG_INT(&m_HuntHunterFixed, "hunt_hunter_fixed", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否选取固定猎人数量（默认0,最小0,最大1）");
	INSTANCE_CONFIG_INT(&m_HuntHunterNumber, "hunt_hunter_number", 1, 1, 0xFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "固定猎人数量为（默认1,最小1,最大inf）");
	INSTANCE_CONFIG_INT(&m_HuntHunterRatio, "hunt_hunter_ratio", 5, 2, 0xFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "几个玩家里选取一个猎人（默认5,最小2,最大inf）");
	INSTANCE_CONFIG_INT(&m_ShowHunterList, "hunt_show_hunterlist", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否开局向全体广播猎人列表（默认0,最小0,最大1）");
}

void CGCHunterN::OnCharacterSpawn(CCharacter *pChr)
{
	m_PlayersInRoom.add(pChr->GetPlayer());
	pChr->IncreaseHealth(10);

	pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
	pChr->GiveWeapon(WEAPON_HAMMER, WEAPON_ID_HAMMER, -1);
}

int CGCHunterN::OnCharacterDeath(CCharacter *pVictim, CPlayer *pKiller, int Weapon)
{
	m_PlayersInRoom.remove(pVictim->GetPlayer());
}

void CGCHunterN::ChooseClass()
{
	char aBuf[512];
	bool SoloPlayerBefore = m_Civics == 1;

	// elect hunter
	int LeastPlayers = m_HuntHunterFixed ? (m_HuntHunterNumber + 1) : 2;
	if(m_Civics < LeastPlayers)
	{
		if(Server()->Tick() % (Server()->TickSpeed() * 2) == 0)
		{
			str_format(aBuf, sizeof(aBuf), "这里是猎人杀PVP\n每回合秘密抽选猎人\n少数猎人对战多数平民\n至少需要 %d 玩家才能开始", LeastPlayers);
			GameServer()->SendBroadcast(aBuf, -1);
			if(m_Timelimit > 0)
				m_GameStartTick = Server()->Tick();
		}
	}
	else if(SoloPlayerBefore)
	{
		// clear broadcast
		GameServer()->SendBroadcast("", -1);
		StartRound();
	}
	else
	{
		Server()->DemoRecorder_HandleAutoStart();

		m_Hunters = m_HuntHunterFixed ? m_HuntHunterNumber : ((m_Civics + m_HuntHunterRatio - 1) / m_HuntHunterRatio);
		char m_aHuntersMessage[32];

		str_copy(aBuf, "Hunter是: ", sizeof(aBuf));
		str_copy(m_aHuntersMessage, "这一回合的Hunter是: ", sizeof(m_aHuntersMessage));

		for(int iHunter = 0; iHunter < m_Hunters; iHunter++)
		{
			short NewHunter;
			while(!NewHunter)
			{
				short RandChoose = rand() % m_PlayersInRoom.size();
				if(m_PlayersInRoom[RandChoose]->GetClass() == PLAYERCLASS_HUNTER)
					continue;

				NewHunter = m_PlayersInRoom[RandChoose]->GetCID();
			}

			// generate info message
			const char *ClientName = Server()->ClientName(GameServer()->m_apPlayers[NewHunter]->GetCID());
			str_append(m_aHuntersMessage, ClientName, sizeof(m_aHuntersMessage));
			if(m_Hunters - iHunter > 1)
				str_append(m_aHuntersMessage, ", ", sizeof(m_aHuntersMessage));
			int aBufLen = str_length(aBuf);
			str_format(aBuf + aBufLen, sizeof(aBuf) - aBufLen, m_Hunters - iHunter == 1 ? "%d:%s" : "%d:%s, ", GameServer()->m_apPlayers[NewHunter]->GetCID(), ClientName);
		}
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);

		// notify all

		GameServer()->SendChatTarget(-1, "——————欢迎来到HunterN猎人杀——————");
		str_format(aBuf, sizeof(aBuf), "本回合有 %d 个猎人Hunter has been selected.", m_Hunters);
		GameServer()->SendChatTarget(-1, aBuf);
		GameServer()->SendChatTarget(-1, "规则：每回合秘密抽选猎人 猎人对战平民 活人看不到死人消息");
		GameServer()->SendChatTarget(-1, "      猎人双倍伤害 有瞬杀锤子(平民无锤)和破片榴弹(对自己无伤)");
		GameServer()->SendChatTarget(-1, "分辨队友并消灭敌人取得胜利！Be warned! Sudden Death.");

		if(m_ShowHunterList)
		{
			GameServer()->SendChatTarget(-1, m_aHuntersMessage);
		}
		else
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
				if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
					GameServer()->SendChatTarget(GameServer()->m_apPlayers[i]->GetCID(), m_aHuntersMessage);
		}
	}
}

void CGCHunterN::OnWorldReset()
{
	m_Hunters = 0;
	m_HunterDeathes = 0;
	m_Civics = 0;
	m_CivicDeathes = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerIfInRoom(i))
		{
			GameServer()->m_apPlayers[i]->SetClass(PLAYERCLASS_CIVICS);
			if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
			{
				GameServer()->m_apPlayers[i]->SetClass(PLAYERCLASS_CIVICS);
				GameServer()->m_apPlayers[i]->Respawn();
				// GameServer()->m_apPlayers[i]->m_Score = 0;
				// GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
				GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick() + Server()->TickSpeed() / 2;
			}
			if(GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				m_Civics++;
		}
	}
}

void CGCHunterN::OnPlayerLeave(CPlayer *pPlayer)
{
	m_PlayersInRoom.remove(pPlayer);
}