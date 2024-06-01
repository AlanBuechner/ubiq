@echo off

if not exist ..\vendor\python\ (
	echo "Installing python"
	mkdir ..\vendor\python
	powershell -command "Invoke-WebRequest -UseBasicParsing -Uri 'https://www.python.org/ftp/python/3.11.0/python-3.11.0-embed-amd64.zip' -OutFile '../vendor/python/python.zip'"
	powershell -command "Expand-Archive ..\vendor\python\python.zip ..\vendor\python"
	del ../vendor/python/python.zip
)

"../vendor/python/python.exe" PythonSetup.py
"../vendor/python/python.exe" Setup.py