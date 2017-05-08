#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

class CMenuMessageBox : public CMenuBaseWindow
{
public:
	CMenuMessageBox();

	void SetMessage( const char *sz );
private:
	void _Init();

	CMenuAction dlgMessage;
	CMenuAction background;
};

#endif // MESSAGEBOX_H
