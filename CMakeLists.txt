#
# Copyright (c) 2015 Pavlo Lavrenenko
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 2.6.0)
project (MAINUI)

set (CMAKE_CXX_STANDARD 11)

set (MAINUI_LIBRARY xashmenu)

set (MAINUI_CONTROLS_SOURCES
	controls/Framework.cpp
	controls/BaseItem.cpp
	controls/Action.cpp
	controls/Bitmap.cpp
	controls/CheckBox.cpp
	controls/ItemsHolder.cpp
	controls/Field.cpp
	controls/PicButton.cpp
	controls/ScrollList.cpp
	controls/Slider.cpp
	controls/SpinControl.cpp
	controls/YesNoMessageBox.cpp
	controls/MessageBox.cpp
	controls/Editable.h
	controls/Switch.cpp
	controls/BaseWindow.cpp
	controls/ProgressBar.cpp
	)

set (MAINUI_MENUS_SOURCES
	menus/AdvancedControls.cpp
	menus/Audio.cpp
	menus/Configuration.cpp
	menus/Controls.cpp
	menus/CreateGame.cpp
	menus/Credits.cpp
	menus/CustomGame.cpp
	menus/FileDialog.cpp
	menus/GameOptions.cpp
	menus/Gamepad.cpp
	menus/LoadGame.cpp
	menus/Main.cpp
	menus/Multiplayer.cpp
	menus/NewGame.cpp
	menus/PlayerSetup.cpp
	menus/SaveLoad.cpp
	menus/ServerBrowser.cpp
	menus/TouchButtons.cpp
	menus/Touch.cpp
	menus/TouchEdit.cpp
	menus/TouchOptions.cpp
	menus/Video.cpp
	menus/VideoModes.cpp
	menus/VideoOptions.cpp
	menus/Zoo.cpp
	menus/ConnectionProgress.cpp
	menus/dynamic/ScriptMenu.cpp
	)

set (MAINUI_SOURCES
	font/FontManager.cpp
	font/WinAPIFont.cpp
	font/FreeTypeFont.cpp
	font/BaseFontBackend.cpp
	EventSystem.h
	EngineCallback.cpp
	BaseMenu.cpp
	Btns.cpp
	MenuStrings.cpp
	CFGScript.cpp
	Utils.cpp
	Scissor.cpp
	udll_int.cpp
	)

list(APPEND MAINUI_SOURCES ${MAINUI_CONTROLS_SOURCES})
list(APPEND MAINUI_SOURCES ${MAINUI_MENUS_SOURCES})
add_library (${MAINUI_LIBRARY} SHARED ${MAINUI_SOURCES})

add_compile_options( -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable )

if( NOT XASH_SDK )
	set( XASH_SDK "../" )
endif( )

include_directories (${XASH_SDK}/common ${XASH_SDK}/engine ${XASH_SDK}/pm_shared . controls/ menus/ utl/ font/)


if( MSVC )
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif( )

# Font Rendering(FreeType or WinAPI)
if( MAINUI_CPP_ENABLE_FONT_RENDER )
	if( NOT WIN32 )
		find_package(PkgConfig)
		pkg_check_modules(FC REQUIRED fontconfig)
		include_directories( ${FC_INCLUDE_DIRS} )
		target_link_libraries( ${MAINUI_LIBRARY} ${FC_LIBRARIES} )
		add_definitions( -DMAINUI_USE_FREETYPE )
	endif( )
	add_definitions( -DMAINUI_USE_CUSTOM_FONT_RENDER )
endif( )

set_target_properties (${MAINUI_LIBRARY} PROPERTIES
	POSITION_INDEPENDENT_CODE 1 )

if( WIN32 )
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
		OUTPUT_NAME menu )
endif()

install( TARGETS ${MAINUI_LIBRARY} DESTINATION ${LIB_INSTALL_DIR}/xash3d 
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

