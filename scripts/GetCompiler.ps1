$SaveFolder = $args[0]
if ($args.count -lt 1){
	Write-Host "Missing save folder parameter"
}

if(Test-Path $SaveFolder){
	Write-Host "Delete Compiler folder in vendor and rerun script to reinstall Compiler"
	exit 0
}

New-Item -ItemType Directory -Force -Path $SaveFolder

$DownloadURL = "https://github.com/AlanBuechner/Socrates/releases/download/v0.0.1-alpha/Socrates.zip"
$ZIPPath = Join-Path -Path $SaveFolder -ChildPath "Compiler.zip"
Invoke-WebRequest -UseBasicParsing -Uri $DownloadURL -OutFile $ZIPPath
Expand-Archive -Path $ZIPPath -DestinationPath $SaveFolder -Force
Remove-Item -Path $ZIPPath
Write-Host "Sucessfully downloaded Socrates"
exit 0