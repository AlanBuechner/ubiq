import os
import inspect
import subprocess
import threading
import multiprocessing
import concurrent.futures
from enum import Enum
import math
import Config

def GetBinDir(projName):
	return os.path.join(Config.location, Config.outDir.format(config=Config.configuration, system=Config.system, arc=Config.architecture, projName=projName))

def GetIntDir(projName):
	return os.path.join(Config.location, Config.intDir.format(config=Config.configuration, system=Config.system, arc=Config.architecture, projName=projName))

def GetSysIncludes():
	# TODO : find visual studio headers programaticly
	# TODO : find window headers programaticly
	includes = []
	includes.append(f"{Config.location}/vendor/Compiler/lib/clang/16/include")
	includes.append(f"{Config.location}/vendor/Compiler/include")
	includes.append(f"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.34.31933/include")
	includes.append(f"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.34.31933/atlmfc/include")
	includes.append(f"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/include")
	includes.append(f"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/VS/UnitTest/include")
	includes.append(f"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/ucrt")
	includes.append(f"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/um")
	includes.append(f"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/shared")
	includes.append(f"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/winrt")
	includes.append(f"C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/cppwinrt")
	includes.append(f"C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Include/um")
	return includes

def GetTarget():
	if(Config.architecture == "x86_64"):
		if(Config.system == "windows"):
			return "x86_64-pc-windows-msvc"

def ResolveFiles(filters, d):
	files = []
	for filt in filters:
		name, ext = os.path.splitext(os.path.basename(filt))
		folder = os.path.dirname(filt)
		if (folder != ""):
			folder += "/"
		if(not os.path.isabs(folder)):
			folder = d+"/"+folder

		if(name == "**"):
			for (dirpath, dirnames, filenames) in os.walk(folder):
				files.extend([dirpath + "/" + f for f in filenames if os.path.splitext(os.path.basename(f))[1] == ext])
		else:
			files.append(folder+name+ext)

	return files

def FindProject(projName):
	for proj in Config.projects:
		if(proj.endswith(projName)):
			return proj
	print(f"project {projName} is not defined")
	return ""

class ObjectEnviernment:
	def __init__(self):
		self.sourceFile = ""
		self.objFile = ""
		self.treeFile = ""
		self.workingDir = ""
		self.includeDirs = []
		self.sysIncludeDirs = []
		self.defines = []
		self._needsBuild = True

	def EvaluateDependencies(self):
		if(self.treeFile != "" and os.path.exists(self.treeFile)):
			file = open(self.treeFile)
			lines = file.readlines()
			for line in lines:
				if(line.endswith("\\\n")):
					line = line[0:-3]
			file.close()

	def Build(self):
		self.EvaluateDependencies()

		if(self._needsBuild):
			file = os.path.basename(self.sourceFile)
			args = []
			args.extend([Config.compiler])
			args.extend(["-cc1"])
			args.extend(["-x", "c++"])
			args.extend(["-O2", "-emit-obj"])
			args.extend(["-triple", GetTarget()])
			args.extend([self.sourceFile])
			args.extend(["-o", self.objFile])
			args.extend([f"-std={Config.cppVersion}"])
			args.extend([

				"-mincremental-linker-compatible",
				"--mrelax-relocations",
				"-disable-free",
				"-clear-ast-before-backend",
				"-disable-llvm-verifier",
				"-discard-value-names",
				"-mrelocation-model", "pic",
				"-pic-level", "2",
				"-mframe-pointer=none",
				"-relaxed-aliasing",
				"-fmath-errno",
				"-ffp-contract=on",
				"-fno-rounding-math",
				"-mconstructor-aliases",
				"-funwind-tables=2",
				"-target-cpu", "x86-64",
				"-mllvm",
				"-x86-asm-syntax=intel",
				"-tune-cpu", "generic",
				"-mllvm",
				"-treat-scalable-fixed-error-as-warning",
				"-D_MT",
				"-flto-visibility-public-std",
				"--dependent-lib=libcmt",
				"--dependent-lib=oldnames",
				"-stack-protector", "2",
				"-fdefault-calling-conv=cdecl",
				"-fdiagnostics-format", "msvc",
				"-gno-column-info",
				"-gcodeview",
				"-debug-info-kind=constructor",
				"-ffunction-sections",
				"-fcoverage-compilation-dir=C:/Users/abuechner/source/repos/ubiq/GameEngine",
				"-resource-dir", "C:/Users/abuechner/source/repos/ubiq/vendor/Compiler/lib/clang/16",

				# limits
				"-ferror-limit", "19",

				# fms compatibility
				"-fms-volatile",
				"-fms-extensions",
				"-fms-compatibility",
				"-fms-compatibility-version=19.34.31935",

				# exceptions
				"-fcxx-exceptions",
				"-fexceptions",
				"-fexternc-nounwind",

				# suppressions
				"-Wno-unused-comparison",

			])
			for i in self.includeDirs:
				args.extend(["-I", i])
			for i in self.sysIncludeDirs:
				args.extend(["-internal-isystem", i])
			for d in self.defines:
				args.extend(["-D", d])
			result = subprocess.run(args, cwd=self.workingDir, capture_output=True, text=True)
			#if(result.returncode == 0 and self.treeFile != 0):
				#args.extend(["-dependency-file", self.treeFile, "-MD", "-MT"]) # generate dependancys
				#subprocess.run(args, cwd=self.workingDir, capture_output=True, text=True)
			log = f"|------------- Building file : {file} -------------|\n"
			log += str(result.stderr)
			log += str(result.stdout)
			log += f"building {file} completed with error code {result.returncode}\n"
			print(log)
			return result.returncode
		else:
			return -1

def BuildObjectEnviernments(objects, jobs = 0):
	if(jobs == 0):
		jobs = multiprocessing.cpu_count()
	pool = concurrent.futures.ThreadPoolExecutor(max_workers=jobs)
	futures = []
	for o in objects:
		futures.append(pool.submit(ObjectEnviernment.Build, o))
	
	pool.shutdown(wait=True)

	numSuccess = 0
	numFailed = 0;
	total = len(objects)
	for f in futures:
		if(f.result()==0):
			numSuccess+=1
		elif(f.result()>0):
			numFailed+=1

	print(f"finished building : {numSuccess} succeeded : {numFailed} failed")
	return numFailed

def BuildSources(sources, projDir, intDir, includes, sysIncluds, defines):
	projName = os.path.basename(projDir)
	files = ResolveFiles(sources, projDir)
	buildActions = []

	for file in files:
		name = os.path.splitext(os.path.basename(file))[0]
		objFile = os.path.join(intDir, name) + ".obj"
		treeFile = os.path.join(intDir, name) + ".d"

		env = ObjectEnviernment()
		env.sourceFile = file
		env.objFile = objFile
		env.treeFile = treeFile
		env.workingDir = projDir
		env.includeDirs = includes
		env.sysIncludeDirs = sysIncluds
		env.defines = defines

		buildActions.append(env)

	# build objs
	return BuildObjectEnviernments(buildActions)

class ResourceEnviernment:
	def __init__(self):
		self.resource = ""
		self.outFile = ""
		self.workingDir = ""
		self.includes = []

	def Build(self):
		name = os.path.basename(self.resource)
		args = ["rc.exe", "/R", "/FO", self.outFile]
		for include in self.includes:
			args.extend(["/I", include])
		args.append(self.resource)
		my_env = os.environ.copy()
		my_env["PATH"] = f"C:/Program Files (x86)/Windows Kits/10/bin/10.0.22000.0/x64:{my_env['PATH']}"
		result = subprocess.run(args, cwd=self.workingDir, shell=True, env=my_env, capture_output=True, text=True)
		log = f"|------------- Building file : {name} -------------|\n"
		log += str(result.stderr)
		log += str(result.stdout)
		log += f"building {name} completed with error code {result.returncode}\n"
		print(log)
		return result.returncode


def BuildResourceEnviernments(objects, jobs = 0):
	if(jobs == 0):
		jobs = multiprocessing.cpu_count()
	pool = concurrent.futures.ThreadPoolExecutor(max_workers=jobs)
	futures = []
	for o in objects:
		futures.append(pool.submit(ResourceEnviernment.Build, o))
	
	pool.shutdown(wait=True)

	numSuccess = 0
	numFailed = 0;
	total = len(objects)
	for f in futures:
		if(f.result()==0):
			numSuccess+=1
		elif(f.result()>0):
			numFailed+=1

	print(f"finished building : {numSuccess} succeeded : {numFailed} failed")
	return numFailed

def BuildResources(resources, dependancys, projDir, intDir):
	projName = os.path.basename(projDir)
	files = ResolveFiles(resources, projDir)
	includes = []
	for d in dependancys:
		includes.append(Config.location + "/" + FindProject(d) + "/embeded")
	buildActions = []

	for file in files:
		name = os.path.splitext(os.path.basename(file))[0]
		outFile = os.path.join(intDir, name) + ".res"

		env = ResourceEnviernment()
		env.resource = file
		env.outFile = outFile
		env.workingDir = projDir
		env.includes = includes

		buildActions.append(env)

	return BuildResourceEnviernments(buildActions)


class BuildType(Enum):
	EXECUTABLE = 0
	STATICLIBRARY = 1

def LinkObjects(intDir, links, projDir, outputFile, buildType):
	projName = os.path.basename(projDir)
	for i in range(len(links)):
		if(not links[i].endswith(".lib")):
			links[i] = GetBinDir(links[i]) + "/" + links[i] + ".lib"
	args = []
	args.extend([Config.compiler])
	#args.extend(["--driver-mode=cpp"])
	#args.extend([f"-fuse-ld={Config.linker}"])
	args.extend(["-target", GetTarget()])
	if(buildType == BuildType.STATICLIBRARY):
		args.extend(["-fuse-ld=llvm-lib"])
	elif(buildType == BuildType.EXECUTABLE):
		args.extend(["-Wl,--subsystem,console"])
	args.extend(["-o", outputFile])
	args.extend(ResolveFiles([f"{intDir}/**.obj"], intDir))
	args.extend(ResolveFiles([f"{intDir}/**.res"], intDir))
	args.extend(links)
	result = subprocess.run(args, cwd=projDir, capture_output=True, text=True)
	log = f"|------------- linking project : {projName} -------------|\n"
	log += str(result.stderr)
	log += str(result.stdout)
	log += f"linking {projName} completed with error code {result.returncode}\n"
	print(log)
	return result.returncode

class ProjectEnviernment:
	def __init__(self):
		self.projectDirectory = ""
		self.pchSource = ""
		self.pchHeader = ""
		self.sources = []
		self.resources = []
		self.headers = []
		self.includes = []
		self.sysIncludes = []
		self.defines = []
		self.links = []
		self.dependancys = []
		self.buildType = BuildType.EXECUTABLE

	def Build(self):
		projName = os.path.basename(self.projectDirectory)
		idir = GetIntDir(projName)
		if(not os.path.exists(idir)):
			os.makedirs(idir)

		bdir = GetBinDir(projName)
		if(not os.path.exists(bdir)):
			os.makedirs(bdir)

		# build resource files
		if(BuildResources(self.resources, self.dependancys, self.projectDirectory, idir) != 0):
			return 1

		# build pch
		# TODO

		# build source files
		if(BuildSources(self.sources, self.projectDirectory, idir, self.includes, self.sysIncludes, self.defines) != 0):
			return 1

		# create and build reflection
		#%{wks.location} \"$(ProjectDir)\\\" \"$(IncludePath)\" \"$(IntermediateOutputPath)\\\"
		#args = []
		#args.extend([Config.reflector, projectDirectory])
		#result = subprocess.run(args, cwd=projectDirectory, capture_output=True, text=True)

		# link
		ext = [".exe", ".lib"][self.buildType.value]
		if(LinkObjects(idir, self.links, self.projectDirectory, bdir+"/"+projName+ext, self.buildType) != 0):
			return 1

		return 0
