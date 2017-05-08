#pragma once
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "BaseWindow.h"
class CMenuBaseItem;

class CMenuFramework : public CMenuBaseWindow
{
public:
	CMenuFramework();

	void Show();
	void Hide();
	bool IsVisible();
	bool IsRoot() { return true; }
};

#endif // FRAMEWORK_H
