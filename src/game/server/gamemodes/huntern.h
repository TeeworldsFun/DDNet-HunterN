/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERN_H
#define GAME_SERVER_GAMEMODES_HUNTERN_H
#include <game/server/gamecontroller.h>
#include <base/tl/array.h>

class CGCHunterN : public IGameController
{
private:
	int m_HuntHunterFixed;
	int m_HuntHunterNumber;
	int m_HuntHunterRatio;
	int m_ShowHunterList;
public:
	CGCHunterN();

	// event
	virtual void OnCharacterSpawn(class CCharacter *pChr) override;
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
	virtual void OnWorldReset() override;
	virtual void OnPlayerLeave(CPlayer *pPlayer) override;
	
	void ChooseClass();

	array<CPlayer *> m_PlayersInRoom;

	int m_Hunters;
	int m_Civics;

	int m_HunterDeathes;
	int m_CivicDeathes;
};

#endif // GAME_SERVER_GAMEMODES_HUNTERN_H
