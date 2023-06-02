#! /usr/bin/env python
# encoding: utf-8
# a1batross, mittorn, 2018

from waflib import Logs, Configure
import os

top = '.'

FT2_CHECK='''extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

int main() { return FT_Init_FreeType( NULL ); }
'''

FC_CHECK='''extern "C" {
#include <fontconfig/fontconfig.h>
}

int main() { return (int)FcInit(); }
'''

def options(opt):
	grp = opt.add_option_group('MainUI C++ options')
	grp.add_option('--enable-stbtt', action = 'store_true', dest = 'USE_STBTT', default = False,
		help = 'prefer stb_truetype.h over freetype [default: %default]')

	return

def configure(conf):
	# conf.env.CXX11_MANDATORY = False
	conf.load('fwgslib cxx11')

	if not conf.env.HAVE_CXX11:
		conf.define('MY_COMPILER_SUCKS', 1)

	conf.env.USE_STBTT = conf.options.USE_STBTT
	conf.define('MAINUI_USE_CUSTOM_FONT_RENDER', 1)

	nortti = {
		'msvc': ['/GR-'],
		'default': ['-fno-rtti']
	}

	conf.env.append_unique('CXXFLAGS', conf.get_flags_by_compiler(nortti, conf.env.COMPILER_CC))

	if conf.env.DEST_OS == 'darwin' or conf.env.DEST_OS == 'android' or conf.env.MAGX:
		conf.env.USE_STBTT = True
		conf.define('MAINUI_USE_STB', 1)

	if conf.env.DEST_OS == 'android':
		conf.define('NO_STL', 1)
		conf.env.append_unique('CXXFLAGS', '-fno-exceptions')

	if conf.env.DEST_OS != 'win32' and conf.env.DEST_OS != 'dos':
		if not conf.env.USE_STBTT and not conf.options.LOW_MEMORY:
			conf.check_pkg('freetype2', 'FT2', FT2_CHECK)
			conf.check_pkg('fontconfig', 'FC', FC_CHECK)
			conf.define('MAINUI_USE_FREETYPE', 1)
		conf.check_cxx(lib='rt', mandatory=False)

def build(bld):
	libs = []

	if bld.env.DEST_OS != 'win32':
		if not bld.env.USE_STBTT:
			libs += ['FT2', 'FC']
		libs += ['RT']
	else:
		libs += ['GDI32', 'USER32']

	if bld.env.DEST_OS == 'linux':
		libs += ['RT']

	source = bld.path.ant_glob([
		'*.cpp',
		'miniutl/*.cpp',
		'font/*.cpp',
		'menus/*.cpp',
		'menus/dynamic/*.cpp',
		'model/*.cpp',
		'controls/*.cpp'
	])

	includes = [
		'.',
		'miniutl/',
		'font/',
		'controls/',
		'menus/',
		'model/',
		'sdk_includes/common',
		'sdk_includes/engine',
		'sdk_includes/public',
		'sdk_includes/pm_shared'
	]

	bld.shlib(
		source   = source,
		target   = 'menu',
		features = 'cxx',
		includes = includes,
		use      = libs,
		install_path = bld.env.LIBDIR,
		subsystem = bld.env.MSVC_SUBSYSTEM,
		cmake_skip = True
	)
