import os
import urllib.request
import json
import re
import zipfile
import contextlib

@contextlib.contextmanager
def pushd(new_dir):
	previous_dir = os.getcwd()
	os.chdir(new_dir)
	try:
		yield
	finally:
		os.chdir(previous_dir)

def DoesFolderExist(relPath):
	return os.path.exists(os.path.join(os.getcwd(), relPath))

def DoesFileExist(relPath):
	return os.path.exists(os.path.join(os.getcwd(), relPath))

def UnzipAndDelete(zip, folder, filelist=None):
	with zipfile.ZipFile(zip, 'r') as zip_ref:
		if filelist == None:
			zip_ref.extractall(folder)
		else:
			for f in filelist:
				zip_ref.extract(f, folder)
	os.remove(zip)

def DownloadZip(name, folder, url, filelist=None, requestFunc=None):
	if not DoesFolderExist(folder):
		print("Installing {name}".format(name=name))
		os.makedirs(folder)
		zipLocation = folder + "/temp.zip"

		try:
			if requestFunc != None:
				url = requestFunc(url)
			urllib.request.urlretrieve(url, zipLocation)
			UnzipAndDelete(zipLocation, folder, filelist)
		except Exception as e:
			print(e)
			print("Failed to install the {name}".format(name=name))
			os.rmdir(folder)
		else:
			print("Finished installing the {name}".format(name=name))
	else:
		print("{name} already installed. To reinstall the {name} delete the folder \"{folder}\"".format(name=name, folder=folder))

os.chdir("../")

# install dxc
def DXC_ResolveURL(url):
	latestURL = ""
	with urllib.request.urlopen(url) as url:
		data = json.load(url)
		pattern = re.compile("dxc.*.zip")
		for asset in data["assets"]:
			if pattern.match(asset["name"]):
				latestURL = asset["browser_download_url"]
				break
	return latestURL
DownloadZip("DXC", "vendor/dxc", "https://api.github.com/repos/microsoft/DirectXShaderCompiler/releases/latest", None, DXC_ResolveURL)

# install pix
DownloadZip("PIX", "vendor/pix", "https://www.nuget.org/api/v2/package/WinPixEventRuntime/")

# install Agility
DownloadZip("Agility", "vendor/Agility", "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.614.0")

# install compiler
DownloadZip("Compiler", "vendor/Compiler", "https://github.com/AlanBuechner/Socrates/releases/download/v0.0.1-alpha/Socrates.zip")

# install premake
DownloadZip("Premake", "vendor/premake", "https://github.com/premake/premake-core/releases/download/v5.0.0-beta1/premake-5.0.0-beta1-windows.zip", ["premake5.exe"])

# install tracy profiler
DownloadZip("Tracy Profiler", "tools/TracyProfiler", "https://github.com/wolfpld/tracy/releases/download/v0.12.0/windows-0.12.0.zip")
