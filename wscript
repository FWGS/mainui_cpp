#! /usr/bin/env python
# encoding: utf-8
# a1batross, mittorn, 2018

from waflib import Logs
import os

top = '.'

def options(opt):
	grp = opt.add_option_group('MainUI C++ options')
	grp.add_option('--enable-stbtt', action = 'store_true', dest = 'USE_STBTT',
		help = 'prefer stb_truetype.h over freetype')

	return

def configure(conf):
	# conf.env.CXX11_MANDATORY = False
	conf.load('fwgslib cxx11')
	if not conf.env.HAVE_CXX11:
		conf.env.append_unique('DEFINES', 'MY_COMPILER_SUCKS')

	conf.env.USE_STBTT = conf.options.USE_STBTT
	conf.env.append_unique('DEFINES', 'MAINUI_USE_CUSTOM_FONT_RENDER')

	nortti = {
		'msvc': ['/GR-'],
		'default': ['-fno-rtti']
	}

	conf.env.append_unique('CXXFLAGS', conf.get_flags_by_compiler(nortti, conf.env.COMPILER_CC))

	if conf.env.DEST_OS == 'darwin' or conf.env.DEST_OS2 == 'android':
		conf.env.USE_STBTT = True
		conf.env.append_unique('DEFINES', 'MAINUI_USE_STB')

	if conf.env.DEST_OS2 == 'android':
		conf.env.append_unique('DEFINES', 'NO_STL')
		conf.env.append_unique('CXXFLAGS', '-fno-exceptions')

	if conf.env.DEST_OS != 'win32':
		if not conf.env.USE_STBTT:
			errormsg = '{0} not available! Install {0} development package. Also you may need to set PKG_CONFIG_PATH environment variable'

			try:
				conf.check_cfg(package='freetype2', args='--cflags --libs', uselib_store='FT2' )
			except conf.errors.ConfigurationError:
				conf.fatal(errormsg.format('freetype2'))
			try:
				conf.check_cfg(package='fontconfig', args='--cflags --libs', uselib_store='FC')
			except conf.errors.ConfigurationError:
				conf.fatal(errormsg.format('fontconfig'))
			conf.env.append_unique('DEFINES', 'MAINUI_USE_FREETYPE');

def build(bld):
	libs = []

	# basic build: dedicated only, no dependencies
	if bld.env.DEST_OS != 'win32':
		if not bld.env.USE_STBTT:
			libs += ['FT2', 'FC']
	else:
		libs += ['GDI32', 'USER32']

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
		'../common',
		'../engine',
		'../pm_shared'
	]

	bld.shlib(
		source   = source,
		target   = 'menu',
		features = 'cxx',
		includes = includes,
		use      = libs,
		install_path = bld.env.LIBDIR,
		subsystem = bld.env.MSVC_SUBSYSTEM
	)
