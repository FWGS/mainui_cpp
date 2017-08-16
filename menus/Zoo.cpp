#include "BaseWindow.h"
#include "Action.h"
#include "Field.h"

class CMenuZoo : public CMenuBaseWindow
{
public:
	virtual void _Init();
	virtual void _VidInit();

	static void OKButtonCommand( CMenuBaseItem *pSelf, void *pExtra )
	{
		pSelf->Parent()->Hide();
	}

	static void CancelButtonCommand( CMenuBaseItem *pSelf, void *pExtra )
	{
		pSelf->Parent()->Hide();
	}

private:
	CMenuAction OKButton;
	CMenuAction CancelButton;
	CMenuAction Label1;
	CMenuAction Label2;
	CMenuAction Label3;
	CMenuAction Label4;
	CMenuField Entry1;
	CMenuField Entry2;
	CMenuField Entry3;
	CMenuField Entry4;
	CMenuField Entry5;
} g_Zoo;

#define SET_TAG_BY_MEMBER_NAME( member ) \
	member.szTag = STR( member ); \
	AddItem( member );

void CMenuZoo::_Init()
{
	szTag = "CDKeyEntryDialog";
	iFlags = QMF_DIALOG;

	RegisterNamedEvent( OKButtonCommand, "Ok" );
	RegisterNamedEvent( CancelButtonCommand, "Cancel" );

	background.bForceColor = true;

	AddItem( background );
	SET_TAG_BY_MEMBER_NAME( OKButton );
	SET_TAG_BY_MEMBER_NAME( CancelButton );
	SET_TAG_BY_MEMBER_NAME( Label1 );
	SET_TAG_BY_MEMBER_NAME( Label2 );
	SET_TAG_BY_MEMBER_NAME( Label3 );
	SET_TAG_BY_MEMBER_NAME( Label4 );
	SET_TAG_BY_MEMBER_NAME( Entry1 );
	SET_TAG_BY_MEMBER_NAME( Entry2 );
	SET_TAG_BY_MEMBER_NAME( Entry3 );
	//SET_TAG_BY_MEMBER_NAME( Entry4 );
	//SET_TAG_BY_MEMBER_NAME( Entry5 );
}

void CMenuZoo::_VidInit()
{

}

void UI_Zoo_Precache()
{
	g_Zoo.SetResourceFilename("resource/CDKeyEntryDialog.res");
}

void UI_Zoo_Menu()
{
	UI_Zoo_Precache();
	g_Zoo.Show();
}
