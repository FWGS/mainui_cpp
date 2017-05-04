#ifndef CMENUCONNECTIONPROGRESS_H
#define CMENUCONNECTIONPROGRESS_H
#include "ItemsHolder.h"
#include "ProgressBar.h"
#include "PicButton.h"
class CMenuConnectionProgress : public CMenuItemsHolder
{
public:
    CMenuConnectionProgress();
	virtual void _Init();
	virtual void _VidInit();
	virtual void Draw();
	virtual const char *Key( int key, int down );
private:
	CMenuProgressBar precacheProgress;
	CMenuProgressBar downloadProgress;
	CMenuPicButton consoleButton;
	CMenuPicButton disconnectButton;
};

void UI_ConnectionProgress_f();

#endif // CMENUCONNECTIONPROGRESS_H
