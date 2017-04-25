#pragma once
#ifndef BASEITEM_H
#define BASEITEM_H

enum
{
	QMF_GRAYED             = BIT( 1 ), // Grays and disables
	QMF_INACTIVE           = BIT( 2 ), // Disables any input
	QMF_HIDDEN             = BIT( 3 ), // Doesn't draw
	QMF_DROPSHADOW         = BIT( 4 ),
	QMF_SILENT             = BIT( 5 ), // Don't play sounds
	QMF_HASMOUSEFOCUS      = BIT( 6 ),
	QMF_MOUSEONLY          = BIT( 7 ), // Only mouse input allowed
	QMF_FOCUSBEHIND        = BIT( 8 ), // Focus draws behind normal item
	QMF_NOTIFY             = BIT( 9 ), // draw notify at right screen side
	QMF_ACT_ONRELEASE      = BIT( 10 ), // call Key_Event when button is released
	QMF_HASKEYBOARDFOCUS   = BIT( 11 ),
	QMF_DIALOG             = BIT( 12 ) // dialogs can't be switched inactive by framework
};

enum ETextAlignment
{
	QM_LEFT = 0, // justify by left
	QM_CENTER,   // justify by center
	QM_RIGHT     // justify by right
};

enum EFontSizes
{
	QM_DEFAULTFONT = 0, // medium size font
	QM_SMALLFONT,       // small
	QM_BIGFONT          // big
};

enum EFocusAnimation
{
	QM_NOFOCUSANIMATION = 0,
	QM_HIGHLIGHTIFFOCUS,      // just simple hightlight
	QM_PULSEIFFOCUS           // pulse animation
};

enum ELetterCase
{
	QM_NOLETTERCASE = 0,
	QM_LOWERCASE,
	QM_UPPERCASE
};

class CMenuItemsHolder;
class CMenuBaseItem
{
public:
	friend class CMenuItemsHolder;

	// The main constructor
	CMenuBaseItem();
	virtual ~CMenuBaseItem();

	// Init is called when Item is added to Framework
	// Called once by Framework
	virtual void Init( void );

	// VidInit is called after VidInit method of Framework
	// VidInit can be called multiple times
	virtual void VidInit( void );

	// Key is called every key press
	// Must return sound name, or NULL
	virtual const char *Key( int key, int down );

	// Draw is called every frame
	virtual void Draw( void );

	// Char is a special key press event for text input
	virtual void Char( int key );

	virtual void MouseMove( int x, int y ) {}

	virtual const char *Activate( void );

	virtual void ToggleInactive( void )
	{
		iFlags ^= QMF_INACTIVE;
	}

	virtual void SetInactive( bool visible )
	{
		if( visible ) iFlags |= QMF_INACTIVE;
		else iFlags &= ~QMF_INACTIVE;
	}

	virtual void Show() { iFlags &= ~QMF_HIDDEN; }

	virtual void Hide() { iFlags |= QMF_HIDDEN;  }

	virtual bool IsVisible() { return !(iFlags & QMF_HIDDEN); }

	void ToggleVisibility()
	{
		if( IsVisible() ) Hide();
		else Show();
	}

	void SetVisibility( bool show )
	{
		if( show ) Show();
		else Hide();
	}

	// Checks item is current selected in parent Framework
	bool IsCurrentSelected( void );


	CEventCallback onGotFocus;
	CEventCallback onLostFocus;
	CEventCallback onActivated;
	CEventCallback onChanged;
	CEventCallback onPressed;

	// called when CL_IsActive returns true, otherwise onActivate
	CEventCallback onActivatedClActive;

	void SetCoord( int x, int y )                { pos.x = x, pos.y = y; }
	void SetSize( int w, int h )                 { size.w = w; size.h = h; }
	void SetRect( int x, int y, int w, int h )   { SetCoord( x, y ); SetSize( w, h ); }
	void SetCharSize( int w, int h )             { charSize.w = w; charSize.h = h; }
	void SetCharSize( EFontSizes fs );
	void SetNameAndStatus( const char *name, const char *status ) { szName = name, szStatusText = status; }

	CMenuItemsHolder* Parent() const			{ return m_pParent; }
	template <class T> T* Parent() const	{ return (T*) m_pParent; } // a shortcut to parent
	bool IsPressed() const { return m_bPressed; }
	int LastFocusTime() const { return m_iLastFocusTime; }

	unsigned int iFlags;

	Point pos;
	Size size;
	Size charSize;

	const char *szName;
	const char *szStatusText;

	unsigned int iColor;
	unsigned int iFocusColor;

	ETextAlignment eTextAlignment;
	EFocusAnimation eFocusAnimation;
	ELetterCase eLetterCase;

protected:
	// calls specific EventCallback
	virtual void _Event( int ev );

	CMenuItemsHolder	*m_pParent;
	bool	m_bPressed;
	int		m_iLastFocusTime;

	Point m_scPos;
	Size m_scSize;
	Size m_scChSize;
};

#endif // BASEITEM_H
