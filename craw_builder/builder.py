import subprocess, os, sys, urllib2, platform, shutil, time, winsound, _winreg

def is_64_bit():
	return platform.architecture()[0] == '64bit'

def download(url):
	try:
		print 'Downloading %s' % url
		request = urllib2.Request(url)
		response = urllib2.urlopen(request)
		data = response.read()
		return data
	except:
		return None
		
def download_file(url, download_path):
	data = download(url)
	if data == None:
		print 'Failed to download %s' % url
		sys.exit(1)
	file = open(download_path, 'w+b')
	file.write(data)
	file.close()

def has_command(command):
	return len(os.popen(command).read()) > 0

def has_svn():
	return has_command('svn --help')
	
def get_sliksvn_path():
	try:
		key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'SOFTWARE\\SlikSvn\\Install')
		value, type = _winreg.QueryValueEx(key, 'Location')
		return os.path.join(value, 'svn.exe')
	except:
		return None
	
def has_scons():
	return has_command('%s --help' % scons_path)
	
def has_bzip2():
	try:
		subprocess.call('bzip2\\bzip2.exe')
		return True
	except:
		return False
		
def has_bjam():
	return has_command('boost\\bjam.exe --help')
	
def string_filter(filter_string, lines):
	return filter(lambda line: line.find(filter_string) != -1, lines)
	
def make_directory(directory):
	try:
		os.mkdir(directory)
	except:
		pass
		
def copy_binary(source):
	try:
		target_directory = 'binaries'
		make_directory(target_directory)
		destination = os.path.join(target_directory, os.path.basename(source))
		shutil.copy(source, destination)
	except:
		pass
	
def setup_svn():
	global svn_path
	if has_svn():
		svn_path = 'svn'
		
	svn_path = get_sliksvn_path()
	if svn_path != None:
		return
		
	print 'It appears that you have no command line SVN client installed. This setup is going to download the most recent version of SlikSVN for you.'
	
	url = 'http://www.sliksvn.com/pub/'
	data = download(url)
	if data == False:
		print 'Failed to download the release list from %s' % url
		sys.exit(1)
		
	lines = data.split('\n')
		
	if is_64_bit():
		filter_string = 'x64.msi'
	else:
		filter_string = 'win32.msi'
		
	lines = string_filter(filter_string, lines)
	if len(lines) == 0:
		print 'Received invalid data from %s' % url
		sys.exit(1)
		
	line = lines[-1]
	tokens = line.split('"')
	if len(tokens) != 3:
		print 'Unable to tokenise data: %s' % line
		sys.exit(1)
		
	release = tokens[1]
	
	url = '%s%s' % (url, release)
	print 'This download is about 5 MiB in size, please be patient.'
	target_directory = 'svn'
	make_directory(target_directory)
	path = os.path.join(target_directory, release)
	download_file(url, path)
	print 'Successfully downloaded the SVN client setup to %s, running it' % path
	os.system(path)
	print 'Setup has finished'
	
	svn_path = get_sliksvn_path()
	if svn_path == None:
		print 'The setup must have failed. I am unable to detect the SlikSvn installation.'
		sys.exit(1)
	
def get_site_packages():
	target = 'site-packages'
	for path in sys.path:
		if len(path) > len(target) and path[- len(target) : ] == target:
			return path
	return None
	
def setup_nil():
	print 'Setting up nil for Python'
	target_directory = get_site_packages()
	if target_directory == None:
		print 'Unable to determine Python site-packages directory'
		sys.exit(1)
	google_checkout('python-nil', os.path.join(target_directory, 'nil'))
	
def setup_scons():
	global scons_path		
	scons_path = get_scons_path()
	if scons_path == False:
		print 'Unable to get the Python library directory to determine the SCons path'
		sys.exit(1)
		
	if has_scons():
		return
		
	print 'Setting up SCons'
	target_directory = 'scons'
	url = 'http://surfnet.dl.sourceforge.net/sourceforge/scons/scons-1.2.0.d20090223.win32.exe'
	path = os.path.join(target_directory, os.path.basename(url))
	make_directory(target_directory)
	download_file(url, path)
	print 'Successfully downloaded the SCons setup to %s, running it' % path
	os.system(path)
	print 'Setup has finished'
	
def setup_bzip2():
	if has_bzip2():
		return
		
	original_url = 'http://surfnet.dl.sourceforge.net/sourceforge/gnuwin32/bzip2-1.0.5-bin.zip'
	print 'Setting up bzip2'
	print 'The original bzip2 binaries are available at %s' % original_url
	print 'I cannot extract zip files so I am downloading the extracted version instead'
	target_directory = 'bzip2'
	make_directory(target_directory)
	source = prebuilt_binary_source
	targets = ['bzip2.dll', 'bzip2.exe']
	for target in targets:
		download_file(source + target, os.path.join(target_directory, target))
		
def setup_boost():
	print 'Setting up boost'
	target_directory = 'boost'
	make_directory(target_directory)
	
	if not has_bjam():
		print 'Getting the bjam binary (originally available at http://surfnet.dl.sourceforge.net/sourceforge/boost/boost-jam-3.1.17-1-ntx86.zip)'
	
		source = prebuilt_binary_source
		targets = ['bjam.exe']
		for target in targets:
			download_file(source + target, os.path.join(target_directory, target))
			
	svn_checkout('http://svn.boost.org/svn/boost/trunk/', 'boost')
	os.chdir(target_directory)
	cpus = int(os.environ.get('NUMBER_OF_PROCESSORS', 2))
	print 'Number of CPUs detected: %d' % cpus
	os.system('bjam toolset=%s variant=release threading=multi runtime-link=static link=static stage -j %d' % (toolset, cpus))
	os.chdir('..')
	
def setup_ail():
	target = 'ail'
	print 'Setting up %s' % target
	google_checkout(target, target)
	os.chdir(target)
	os.system('%s boost=..\\..\\boost' % scons_path)
	os.chdir('..')
	
def setup_syringe():
	target = 'syringe'
	print 'Setting up %s' % target
	google_checkout(target, target)
	os.chdir(target)
	os.system('%s boost=..\\..\\boost ail=..\\..\\ail ail_lib=..\\..\\ail\\build\\ail.lib' % scons_path)
	os.chdir('..')
	copy_binary('%s\\build\\%s.exe' % (target, target))
	
def setup_craw():
	target = 'craw'
	print 'Setting up %s' % target
	google_checkout('craw-module', target)
	os.chdir(target)
	os.system('%s boost=..\\..\\boost boost_lib=..\\..\\boost\\stage\\lib ail=..\\..\\ail ail_lib=..\\ail\\build\\ail.lib' % scons_path)
	os.chdir('..')
	copy_binary('%s\\build\\%s.dll' % (target, target))
	
def svn_checkout(url, local):
	command = '%s checkout %s %s' % (svn_path, url, local)
	print 'Executing %s' % command
	subprocess.call(command)

def google_checkout(project, local):
	svn_checkout('http://%s.googlecode.com/svn/trunk/' % project, local)

def get_scons_path():
	target = 'lib'
	for path in sys.path:
		if os.path.basename(path) == target:
			return os.path.join(path[0 : - len(target)], 'Scripts', 'scons.bat')
	return None
	
prebuilt_binary_source = 'http://siyobik.info/software/'
toolset = 'msvc-9.0'

start = time.time()

setup_svn()
setup_nil()
setup_scons()
setup_boost()
setup_ail()
setup_syringe()
setup_craw()

seconds_per_minute = 60
seconds_passed = int(time.time() - start)
print 'Setup finished after %d:%02d minute(s)' % (seconds_passed / seconds_per_minute, seconds_passed % seconds_per_minute)
winsound.MessageBeep()