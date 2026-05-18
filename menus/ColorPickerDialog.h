/*
ColorPickerDialog.h - color picker dialog window
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

#pragma once
#ifndef COLORPICKERDIALOG_H
#define COLORPICKERDIALOG_H

#include "Framework.h"
#include "PicButton.h"
#include "Field.h"
#include "SpinControl.h"
#include "ColorPicker.h"
#include "StringArrayModel.h"

class CMenuPlayerSetup;

class CMenuColorPickerDialog : public CMenuBaseWindow
{
public:
	typedef CMenuBaseWindow BaseClass;
	CMenuColorPickerDialog();

	void Show( byte r, byte g, byte b, HIMAGE logoImage );
	CEventCallback onOk;

	void GetRGB( byte &r, byte &g, byte &b ) const
	{
		m_picker.GetRGB( r, g, b );
	}

private:
	void _Init() override;
	void _VidInit() override;
	void Draw() override;

	void UpdateLogoPreview();
	void UpdateFieldsFromPicker();
	void UpdatePickerFromFields();
	void SwitchMode( bool toRGB );

	CMenuColorPicker  m_picker;
	CMenuPicButton    m_btnOk;
	CMenuPicButton    m_btnCancel;
	CMenuSpinControl  m_modeSwitch;
	CMenuField        m_fields[3];
	bool              m_modeRGB;
	bool              m_updatingFields;

	struct FieldLabel { Point pos; Size size; const char *text; } m_labels[3];

	class CLogoPreview : public CMenuBaseItem
	{
	public:
		void Draw() override;
		HIMAGE hImage = 0;
		byte   r = 255, g = 255, b = 255;
	} m_logoPreviewItem;
};

#endif // COLORPICKERDIALOG_H
