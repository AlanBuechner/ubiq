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

def UnzipAndDelete(zip, folder):
	with zipfile.ZipFile(zip, 'r') as zip_ref:
		zip_ref.extractall(folder)
	os.remove(zip)

os.chdir("../")

dxcInstallLocation = "vendor/dxc"
pixInstallLocation = "vendor/pix"
compilerInstallLocation = "vendor/Compiler"
premakeInstallLocation = "vendor/premake"

# install dxc
if not DoesFolderExist(dxcInstallLocation):
	print("installing DXC")
	os.makedirs(dxcInstallLocation)
	zipLocation = dxcInstallLocation + "/dxc.zip"

	jsonURL = "https://api.github.com/repos/microsoft/DirectXShaderCompiler/releases/latest"
	latestURL = ""
	try:
		with urllib.request.urlopen(jsonURL) as url:
			data = json.load(url)
			pattern = re.compile("dxc.*.zip")
			for asset in data["assets"]:
				if pattern.match(asset["name"]):
					latestURL = asset["browser_download_url"]
					break
		urllib.request.urlretrieve(latestURL, zipLocation)
		UnzipAndDelete(zipLocation, dxcInstallLocation)
	except:
		print("Failed to install DXC")
		os.rmdir(dxcInstallLocation)
	else:
		print("Finished installing DXC")
else:
	print("DXC already installed. To reinstall DXC delete the folder \"{dxc}\"".format(dxc=dxcInstallLocation))
	

print()


# install pix
if not DoesFolderExist(pixInstallLocation):
	print("Installing PIX")
	os.makedirs(pixInstallLocation)
	zipLocation = pixInstallLocation + "/pix.zip"

	try:
		urllib.request.urlretrieve("https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001", zipLocation)
		UnzipAndDelete(zipLocation, pixInstallLocation)
	except Exception as e:
		print(e)
		print("Failed to install PIX")
		os.rmdir(pixInstallLocation)
	else:
		print("Finished installing PIX")
else:
	print("PIX already installed. To reinstall PIX delete the folder \"{pix}\"".format(pix=pixInstallLocation))


print()


# install compiler
if not DoesFolderExist(compilerInstallLocation):
	print("Installing Compiler")
	os.makedirs(compilerInstallLocation)
	zipLocation = compilerInstallLocation + "/Compiler.zip"

	try:
		urllib.request.urlretrieve("https://github.com/AlanBuechner/Socrates/releases/download/v0.0.1-alpha/Socrates.zip", zipLocation)
		UnzipAndDelete(zipLocation, compilerInstallLocation)
	except Exception as e:
		print(e)
		print("Failed to install the compiler")
		os.rmdir(compilerInstallLocation)
	else:
		print("Finished installing the compiler")
else:
	print("Compiler already installed. To reinstall the Compiler delete the folder \"{comp}\"".format(comp=compilerInstallLocation))


print()


# install premake
premakeEXE = premakeInstallLocation+"/premake5.exe"
if not DoesFileExist(premakeEXE):
	print("Installing Premake")
	os.makedirs(premakeInstallLocation)
	zipLocation = premakeInstallLocation + "/premake.zip"

	try:
		urllib.request.urlretrieve("https://github.com/premake/premake-core/releases/download/v5.0.0-beta1/premake-5.0.0-beta1-windows.zip", zipLocation)
		with zipfile.ZipFile(zipLocation, 'r') as zip_ref:
			zip_ref.extract("premake5.exe", premakeInstallLocation)
		os.remove(zipLocation)
	except Exception as e:
		print(e)
		print("Failed to install Premake")
	else:
		print("Finished installing Premake")
else:
	print("Premake already installed. To reinstall Premake delete \"{pre}\"".format(pre=premakeEXE))


print()

