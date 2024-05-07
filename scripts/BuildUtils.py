import os
import subprocess
import multiprocessing
import concurrent.futures
from enum import Enum
import math
import shutil
import Config

from scripts.Utils.Utils import *
from scripts.Utils.Windows import *

# ------------------------------------------- Sources ------------------------------------------- #

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
		if(os.path.exists(self.objFile) and self.treeFile != "" and os.path.exists(self.treeFile)):
			lastModifyTime = os.path.getmtime(self.objFile)
			file = open(self.treeFile)
			s = file.read()
			lines = s.split(".d:")[1].split("\n")
			#print(self.workingDir)
			#print()
			needsBuild = False
			for line in lines:
				# sanitize path
				if(line.endswith("\\")):
					line = line[0:-1]
				line = line.strip()

				# check the path is not empty
				if(line == ""):
					continue

				# get all the tokens in the line
				tokens = line.split(" ")
				curr = ""
				lastSucces = False
				for t in tokens:
					lastSucces = False
					if curr != "":
						curr += " "
					curr += t
					path = curr
					if(not os.path.isabs(path)):
						path = os.path.join(self.workingDir, path)
					#print(f"{curr} : {t} \n{path} : {os.path.isfile(path)}")
					#print()
					if os.path.isfile(path):
						lastTime = os.path.getmtime(path) # get last time file was modified
						curr = "" # reset path
						lastSucces = True
						# check if this dependancy was modified after the last time the obj was built
						if lastTime > lastModifyTime:
							needsBuild = True
							break
				if lastSucces == False: # hit the end of the line with unresolved path meaning a file was deleted
					needsBuild = True
				if needsBuild == True:
					break
			self._needsBuild = needsBuild
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
				#"--mrelax-relocations",
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
				"-gno-column-info",
				"-ffunction-sections",

				# debug
				#"-fstandalone-debug",
				"-debug-info-kind=standalone",
				"-gcodeview",

				# limits
				"-ferror-limit", "10",

				# visual studio
				"-fdiagnostics-format", "msvc",
				"-fms-volatile",
				"-fms-extensions",
				"-fms-compatibility",
				f"-fms-compatibility-version={GetFMSVersion()}",

				# exceptions
				"-fcxx-exceptions",
				"-fexceptions",
				"-fexternc-nounwind",

				# suppressions
				"-Wno-unused-comparison",

				# dependancy file
				"-dependency-file", self.treeFile,
				"-MT", self.treeFile,

			])
			for i in self.includeDirs:
				args.extend(["-I", i])
			for i in self.sysIncludeDirs:
				args.extend(["-internal-isystem", i])
			for d in self.defines:
				args.extend(["-D", d])
			result = subprocess.run(args, cwd=self.workingDir, capture_output=True, text=True)
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
	numFailed = 0
	total = len(objects)
	for f in futures:
		if(f.result()==0):
			numSuccess+=1
		elif(f.result()>0):
			numFailed+=1

	print(f"finished building sources : {numSuccess} succeeded : {numFailed} failed : {total - (numSuccess+numFailed)} up-to-date")
	if(numSuccess == 0 and numFailed == 0):
		return -1 # no work done
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






# ------------------------------------------- Resources ------------------------------------------- #
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
		my_env["PATH"] = f"{GetWindowsKitBin()}/x64:{my_env['PATH']}"
		result = subprocess.run(args, cwd=self.workingDir, shell=True, env=my_env, capture_output=True, text=True)
		log = f"|------------- Building file : {name} -------------|\n"
		log += str(result.stderr)
		#log += str(result.stdout)
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

	print(f"finished building resources : {numSuccess} succeeded : {numFailed} failed : {total - (numSuccess+numFailed)} up-to-date")
	if(numSuccess == 0 and numFailed == 0):
		return -1 # no work done
	return numFailed

def BuildResources(resources, dependancys, projDir, intDir):
	projName = os.path.basename(projDir)
	files = ResolveFiles(resources, projDir)
	if(len(files) == 0):
		return -1
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






# ------------------------------------------- Linker ------------------------------------------- #
class BuildType(Enum):
	EXECUTABLE = 0
	STATICLIBRARY = 1

def CollectIntFolders(dependancys):
	intDirs = []
	for d in dependancys:
		proj = GetProject(d)["module"].GetProject()
		intDirs.append(proj.intDir)
		intDirs.extend(CollectIntFolders(proj.dependancys))
	return intDirs

def LinkObjects(intDir, dependancys, links, projDir, outputFile, buildType, needsBuild):
	if(not needsBuild):
		if(not os.path.isfile(outputFile)):
			needsBuild = True
		else:
			lastBuildTime = os.path.getmtime(outputFile)
			for i in range(len(links)):
				lib = ""
				if(not links[i].endswith(".lib")):
					lib = GetProject(links[i])["module"].GetProject().GetOutput()
					if(not os.path.isabs(lib)):
						lib = os.path.join(projDir, lib)
				else:
					lib = links[i]
					if(not os.path.isabs(lib)):
						lib = os.path.join(projDir, lib)
					if(not os.path.isfile(lib)): # file must be found in the path
						lib = "" # TODO : find file in path (safe to ignore for now)

				buildTime = os.path.getmtime(lib)
				if(buildTime > lastBuildTime):
					needsBuild = True


	if(needsBuild):
		projName = os.path.basename(projDir)
		for i in range(len(links)):
			if(not links[i].endswith(".lib")):
				links[i] = GetProject(links[i])["module"].GetProject().GetOutput()
		args = []
		args.extend([Config.compiler])
		args.extend(["--driver-mode=cl"])
		args.extend(["-target", GetTarget()])

		if(buildType == BuildType.STATICLIBRARY):
			args.extend(["-fuse-ld=llvm-lib"])
		elif(buildType == BuildType.EXECUTABLE):
			args.extend(["-Zi"])
			args.extend(["-Wl,--subsystem,console"])
			args.extend(ResolveFiles([f"{intDir}/**.res"], intDir))
			intDirs = CollectIntFolders(dependancys)
			for intd in intDirs:
				args.extend(ResolveFiles([f"{intd}/**.res"], intd))

		args.extend(["-o", outputFile])
		args.extend(ResolveFiles([f"{intDir}/**.obj"], intDir))
		args.extend(links)
		result = subprocess.run(args, cwd=projDir, capture_output=True, text=True)
		log = f"|------------- linking project : {projName} -------------|\n"
		log += str(result.stderr)
		log += str(result.stdout)
		log += f"linking {projName} completed with error code {result.returncode}\n"
		print(log)
		return result.returncode
	return -1






# ------------------------------------------- Projects ------------------------------------------- #
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
		self.dlls = []
		self.dependancys = []
		self.buildType = BuildType.EXECUTABLE
		self.intDir = ""
		self.binDir = ""
		self.buildName = ""
		self.genReflection = False

	def GetOutput(self):
		ext = [".exe", ".lib"][self.buildType.value]
		buildName = self.buildName
		if(buildName == ""):
			buildName = os.path.basename(self.projectDirectory)
		return self.binDir + "/" + buildName + ext

	def Build(self):
		projName = os.path.basename(self.projectDirectory)
		idir = self.intDir
		if(not os.path.exists(idir)):
			os.makedirs(idir)

		bdir = self.binDir
		if(not os.path.exists(bdir)):
			os.makedirs(bdir)

		# build resource files
		resourceBuildStatus = BuildResources(self.resources, self.dependancys, self.projectDirectory, idir)
		if(resourceBuildStatus > 0): # faild to build resources
			return 1

		# build pch
		# TODO

		# build source files
		sourceBuildStatus = BuildSources(self.sources, self.projectDirectory, idir, self.includes, self.sysIncludes, self.defines)
		if(sourceBuildStatus > 0): # faild to build sources
			return 1

		# create and build reflection
		reflectionBuildStatus = -1
		if(self.genReflection and sourceBuildStatus != -1):
			args = []
			reflectorProject = GetProject("Socrates")["module"].GetProject()
			args.extend([reflectorProject.GetOutput(), self.projectDirectory])
			result = subprocess.run(args, cwd=self.projectDirectory, capture_output=True, text=True)
			log = f"|------------- Generating reflection : {projName} -------------|\n"
			log += str(result.stderr)
			log += str(result.stdout)
			log += f"|------------- Finished generating reflection : {projName} -------------|\n"
			print(log)
			reflectionBuildStatus = BuildSources(["generated/generated.cpp"], self.projectDirectory, idir, self.includes, self.sysIncludes, self.defines)
			if(reflectionBuildStatus != 0):
				return 1

		# link
		ext = [".exe", ".lib"][self.buildType.value]
		needsBuild = resourceBuildStatus != -1 or sourceBuildStatus != -1 or reflectionBuildStatus != -1
		linkStatus = LinkObjects(idir, self.dependancys, self.links, self.projectDirectory, self.GetOutput(), self.buildType, needsBuild)
		if(linkStatus > 0):
			return 1

		# copy dlls
		for dll in self.dlls:
			dllName = os.path.basename(dll)
			shutil.copy(dll, self.binDir + "/" + dllName)

		return 0
