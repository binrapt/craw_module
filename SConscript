import nil.directory, sys, os

module = 'craw'
boost = ARGUMENTS.get('boost')
boost_lib = ARGUMENTS.get('boost_lib')
ail = ARGUMENTS.get('ail')
ail_lib = ARGUMENTS.get('ail_lib')

include_directories = [boost, ail]

dependencies = include_directories + [boost_lib, ail_lib]

if len(filter(lambda x: x == None, dependencies)) > 0:
	print 'This module requires boost (www.boost.org) and ail (ail.googlecode.com) so you will have to specify the paths in the scons arguments:'
	print 'scons boost=<boost header directory> boost_lib=<boost binary directory> ail=<ail header directory> ail_lib=<path to ail.lib>'
	sys.exit(1)

defines = {
	'_CRT_SECURE_NO_WARNINGS': 1
}

"""
flags = [
	'/O2',
	'/Oi',
	'/GL',
	#'/FD',
	'/MD',
	'/Gy',
	'/W3',
	'/nologo',
	'/c',
	#'/Zi',
	'/EHsc'
]
"""

flags = [
	'/EHsc'
]

relative_source_directory = os.path.join('..', module)

source_files = map(lambda path: os.path.basename(path), nil.directory.get_files_by_extension(relative_source_directory, 'cpp'))

include_directories += [module]

cpus = int(os.environ.get('NUMBER_OF_PROCESSORS', 2))

thread_string = 'thread'
if cpus > 1:
	thread_string += 's'
print 'Compiling project with %d %s' % (cpus, thread_string)

environment = Environment(CPPPATH = include_directories, CCFLAGS = flags, LIBPATH = boost_lib, LIBS = [ail_lib, 'user32.lib', 'kernel32.lib'], CPPDEFINES = defines)
environment.SetOption('num_jobs', cpus)
environment.SharedLibrary(module, source_files)