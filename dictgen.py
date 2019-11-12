#!/usr/bin/env python3
# dictgen.py -- VGUI2 dictionary generator
# Copyright(C) 2019 a1batross
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import os
import sys
import re

EXTENSIONS = ('.cpp', '.h')
TRANSLATABLE_PATTERN = re.compile('L\s*\(\s*\".*?\"\s*\)')
STRING_LITERAL_PATTERN = re.compile('\".*?\"')

HEADER = '''"lang"
{
"Language" "<YOUR_LANGUAGE_HERE>"
"Tokens"
{
'''

FOOTER = '''}
}
'''

def process_file(name):
	trans = []

	# TODO: dumb! It can't find multilines
	with open(name, "r") as f:
		for line in f.readlines():
			trans += re.findall(TRANSLATABLE_PATTERN, line)

	return trans

def strip_quotes(s):
	return s[s.find('"') + 1 : s.rfind('"')]

def process_trans(trans):
	def extract_string_literal(s):
		return "".join(re.split(STRING_LITERAL_PATTERN, strip_quotes(s)))

	# #1: extract string literals from L(" ") and combine them into one
	trans = [extract_string_literal(t) for t in trans]

	# #2: filter unique
	trans = list(set(trans))

	# #3: remove strings.lst compatible strings
	trans = [t for t in trans if not t.startswith('StringsList_')]

	# #4: sort :)
	trans.sort()

	return trans

def vgui_translation_parse(name):
	trans = []
	parsing = False
	with open(name, "r", encoding = 'utf-16') as f:
		contents = f.read()
		strings = re.findall(STRING_LITERAL_PATTERN, contents)
		is_trans = True

		for t in strings:
			if not parsing:
				if t == '"Tokens"':
					parsing = True # now wait for strings

				continue

			if parsing and is_trans:
				trans += [strip_quotes(t)]

			is_trans = not is_trans

	return trans

def create_translations_file(name, trans):
	maxlength = len(max(trans, key=len)) + 1

	# we are working in UTF-8, it's easier to handle than UTF-16
	with open(name, "w", encoding = 'utf-8',newline = '\r\n') as f:
		f.write(HEADER)
		for t in trans:
			length = maxlength - len(t)

			f.write('"%s"%*s""\n' % (t, length, ' '))
		f.write(FOOTER)

		print('Created skeleton translation file at %s' % name)

def main():
	outfilefull = os.path.join('translations', 'gameui_skeleton_full.txt')
	outfile = os.path.join('translations', 'gameui_skeleton.txt')

	files = [ os.path.join(folder, name)
		for (folder, subs, files) in os.walk('.')
		for name in files + subs
		if name.endswith(EXTENSIONS) ]

	files.sort()

	trans = []
	for f in files:
		trans += process_file(f)

	trans = process_trans(trans)
	print('%d strings needs to be translated(without non-free translations)' % len(trans))
	create_translations_file(outfilefull, trans)

	nonfree_translations = None
	try:
		nonfree_translations = os.listdir('nonfree_translations')
	except OSError:
		# silently ignore...
		pass

	if nonfree_translations:
		avail_trans = []
		for i in nonfree_translations:
			avail_trans += vgui_translation_parse(os.path.join('nonfree_translations', i))

		print('%d translated strings available from original GameUI/HL translations' % len(avail_trans))
		trans = [t for t in trans if not t in avail_trans]
		print('%d strings needs to be translated' % len(trans))
		create_translations_file(outfile, trans)
	else:
		def check_nonfree(s):
			return (s.startswith('GameUI_') or s.startswith('Valve_') or s.startswith('Cstrike_'))

		trans = [t for t in trans if not check_nonfree(t)]
		print('%d strings needs to be translated' % len(trans))
		create_translations_file(outfile, trans)

if __name__ == '__main__':
	main()
