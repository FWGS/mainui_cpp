/*
ColorPickerDialog.cpp - color picker dialog window
Copyright (C) 2026 $_Vladislav

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "ColorPickerDialog.h"
#include "BaseMenu.h"
#include "extdll_menu.h"
#include "fmtstr.h"

namespace Layout
{
	static const int DLG_W              = 600;
	static const int DLG_H              = 430;
	static const int PAD                = 20;
	static const int TITLE_H            = 28;
	static const int PICKER_W           = 300;
	static const int PICKER_H           = 220;
	static const int LOGO_W             = 180;
	static const int LOGO_H             = 180;
	static const int BTN_W              = 120;
	static const int BTN_H              = 36;
	static const int FIELD_W            = 56;
	static const int FIELD_H            = 28;
	static const int LABEL_W            = 18;
	static const int MODE_W             = 140;
	static const int INPUT_X            = 102;
	static const int UI_VIRTUAL_HEIGHT  = 768;

	static const int CONTENT_GAP        = 16;
	static const int INPUT_GAP          = 24;
	static const int INPUT_GROUP_GAP    = 14;
	static const int FIELD_GAP          = 8;
	static const int LABEL_GAP          = 4;
	static const int LABEL_Y_OFFSET     = 4;
	static const int BTN_GAP            = 16;
}

CMenuColorPickerDialog::CMenuColorPickerDialog()
	: CMenuBaseWindow( "CMenuColorPickerDialog" ),
	  m_modeRGB( true ), m_updatingFields( false )
{
}

void CMenuColorPickerDialog::_Init()
{
	SetRect( ( uiStatic.width - Layout::DLG_W ) / 2, ( Layout::UI_VIRTUAL_HEIGHT - Layout::DLG_H ) / 2, Layout::DLG_W, Layout::DLG_H );

	int currentY = Layout::PAD + Layout::TITLE_H + Layout::CONTENT_GAP;

	m_picker.SetRect( Layout::PAD, currentY, Layout::PICKER_W, Layout::PICKER_H );
	SET_EVENT_MULTI( m_picker.onChanged,
	{
		CMenuColorPickerDialog *dlg = (CMenuColorPickerDialog *)pSelf->Parent();
		dlg->UpdateLogoPreview();
		dlg->UpdateFieldsFromPicker();
	});

	int logoX = Layout::DLG_W - Layout::PAD - Layout::LOGO_W;
	int logoY = currentY + ( Layout::PICKER_H - Layout::LOGO_H ) / 2;
	m_logoPreviewItem.SetRect( logoX, logoY, Layout::LOGO_W, Layout::LOGO_H );

	currentY += Layout::PICKER_H + Layout::INPUT_GAP;

	static const char *modeItems[] = { "RGB", "HSV" };
	static CStringArrayModel modeModel( modeItems, 2 );

	m_modeSwitch.Setup( &modeModel );
	m_modeSwitch.SetCurrentValue( 0.f );
	m_modeSwitch.SetRect( Layout::INPUT_X, currentY, Layout::MODE_W, Layout::FIELD_H );
	SET_EVENT_MULTI( m_modeSwitch.onChanged,
	{
		CMenuColorPickerDialog *dlg   = (CMenuColorPickerDialog *)pSelf->Parent();
		bool                    toRGB = ( ( (CMenuSpinControl *)pSelf )->GetCurrentValue() < 0.5f );
		dlg->SwitchMode( toRGB );
	});

	static const char *rgbLabels[3] = { "R", "G", "B" };
	int                fieldX       = Layout::INPUT_X + Layout::MODE_W + Layout::INPUT_GROUP_GAP;

	for( int i = 0; i < 3; i++ )
	{
		int lx = fieldX + i * ( Layout::LABEL_W + Layout::FIELD_W + Layout::FIELD_GAP );
		int fx = lx + Layout::LABEL_W + Layout::LABEL_GAP;

		m_labels[i].pos  = { lx, currentY + Layout::LABEL_Y_OFFSET };
		m_labels[i].size = { Layout::LABEL_W, Layout::FIELD_H };
		m_labels[i].text = rgbLabels[i];

		m_fields[i].iMaxLength   = 3;
		m_fields[i].bNumbersOnly = true;
		m_fields[i].SetRect( fx, currentY, Layout::FIELD_W, Layout::FIELD_H );

		SET_EVENT_MULTI( m_fields[i].onChanged,
		{
			CMenuColorPickerDialog *dlg = (CMenuColorPickerDialog *)pSelf->Parent();
			dlg->UpdatePickerFromFields();
		});
	}

	const int buttonY   = Layout::DLG_H - Layout::PAD - Layout::BTN_H;
	// Center the gap between OK and Cancel buttons exactly on the right edge of the color picker column (PAD + PICKER_W)
	const int buttonsX  = ( Layout::PAD + Layout::PICKER_W ) - Layout::BTN_W - ( Layout::BTN_GAP / 2 );

	m_btnOk.SetPicture( PC_OK );
	m_btnOk.szName = L( "OK" );
	m_btnOk.eTextAlignment = QM_CENTER;
	m_btnOk.SetRect( buttonsX, buttonY, Layout::BTN_W, Layout::BTN_H );
	SET_EVENT_MULTI( m_btnOk.onReleased,
	{
		CMenuColorPickerDialog *dlg = (CMenuColorPickerDialog *)pSelf->Parent();
		if( dlg->onOk )
			dlg->onOk( dlg );
		dlg->Hide();
	});

	m_btnCancel.SetPicture( PC_CANCEL );
	m_btnCancel.szName = L( "Cancel" );
	m_btnCancel.eTextAlignment = QM_CENTER;
	m_btnCancel.SetRect( buttonsX + Layout::BTN_W + Layout::BTN_GAP, buttonY, Layout::BTN_W, Layout::BTN_H );
	SET_EVENT_MULTI( m_btnCancel.onReleased,
	{
		CMenuColorPickerDialog *dlg = (CMenuColorPickerDialog *)pSelf->Parent();
		dlg->Hide();
	});

	AddItem( m_picker );
	AddItem( m_logoPreviewItem );
	AddItem( m_modeSwitch );
	for( int i = 0; i < 3; i++ )
		AddItem( m_fields[i] );
	AddItem( m_btnOk );
	AddItem( m_btnCancel );
}

void CMenuColorPickerDialog::_VidInit()
{
	SetRect( ( uiStatic.width - Layout::DLG_W ) / 2, ( Layout::UI_VIRTUAL_HEIGHT - Layout::DLG_H ) / 2, Layout::DLG_W, Layout::DLG_H );
	pos.x += uiStatic.xOffset;
	pos.y += uiStatic.yOffset;
}

void CMenuColorPickerDialog::Draw()
{
	UI_FillRect( 0, 0, gpGlobals->scrWidth, gpGlobals->scrHeight, 0x40000000 );

	EngFuncs::FillRGBA( m_scPos.x, m_scPos.y, m_scSize.w, m_scSize.h, 20, 20, 20, 235 );

	UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );

	UI_DrawString( font, m_scPos.x, m_scPos.y + Layout::PAD * uiStatic.scaleY, m_scSize.w, Layout::TITLE_H * uiStatic.scaleY, L( "Logo Color" ), uiColorHelp, m_scChSize, QM_CENTER, ETF_SHADOW );

	EngFuncs::FillRGBA( m_scPos.x + Layout::PAD * uiStatic.scaleX, m_scPos.y + ( Layout::PAD + Layout::TITLE_H ) * uiStatic.scaleY, m_scSize.w - Layout::PAD * 2 * uiStatic.scaleX, 1, 255, 255, 255, 40 );

	for( int i = 0; i < 3; i++ )
	{
		int lx = m_scPos.x + m_labels[i].pos.x * uiStatic.scaleX;
		int ly = m_scPos.y + m_labels[i].pos.y * uiStatic.scaleY;
		int lw = m_labels[i].size.w * uiStatic.scaleX;
		int lh = m_labels[i].size.h * uiStatic.scaleY;

		UI_DrawString( font, lx, ly, lw, lh, m_labels[i].text, uiColorHelp, m_scChSize, QM_LEFT, ETF_SHADOW );
	}

	BaseClass::Draw();
}

void CMenuColorPickerDialog::CLogoPreview::Draw()
{
	if( !hImage )
	{
		UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
		UI_DrawString( font, m_scPos, m_scSize, L( "No logo" ), colorBase, m_scChSize, QM_CENTER, ETF_SHADOW );
	}
	else
	{
		EngFuncs::PIC_Set( hImage, r, g, b );
		EngFuncs::PIC_DrawTrans( m_scPos, m_scSize );
	}
	UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );
}

void CMenuColorPickerDialog::UpdateLogoPreview()
{
	byte r, g, b;
	m_picker.GetRGB( r, g, b );
	m_logoPreviewItem.r = r;
	m_logoPreviewItem.g = g;
	m_logoPreviewItem.b = b;
}

void CMenuColorPickerDialog::UpdateFieldsFromPicker()
{
	if( m_updatingFields )
		return;
	m_updatingFields = true;

	if( m_modeRGB )
	{
		byte r, g, b;
		m_picker.GetRGB( r, g, b );
		m_fields[0].SetBuffer( CNumStr( (int)r ) );
		m_fields[1].SetBuffer( CNumStr( (int)g ) );
		m_fields[2].SetBuffer( CNumStr( (int)b ) );
	}
	else
	{
		m_fields[0].SetBuffer( CNumStr( (int)( m_picker.GetHue() + 0.5f ) ) );
		m_fields[1].SetBuffer( CNumStr( (int)( m_picker.GetSat() * 100.f + 0.5f ) ) );
		m_fields[2].SetBuffer( CNumStr( (int)( m_picker.GetVal() * 100.f + 0.5f ) ) );
	}

	m_updatingFields = false;
}

void CMenuColorPickerDialog::UpdatePickerFromFields()
{
	if( m_updatingFields || m_picker.IsDragging() )
		return;
	m_updatingFields = true;

	if( m_modeRGB )
	{
		int r = Q_max( 0, Q_min( 255, atoi( m_fields[0].GetBuffer() ) ) );
		int g = Q_max( 0, Q_min( 255, atoi( m_fields[1].GetBuffer() ) ) );
		int b = Q_max( 0, Q_min( 255, atoi( m_fields[2].GetBuffer() ) ) );
		m_picker.SetRGB( (byte)r, (byte)g, (byte)b );
	}
	else
	{
		float h = Q_max( 0.f, Q_min( 360.f, (float)atoi( m_fields[0].GetBuffer() ) ) );
		float s = Q_max( 0.f, Q_min( 1.f, atoi( m_fields[1].GetBuffer() ) / 100.f ) );
		float v = Q_max( 0.f, Q_min( 1.f, atoi( m_fields[2].GetBuffer() ) / 100.f ) );
		m_picker.SetHSV( h, s, v );
	}

	UpdateLogoPreview();
	m_updatingFields = false;
}

void CMenuColorPickerDialog::SwitchMode( bool toRGB )
{
	m_modeRGB = toRGB;
	static const char *rgbLabels[3] = { "R", "G", "B" };
	static const char *hsvLabels[3] = { "H", "S", "V" };
	const char *const *labels       = toRGB ? rgbLabels : hsvLabels;
	for( int i = 0; i < 3; i++ )
		m_labels[i].text = labels[i];
	UpdateFieldsFromPicker();
}

void CMenuColorPickerDialog::Show( byte r, byte g, byte b, HIMAGE logoImage )
{
	m_picker.SetRGB( r, g, b, false );
	m_logoPreviewItem.hImage = logoImage;
	m_logoPreviewItem.r      = r;
	m_logoPreviewItem.g      = g;
	m_logoPreviewItem.b      = b;

	m_modeRGB = true;
	m_modeSwitch.SetCurrentValue( 0.f );
	SwitchMode( true );

	BaseClass::Show();
}
