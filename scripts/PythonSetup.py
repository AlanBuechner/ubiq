import os
import urllib.request
import json
import re
import zipfile

def DoesFileExist(relPath):
	return os.path.exists(os.path.join(os.getcwd(), relPath))

originalPath = os.getcwd()
os.chdir(os.path.join(originalPath, "../vendor/python"))

# set up python311._pth
with open("python311._pth", "w") as file:
	lines = ["python311.zip\n", ".\n" "\n", "# Uncomment to run site.main() automatically\n", "import site\n"]
	file.writelines(lines)

# download and run get pip script
print("downloading pip")
try:
	urllib.request.urlretrieve("https://bootstrap.pypa.io/get-pip.py", "get-pip.py")
	os.system("python.exe get-pip.py")
except Exception as e:
	print(e)
	print("Failed to get pip")
if DoesFileExist("get-pip.py"):
	os.remove("get-pip.py")

os.chdir(originalPath)
