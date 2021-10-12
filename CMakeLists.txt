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

cmake_minimum_required(VERSION 2.8.12)
project(MAINUI)

# By default we require C++11 support.
# But it can be overriden with -DMY_COMPILER_SUCKS compiler option
# It's not recommended and may not work(as it's done only for MSVC6 support)
set(CMAKE_CXX_STANDARD 11)

set(MAINUI_LIBRARY xashmenu)

option(MAINUI_USE_CUSTOM_FONT_RENDER "Use custom font rendering" ON)
option(MAINUI_USE_STB "Use stb_truetype.h for rendering(*nix-only)" OFF)
option(MAINUI_RENDER_PICBUTTON_TEXT "Use custom font render to render picbuttons (experimental)" OFF)
option(MAINUI_FONT_SCALE "Scale fonts by height" OFF)
option(MAINUI_FORCE_PIE "Force enable -fpie/-fPIC" OFF) # may be required by some systems, although not recommended for i386

file(GLOB MAINUI_CONTROLS_SOURCES "controls/*.cpp")
file(GLOB MAINUI_MENUS_SOURCES "menus/*.cpp" "menus/dynamic/*.cpp")
file(GLOB MAINUI_FONT_RENDER_SOURCES "font/*.cpp")
file(GLOB MAINUI_SOURCES "miniutl/*.cpp" "*.cpp")

if(CS16CLIENT)
	add_definitions(-DCS16CLIENT)
	list(APPEND MAINUI_SOURCES
		${XASH_SDK}/common/interface.cpp
		menus/Scoreboard.cpp
		menus/client/JoinGame.cpp
		menus/client/JoinClass.cpp
		menus/client/ClientWindow.cpp
		)
endif()

list(APPEND MAINUI_SOURCES ${MAINUI_CONTROLS_SOURCES})
list(APPEND MAINUI_SOURCES ${MAINUI_MENUS_SOURCES})
list(APPEND MAINUI_SOURCES ${MAINUI_FONT_RENDER_SOURCES})
add_library(${MAINUI_LIBRARY} MODULE ${MAINUI_SOURCES})

if(NOT WIN32 AND NOT MINGW)
	add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable)
else()

	# Only for MSVC6 compatibility mode! Newer VC does fail with this define
	# add_definitions(-DMY_COMPILER_SUCKS)
endif()

if(NOT XASH_SDK)
	set(XASH_SDK "../")
endif()

if(XASH_SAILFISH)
	add_definitions(-D__SAILFISH__)
endif()

if(MAINUI_FONT_SCALE)
	add_definitions(-DMAINUI_FONT_SCALE)
endif()

# Force stb_truetype for Apple devices, as there is no Apple native font renderer
# And freetype&fontconfig isn't used on this platform by default
if(APPLE OR ANDROID)
	set(MAINUI_USE_STB TRUE)
endif()

include_directories(${XASH_SDK}/common ${XASH_SDK}/engine ${XASH_SDK}/pm_shared . controls/ menus/ miniutl/ font/ model/)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Font Rendering(FreeType or WinAPI)
if(MAINUI_USE_CUSTOM_FONT_RENDER)
	# Win32 will always use GDI font renderer
	if(NOT WIN32)
		if(MAINUI_USE_STB)
			# Use stbtt
			add_definitions(-DMAINUI_USE_STB)
		else()
			# Use freetype2
			find_package(PkgConfig)
			pkg_check_modules(FC REQUIRED fontconfig)
			include_directories(${FC_INCLUDE_DIRS})
			target_link_libraries(${MAINUI_LIBRARY} ${FC_LIBRARIES})
			add_definitions(-DMAINUI_USE_FREETYPE)
		endif()
	endif()

	add_definitions(-DMAINUI_USE_CUSTOM_FONT_RENDER)
	if(MAINUI_RENDER_PICBUTTON_TEXT)
		add_definitions(-DMAINUI_RENDER_PICBUTTON_TEXT)
	endif()
endif()

# Name library as it named in Unkle Mike's Xash3D
if(MAINUI_NAME)
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
		OUTPUT_NAME ${MAINUI_NAME})
elseif(WIN32)
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
		OUTPUT_NAME menu PREFIX "")
endif()

if(XASH_64BIT AND NOT ANDROID)
	get_target_property(MAINUI_NAME ${MAINUI_LIBRARY} OUTPUT_NAME)
	if(NOT MAINUI_NAME)
		set(MAINUI_NAME ${MAINUI_LIBRARY})
	endif()
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES 
		OUTPUT_NAME ${MAINUI_NAME}64)
endif()

if(MAINUI_FORCE_PIE OR (APPLE AND CMAKE_OSX_ARCHITECTURES) OR NOT (ARCH STREQUAL "i386"))
	message(STATUS "Enabled PIE for ${MAINUI_LIBRARY}")
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
		POSITION_INDEPENDENT_CODE 1)
else()
	message(STATUS "Disabled PIE for ${MAINUI_LIBRARY}")
endif()

install(TARGETS ${MAINUI_LIBRARY} DESTINATION .)

if(MSVC)
	install(FILES $<TARGET_PDB_FILE:${MAINUI_LIBRARY}> DESTINATION . OPTIONAL)
endif()
