#ifndef GAME_SERVER_WEAPON_H
#define GAME_SERVER_WEAPON_H

#include <game/server/entities/character.h>

class CWeapon
{
private:
	CCharacter *m_pOwnerChar;
	class CGameContext *m_pGameServer;
	class CGameWorld *m_pGameWorld;
	class IServer *m_pServer;

protected:
	int m_Ammo;
	int m_MaxAmmo;
	int m_AmmoRegenStart;
	int m_AmmoRegenTime;
	bool m_FullAuto;

	int m_ReloadTimer;

	virtual void Fire(vec2 Direction) = 0;

public:
	CWeapon(CCharacter *pOwnerChar);

	void HandleFire(vec2 Direction);

	virtual void Tick();
	class CGameContext *GameServer() { return m_pGameServer; }
	class CGameWorld *GameWorld() { return m_pGameWorld; }
	class IServer *Server() { return m_pServer; }

	virtual void OnEquip(){};
	virtual void OnUnequip(){};

	// custom snap that snaps with character
	virtual void Snap(int SnappingClient, int OtherMode){};
	virtual int GetType() { return WEAPON_HAMMER; }

	virtual bool IsFullAuto() { return m_FullAuto; }

	void SetAmmo(int Ammo) { m_Ammo = Ammo; }
	int GetAmmo() { return m_Ammo; }
	int NumAmmoIcons() { return clamp(m_Ammo, 0, 10); }
	bool IsReloading() { return m_ReloadTimer != 0; };
	void Reload() { m_ReloadTimer = 0; };
	bool IsEmpty() { return m_Ammo == 0; };
};

#endif