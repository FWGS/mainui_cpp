#include "BaseMenu.h"
#include "ClientWindow.h"
#include "ScrollView.h"
#include "CheckBox.h"

struct BuyMenuWeaponInfo
{
	const char *name, *image, *command, *name2;
};

class CClientBaseBuyMenu : public CClientWindow
{
public:
	virtual void Init()
	{
		if( !WasInit() )
		{
			CClientWindow::Init();

			if( eAmmoClass == BUY_EQUIPMENT )
			{
				AddButton( '0', L("Cancel"), Point( 100, 580 ), VoidCb( &CMenuBaseItem::Hide ));
			}
			else
			{
				AddButton( '0', L("Cancel"), Point( 100, 530 ), VoidCb( &CMenuBaseItem::Hide ));
			}

			if( eAmmoClass == BUY_PRIMAMMO )
			{
				bitmap.SetSize( 400, 100 );
			}
			else
			{
				bitmap.SetSize( 400, 200 );
			}

			bitmap.SetCoord( 400, 180 );
			bitmap.bDrawStroke = true;
			bitmap.SetInactive( true );
			bitmap.eRenderMode = QM_DRAWTRANS;
			bitmap.colorStroke  = uiInputTextColor;
			bitmap.iStrokeWidth = 1;
			AddItem( bitmap );

			int startH;
			if( eAmmoClass == BUY_PRIMAMMO )
			{
				startH = 300;
			}
			else
			{
				startH = 400;
			}

			Point pt = Point( 400, startH );

			confirm.SetRect( 700, startH, 100, 32 );
			confirm.onActivated = MenuCb( &CClientBaseBuyMenu::confirmCb );
			confirm.SetCharSize( QM_SMALLFONT );
			confirm.bDrawStroke = true;
			confirm.colorStroke = uiInputTextColor;
			confirm.iStrokeWidth = 1;
			confirm.szName = L( "Buy" );
			confirm.SetBackground( 0U, PackRGBA( 255, 0, 0, 64 ) );
			AddItem( confirm );

			priceLabel.SetInactive( true );
			priceLabel.pos = pt;
			priceLabel.m_bLimitBySize = false;
			priceLabel.szName = L("CStrike_PriceLabel");
			priceLabel.SetCharSize( QM_SMALLFONT );
			AddItem( priceLabel );
			const int height = g_FontMgr.GetFontTall( priceLabel.font );
			pt.y += height;

			originLabel.SetInactive( true );
			originLabel.pos = pt;
			originLabel.m_bLimitBySize = false;
			if( eAmmoClass == BUY_EQUIPMENT )
				originLabel.szName = L("CStrike_DescriptionLabel");
			else originLabel.szName = L("CStrike_OriginLabel");
			originLabel.SetCharSize( QM_SMALLFONT );
			AddItem( originLabel );
			pt.y += height;

			if( eAmmoClass != BUY_EQUIPMENT )
			{
				calibreLabel.SetInactive( true );
				calibreLabel.pos = pt;
				calibreLabel.m_bLimitBySize = false;
				calibreLabel.szName = L("CStrike_CalibreLabel");
				calibreLabel.SetCharSize( QM_SMALLFONT );
				AddItem( calibreLabel );
				pt.y += height;

				clipLabel.SetInactive( true );
				clipLabel.pos = pt;
				clipLabel.m_bLimitBySize = false;
				clipLabel.szName = L("CStrike_ClipCapacityLabel");
				clipLabel.SetCharSize( QM_SMALLFONT );
				AddItem( clipLabel );
				pt.y += height;

				rofLabel.SetInactive( true );
				rofLabel.pos = pt;
				rofLabel.m_bLimitBySize = false;
				rofLabel.szName = L("CStrike_RateOfFireLabel");
				rofLabel.SetCharSize( QM_SMALLFONT );
				AddItem( rofLabel );
				pt.y += height;

				weightLabel.SetInactive( true );
				weightLabel.pos = pt;
				weightLabel.m_bLimitBySize = false;
				weightLabel.szName = L("CStrike_WeightEmptyLabel");
				weightLabel.SetCharSize( QM_SMALLFONT );
				AddItem( weightLabel );
				pt.y += height;

				projectLabel.SetInactive( true );
				projectLabel.pos = pt;
				projectLabel.m_bLimitBySize = false;
				projectLabel.szName = L("CStrike_ProjectileWeightLabel");
				projectLabel.SetCharSize( QM_SMALLFONT );
				AddItem( projectLabel );
				pt.y += height;

				muzzlevelLabel.SetInactive( true );
				muzzlevelLabel.pos = pt;
				muzzlevelLabel.m_bLimitBySize = false;
				muzzlevelLabel.szName = L("CStrike_MuzzleVelocityLabel");
				muzzlevelLabel.SetCharSize( QM_SMALLFONT );
				AddItem( muzzlevelLabel );
				pt.y += height;

				muzzleenLabel.SetInactive( true );
				muzzleenLabel.pos = pt;
				muzzleenLabel.m_bLimitBySize = false;
				muzzleenLabel.szName = L("CStrike_MuzzleEnergyLabel");
				muzzleenLabel.SetCharSize( QM_SMALLFONT );
				AddItem( muzzleenLabel );
				pt.y += height;
			}

			pt = Point( 600, startH );

			priceLabel_.SetInactive( true );
			priceLabel_.pos = pt;
			priceLabel_.m_bLimitBySize = false;
			priceLabel_.SetCharSize( QM_SMALLFONT );
			AddItem( priceLabel_ );
			pt.y += height;

			originLabel_.SetInactive( true );
			originLabel_.pos = pt;
			originLabel_.m_bLimitBySize = eAmmoClass == BUY_EQUIPMENT;
			originLabel_.SetCharSize( QM_SMALLFONT );
			originLabel_.SetSize( 200, 999 );
			AddItem( originLabel_ );
			pt.y += height;

			if( eAmmoClass != BUY_EQUIPMENT )
			{
				calibreLabel_.SetInactive( true );
				calibreLabel_.pos = pt;
				calibreLabel_.m_bLimitBySize = false;
				calibreLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( calibreLabel_ );
				pt.y += height;

				clipLabel_.SetInactive( true );
				clipLabel_.pos = pt;
				clipLabel_.m_bLimitBySize = false;
				clipLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( clipLabel_ );
				pt.y += height;

				rofLabel_.SetInactive( true );
				rofLabel_.pos = pt;
				rofLabel_.m_bLimitBySize = false;
				rofLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( rofLabel_ );
				pt.y += height;

				weightLabel_.SetInactive( true );
				weightLabel_.pos = pt;
				weightLabel_.m_bLimitBySize = false;
				weightLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( weightLabel_ );
				pt.y += height;

				projectLabel_.SetInactive( true );
				projectLabel_.pos = pt;
				projectLabel_.m_bLimitBySize = false;
				projectLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( projectLabel_ );
				pt.y += height;

				muzzlevelLabel_.SetInactive( true );
				muzzlevelLabel_.pos = pt;
				muzzlevelLabel_.m_bLimitBySize = false;
				muzzlevelLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( muzzlevelLabel_ );
				pt.y += height;

				muzzleenLabel_.SetInactive( true );
				muzzleenLabel_.pos = pt;
				muzzleenLabel_.m_bLimitBySize = false;
				muzzleenLabel_.SetCharSize( QM_SMALLFONT );
				AddItem( muzzleenLabel_ );
				pt.y += height;
			}
		}
	}

	virtual void Reload()
	{
		bitmap.SetPicture( NULL );
		bitmap.colorBase = 0x0;
		confirm.Hide();
		priceLabel_.szName =
			originLabel_.szName =
			calibreLabel_.szName =
			clipLabel_.szName =
			rofLabel_.szName =
			weightLabel_.szName =
			projectLabel_.szName =
			muzzlevelLabel_.szName =
			muzzleenLabel_.szName = "";
		weightLabel.szName = L( "CStrike_WeightEmptyLabel" );
	}

	void AddWeapon( BuyMenuWeaponInfo info )
	{
		weapons[numWeapons] = info;

		int key = '0' + numWeapons;
		Point pt;
		pt.x = 100;
		pt.y = 180 + numWeapons * 50;

		CEventCallback cb_;

		cb_ = MenuCb( &CClientBaseBuyMenu::cb );
		cb_.pExtra = &weapons[numWeapons];

		snprintf( buf, sizeof( buf ), "Cstrike_%s", info.name );

		AddButton( key, L( buf ), pt, cb_ );

		numWeapons++;
	}

	CMenuBitmap bitmap;
	CMenuAction confirm;
	CMenuAction priceLabel, priceLabel_;
	CMenuAction originLabel, originLabel_;
	CMenuAction calibreLabel, calibreLabel_;
	CMenuAction clipLabel, clipLabel_;
	CMenuAction rofLabel, rofLabel_;
	CMenuAction weightLabel, weightLabel_;
	CMenuAction projectLabel, projectLabel_;
	CMenuAction muzzlevelLabel, muzzlevelLabel_;
	CMenuAction muzzleenLabel, muzzleenLabel_;

	BuyMenuWeaponInfo weapons[10];
	int numWeapons = 0;

	enum ammoClass_e
	{
		BUY_NONE,
		BUY_PRIMAMMO,
		BUY_SECAMMO,
		BUY_EQUIPMENT
	} eAmmoClass;

	char buf[1024];
	char buf2[1024];

	void cb( void *pExtra )
	{
		BuyMenuWeaponInfo *weapon = (BuyMenuWeaponInfo*)pExtra;

		confirm.Show();

		snprintf( buf, sizeof( buf ), "CStrike_%sPrice", weapon->name2 );
		priceLabel_.szName = L( buf );

		if( eAmmoClass == BUY_EQUIPMENT )
		{
			snprintf( buf, sizeof( buf ), "CStrike_%sDescription", weapon->name2 );
			int i = 0;
			for( const char *t = L( buf ); *t; t++ )
			{
				if( *t == '\\' && *(t+1) == 'n' )
				{
					t+=2;
					continue;
				}
				buf2[i++] = *t;
			}
			originLabel_.szName = buf2;
		}
		else
		{
			snprintf( buf, sizeof( buf ), "CStrike_%sOrigin", weapon->name2 );
			originLabel_.szName = L( buf );

			snprintf( buf, sizeof( buf ), "CStrike_%sCalibre", weapon->name2 );
			calibreLabel_.szName = L( buf );

			snprintf( buf, sizeof( buf ), "CStrike_%sClipCapacity", weapon->name2 );
			clipLabel_.szName = L( buf );

			snprintf( buf, sizeof( buf ), "CStrike_%sRateOfFire", weapon->name2 );
			rofLabel_.szName = L( buf );

			snprintf( buf, sizeof( buf ), "CStrike_%sWeightEmpty", weapon->name2 );
			const char *tr = L( buf );

			if( tr == buf ) // no translation
			{
				weightLabel.szName = L( "CStrike_WeightLoadedLabel" );
				snprintf( buf, sizeof( buf ), "CStrike_%sWeightLoaded", weapon->name2 );
				tr = L( buf );
			}
			else
			{
				weightLabel.szName = L( "CStrike_WeightEmptyLabel" );
			}

			weightLabel_.szName = L( tr );

			snprintf( buf, sizeof( buf ), "CStrike_%sProjectileWeight", weapon->name2 );
			projectLabel_.szName = L( buf );

			snprintf( buf, sizeof( buf ), "CStrike_%sMuzzleVelocity", weapon->name2 );
			muzzlevelLabel_.szName = L( buf );

			snprintf( buf, sizeof( buf ), "CStrike_%sMuzzleEnergy", weapon->name2 );
			muzzleenLabel_.szName = L( buf );
		}

		confirm.onActivated.pExtra = (void*)weapon->command;

		snprintf( buf, sizeof( buf ), "gfx/vgui/%s.tga", weapon->image );
		bitmap.SetPicture( buf );
		bitmap.colorBase = uiColorWhite;
	}

	void confirmCb( void *pExtra )
	{
		const  char *command = (const char*)pExtra;
		EngFuncs::ClientCmd( TRUE, command );

		if( EngFuncs::GetCvarFloat( "ui_cs_autofill" ))
		{
			switch( eAmmoClass )
			{
			case BUY_NONE:
				break;
			case BUY_PRIMAMMO: EngFuncs::ClientCmd( FALSE, "primammo" ); break;
			case BUY_SECAMMO:  EngFuncs::ClientCmd( FALSE, "secammo" ); break;
			case BUY_EQUIPMENT: break;
			}
		}
		Hide();
	}
};
class CClientMainBuyMenu : public CClientWindow {
public:
	virtual void _Init() override;

private:
	CMenuCheckBox autoFill;
};
class CClientPistolsTMenu         : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientPistolsCTMenu        : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientShotgunsMenu         : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientSubMachineGunsTMenu  : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientSubMachineGunsCTMenu : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientRiflesTMenu          : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientRiflesCTMenu         : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientMachineGunsMenu      : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientItemTMenu            : public CClientBaseBuyMenu { virtual void _Init() override; };
class CClientItemCTMenu           : public CClientBaseBuyMenu { virtual void _Init() override; };

template <typename T>
void Menu_Show( void )
{
	static T menu;
	menu.Show();
}

template <typename T, typename CT>
void Menu_Show_Team( int num ) { num == TEAM_TERRORIST ? Menu_Show<T>() : Menu_Show<CT>(); }

template <typename T, typename CT>
void Menu_Show_Team( CMenuBaseItem *item, void *pExtra ) {
	item->Parent()->Hide();
	Menu_Show_Team<T, CT>( g_pClient->GetLocalPlayerTeam() );
}

void UI_BuyMenu_Show( int param1, int param2 ) { Menu_Show<CClientMainBuyMenu>(); }
void UI_BuyMenu_Pistol_Show(int param1, int param2 ) { Menu_Show_Team<CClientPistolsTMenu, CClientPistolsCTMenu>( param2 ); }
void UI_BuyMenu_Shotgun_Show(int param1, int param2 ) {	Menu_Show<CClientShotgunsMenu>( ); }
void UI_BuyMenu_Submachine_Show(int param1, int param2 ) { Menu_Show_Team<CClientSubMachineGunsTMenu, CClientSubMachineGunsCTMenu>( param2 ); }
void UI_BuyMenu_Rifle_Show(int param1, int param2 ) { Menu_Show_Team<CClientRiflesTMenu, CClientRiflesCTMenu>( param2 ); }
void UI_BuyMenu_Machinegun_Show(int param1, int param2 ) { Menu_Show_Team<CClientMachineGunsMenu, CClientMachineGunsMenu>( param2 ); }
void UI_BuyMenu_Item_Show(int param1, int param2 ) { Menu_Show_Team<CClientItemTMenu, CClientItemCTMenu>( param2 ); }

void CClientMainBuyMenu::_Init()
{
	szName = L("Cstrike_Select_Category");

	Point pt = Point( 100, 180 );
	AddButton( '1', L( "Cstrike_Pistols" ), pt, Menu_Show_Team<CClientPistolsTMenu, CClientPistolsCTMenu> );
	pt.y += 50;
	AddButton( '2', L( "Cstrike_Shotguns" ), pt, Menu_Show_Team<CClientShotgunsMenu, CClientShotgunsMenu> );
	pt.y += 50;
	AddButton( '3', L( "Cstrike_SubMachineGuns" ), pt, Menu_Show_Team<CClientSubMachineGunsTMenu, CClientSubMachineGunsCTMenu> );
	pt.y += 50;
	AddButton( '4', L( "Cstrike_Rifles" ), pt, Menu_Show_Team<CClientRiflesTMenu, CClientRiflesCTMenu> );
	pt.y += 50;
	AddButton( '5', L( "Cstrike_MachineGuns" ), pt, Menu_Show_Team<CClientMachineGunsMenu, CClientMachineGunsMenu> );
	pt.y += 50;
	AddButton( '6', L( "Cstrike_Prim_Ammo" ), pt, ExecAndHide( "primammo" ) );
	pt.y += 50;
	AddButton( '7', L( "Cstrike_Sec_Ammo" ), pt, ExecAndHide( "secammo" ));
	pt.y += 50;
	AddButton( '8', L( "Cstrike_Equipment" ), pt, Menu_Show_Team<CClientItemTMenu, CClientItemCTMenu> );
	pt.y += 50;
	AddButton( '0', L( "Cancel" ), pt, VoidCb( &CMenuBaseItem::Hide ) );

	pt.x = 400;
	pt.y = 180;
	AddButton( 0, L( "Cstrike_BuyMenuAutobuy" ), pt, ExecAndHide( "autobuy" ) );
	pt.y += 50;
	AddButton( 0, L( "Cstrike_BuyMenuRebuy" ), pt, ExecAndHide( "rebuy" ) );
	pt.y += 50;

	autoFill.szName = L( "Auto buy ammo" );
	autoFill.pos = pt;
	autoFill.SetCharSize( QM_SMALLFONT );
	autoFill.colorBase = uiPromptTextColor;
	autoFill.LinkCvar( "ui_cs_autofill" );
	autoFill.onChanged = CMenuEditable::WriteCvarCb;

	AddItem( autoFill );
}
void CClientPistolsTMenu::_Init()
{
	szName = L( "Cstrike_PistolsLabel" );
	eAmmoClass = BUY_SECAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "Glock18", "glock18", "glock", "Glock" });
	AddWeapon( BuyMenuWeaponInfo{ "USP45", "usp45", "usp", "USP45" });
	AddWeapon( BuyMenuWeaponInfo{ "P228", "p228", "p228", "P228" });
	AddWeapon( BuyMenuWeaponInfo{ "DesertEagle", "deserteagle", "deagle", "DesertEagle" });
	AddWeapon( BuyMenuWeaponInfo{ "Elites", "elites", "elites", "Elites" });

}
void CClientPistolsCTMenu::_Init()
{
	szName = L( "Cstrike_PistolsLabel" );
	eAmmoClass = BUY_SECAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "Glock18", "glock18", "glock", "Glock" });
	AddWeapon( BuyMenuWeaponInfo{ "USP45", "usp45", "usp", "USP45" });
	AddWeapon( BuyMenuWeaponInfo{ "P228", "p228", "p228", "P228" });
	AddWeapon( BuyMenuWeaponInfo{ "DesertEagle", "deserteagle", "deagle", "DesertEagle" });
	AddWeapon( BuyMenuWeaponInfo{ "FiveSeven", "fiveseven", "fiveseven", "FiveSeven" });
}
void CClientShotgunsMenu::_Init()
{
	szName = L( "Cstrike_ShotgunsLabel" );
	eAmmoClass = BUY_PRIMAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "M3", "m3", "m3", "M3" });
	AddWeapon( BuyMenuWeaponInfo{ "XM1014", "xm1014", "xm1014",	"XM1014" });
}

void CClientSubMachineGunsTMenu::_Init()
{
	szName = L( "Cstrike_SubmachinegunsLabel" );
	eAmmoClass = BUY_PRIMAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "MAC10", "mac10", "mac10", "MAC10" });
	AddWeapon( BuyMenuWeaponInfo{ "MP5", "mp5", "mp5", "MP5" });
	AddWeapon( BuyMenuWeaponInfo{ "UMP45", "ump45", "ump45", "UMP45" });
	AddWeapon( BuyMenuWeaponInfo{ "P90", "p90", "p90", "P90" });
}
void CClientSubMachineGunsCTMenu::_Init()
{
	szName = L( "Cstrike_SubmachinegunsLabel" );
	eAmmoClass = BUY_PRIMAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "Tmp", "tmp", "tmp", "TMP" });
	AddWeapon( BuyMenuWeaponInfo{ "MP5", "mp5", "mp5", "MP5" });
	AddWeapon( BuyMenuWeaponInfo{ "UMP45", "ump45", "ump45", "UMP45" });
	AddWeapon( BuyMenuWeaponInfo{ "P90", "p90", "p90", "P90" });
}
void CClientRiflesTMenu::_Init()
{
	szName = L( "Cstrike_RiflesLabel" );
	eAmmoClass = BUY_PRIMAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "Galil", "galil", "galil", "Galil" });
	AddWeapon( BuyMenuWeaponInfo{ "AK47", "ak47", "ak47", "AK47" });
	AddWeapon( BuyMenuWeaponInfo{ "Scout_TER", "scout", "scout", "Scout" });
	AddWeapon( BuyMenuWeaponInfo{ "SG552", "sg552", "sg552", "SG552" });
	AddWeapon( BuyMenuWeaponInfo{ "AWP_TER", "awp", "awp", "AWP" });
	AddWeapon( BuyMenuWeaponInfo{ "G3SG1", "g3sg1", "g3sg1", "G3SG1" });

}
void CClientRiflesCTMenu::_Init()
{
	szName = L( "Cstrike_RiflesLabel" );
	eAmmoClass = BUY_PRIMAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "Famas", "famas", "famas", "Famas" });
	AddWeapon( BuyMenuWeaponInfo{ "Scout_CT", "scout", "scout", "Scout" });
	AddWeapon( BuyMenuWeaponInfo{ "M4A1", "m4a1", "m4a1", "M4A1" });
	AddWeapon( BuyMenuWeaponInfo{ "Aug", "aug", "aug", "Aug" });
	AddWeapon( BuyMenuWeaponInfo{ "SG550", "sg550", "sg550", "SG550" });
	AddWeapon( BuyMenuWeaponInfo{ "AWP_CT", "awp", "awp", "AWP" });

}
void CClientMachineGunsMenu::_Init()
{
	szName = L( "Cstrike_MachinegunsLabel" );
	eAmmoClass = BUY_PRIMAMMO;

	AddWeapon( BuyMenuWeaponInfo{ "M249", "m249", "m249", "M249" });
}
void CClientItemTMenu::_Init()
{
	szName = L( "Cstrike_EquipmentLabel" );
	eAmmoClass = BUY_EQUIPMENT;

	AddWeapon( BuyMenuWeaponInfo{ "Kevlar", "kevlar", "vest", "Kevlar" });
	AddWeapon( BuyMenuWeaponInfo{ "Kevlar_Helmet", "kevlar_helmet", "vesthelm", "KevlarHelmet" });
	AddWeapon( BuyMenuWeaponInfo{ "Flashbang", "flashbang", "flash", "Flashbang" });
	AddWeapon( BuyMenuWeaponInfo{ "HE_Grenade", "hegrenade", "hegren", "Flashbang" });
	AddWeapon( BuyMenuWeaponInfo{ "Smoke_Grenade", "smokegrenade", "sgren", "SmokeGrenade" });
	AddWeapon( BuyMenuWeaponInfo{ "NightVision_Button_TER", "nightvision", "nvgs", "Nightvision" });
}
void CClientItemCTMenu::_Init()
{
	szName = L( "Cstrike_EquipmentLabel" );
	eAmmoClass = BUY_EQUIPMENT;

	AddWeapon( BuyMenuWeaponInfo{ "Kevlar", "kevlar", "vest", "Kevlar" });
	AddWeapon( BuyMenuWeaponInfo{ "Kevlar_Helmet", "kevlar_helmet", "vesthelm", "KevlarHelmet" });
	AddWeapon( BuyMenuWeaponInfo{ "Flashbang", "flashbang", "flash", "Flashbang" });
	AddWeapon( BuyMenuWeaponInfo{ "HE_Grenade", "hegrenade", "hegren", "Flashbang" });
	AddWeapon( BuyMenuWeaponInfo{ "Smoke_Grenade", "smokegrenade", "sgren", "SmokeGrenade" });
	AddWeapon( BuyMenuWeaponInfo{ "Defuser", "defuser", "defuser", "Defuser" });
	AddWeapon( BuyMenuWeaponInfo{ "NightVision_Button_CT", "nightvision", "nvgs", "Nightvision" });
	AddWeapon( BuyMenuWeaponInfo{ "Shield", "shield", "shield", "Shield" });
}
