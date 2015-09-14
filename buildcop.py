#!/usr/bin/python3

from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import os
import shlex # used only for splitting command arguments into array
import stat
import subprocess
import sys
import time

BS_SUCCESS = 0
BS_UNKNOWN = -1
BS_UNAVAILABLE = -2
BS_FAILED = 1

def error(*mes):
	print("Error:", *mes, file=sys.stderr)

def log(*mes):
	print("Log:", *mes)

build_thread_pool = ThreadPoolExecutor(max_workers=max_thread_count)
test_thread_pool = ThreadPoolExecutor(max_workers=max_thread_count)

class Project:
	def __init__(self, proj_dir):
		log("Attempting to instantiate project", str(proj_dir))
		self.proj_path = proj_dir # i believe Path objects are immutable (though not the files they point to)
		os.chdir(str(self.proj_path))
		
		config_file = self.proj_path / ".buildcop"
		with config_file.open() as _file:
			line = _file.readline()
			while line != "":
				if line.startswith("#"):
					continue
				
				key = line[:line.find(":")]
				value = line[line.find(":") + 1:].strip()
				if key == "build":
					self.build_command = Path(value)
				elif key == "test":
					self.test_command = path(value)
				elif key == "args":
					self.args = shlex.split(value)
				elif isInt(key):
					key = int(key)
					if key >= 0 and key < 256:
						self.exit_codes[int(key)] = value
				line = _file.readline()
		log("Loaded config file, build:", str(self.build_command), "test:", str(self.test_command))
		
		if os.name == "posix":
			self.output_path = Path("/tmp/buildcop") / self.getName()
		else:
			error("Temporary build and test output storage not implemented for non-POSIX systems")
			exit(1)
		
		if self.build_command.name == "":
			self.build_command = firstOrDefault(
			    ( build 
			      for build in self.proj_path.iterdir() 
			      if build.is_file() and build.name.startswith("build") and isExecutable(build) ), 
			      default=self.build_command)
			
			# only searching the top-level of the path directory
		elif not self.build_command.is_file():
			# search the current directory
			result = findInPath(self.build_command, Path("."))
			if result is not None:
				self.build_command = result
			else:
				# search path environment variable
				split_path = None
				if os.name == "posix":
					split_path = stringListToPaths(os.environ["PATH"].split(":"))
				else:
					split_path = stringListToPaths(os.environ["PATH"].split(";"))
				
				self.build_command = firstOrDefault(( findInPath(self.build_command, path_part) for path_part in split_path ),
				    default=Path(""))
		
		if self.build_command.name == "":
			build_state = BS_UNAVAILABLE
		else:
			build_state = BS_UNKNOWN
		
		if self.test_command.name == "":
			test_state = BS_UNAVAILABLE
		else:
			test_state = BS_UNKNOWN
		
		log("Instantiated project", self.getName(), "at", str(self.proj_path))
	
	def __del__(self):
		if build_output_file is not None:
			build_output_file.close()
		
		if test_output_file is not None:
			test_output_file.close()
	
	def getName(self):
		return self.proj_path.name
	
	def runBuild(self):
		# TODO: consider storing old build/test outputs
		self.build_output_file = (self.output_path / "build" / (time.time() + ".txt")).open()
		self.build_output_file = self.build_output_path.open("w") # automatically truncates/creates the file
		
		build_thread_pool.submit(subprocess.call, str(build_command.absolute()), *self.args, 
		    kwargs={"stdout": self.build_output_file, 
		            "stderr": self.build_output_file})
		build_thread_pool.add_done_callback(self.buildEnded)
	
	def buildEnded(self, future):
		self.build_output_file.close()
		self.build_output_file = self.build_output_path.open() # lower file to read-only privledges
		self.build_state = future.result()
	
	#def getOutput(self):
	#	
	
	build_state = BS_UNAVAILABLE
	test_state = BS_UNAVAILABLE
	
	proj_path = Path()
	output_path = Path()
	
	build_output_file = None
	test_output_file = None
	
	# loaded from config files
	args = []
	exit_codes = [""] * 256 # don't care if these are all the same reference
	build_command = Path("")
	test_command = Path("")

def isInt(s):
	try:
		int(s)
		return True
	except ValueError:
		return False

def firstOrDefault(iterator, default=None):
	for thing in iterator:
		return thing
	return default

def isExecutable(file_path):
	if not file_path.exists():
		error("Impossible condition (checking if nonexistant file is executable)")
		return False
	
	file_stat = file_path.stat()
	
	# damn they like these variable names short
	if file_stat.st_mode & stat.S_IXOTH: # i(s) (e)x(ecutable) to oth(ers)
		return True
	elif os.getgid() == file_stat.ST_GID and file_stat.st_mode & stat.S_IXGRP: # to gr(ou)p
		return True
	elif os.getuid() == file_stat.ST_UID and file_stat.st_mode & stat.S_IXUSR: # to us(e)r
		return True
	else:
		return False

def firstPathIsRelative(child, parent):
	try:
		child.relative_to(parent)
		return True
	except ValueError:
		return False

def findInPath(search, _path):
	result = _path / search
	if result.exists:
		return result
	else:
		return None

def stringListToPaths(slist):
	return [ Path(_path) for _path in slist ]

def getProjectList(file_path, proj_list=[]):
	if file_path.is_dir():
		for conf_file in file_path.glob("**/.buildcop"):
			try:
				proj_list.append(Project(conf_file.parent))
			except FileNotFoundError:
				error("Attempted to instantiate a project using a path that didn't contain a project file at", str(file_path))
	
	return proj_list # just cuz it makes sense


search_path = Path(sys.argv[1])
print("Search path: \"%s\"" % str(search_path))
if search_path.exists():
	print("Search Path found")
else:
	error("Path not found")
	exit()

if len(sys.argv) > 2 and isInt(sys.argv[2]):
	max_thread_count = int(sys.argv[2])

proj_list = getProjectList(search_path)
for proj in proj_list:
	proj.runBuild()