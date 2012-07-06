# SConstruct
# Crow construction file
# Author: Joe Riedel

import os

project_name = 'Abducted'
build_switches_override = None
variant_override = '../'

SConscript('./Radiance/SConscript', ['build_switches_override', 'variant_override'])

Import('radvars')
(build, source) = radvars

variant_dir = build.variantDir(project_name)
exe_type = 'EXE'
output = SConscript(
	'Source/Game/SConstruct', 
	variant_dir='Bin/Intermediate/' + project_name + build.targetDir(), 
	duplicate=0,
	exports=['variant_dir', 'exe_type', 'project_name']
)

if build.win() and build.tools() and (not build.switches.no_com()) : # build .com variant on windows for command line stuff.
	variant_dir = build.variantDir(project_name + 'Com')
	exe_type = 'COM'
	SConscript(
		'Source/Game/SConstruct', 
		variant_dir='Bin/Intermediate/' + project_name + 'Com' + build.targetDir(), 
		duplicate=0,
		exports=['variant_dir', 'exe_type', 'project_name']
	)
