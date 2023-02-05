$SaveFolder = $args[0]
if ($args.count -lt 1){
	Write-Host "Missing save folder parameter"
}

if(Test-Path $SaveFolder){
	Write-Host "Delete pix folder in GameEngine\vendor and rerun script to reinstall pix"
	exit 0
}

New-Item -ItemType Directory -Force -Path $SaveFolder

$DownloadURL = "https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001"
$ZIPPath = Join-Path -Path $SaveFolder -ChildPath "pix.zip"
Invoke-WebRequest -UseBasicParsing -Uri $DownloadURL -OutFile $ZIPPath
Expand-Archive -Path $ZIPPath -DestinationPath $SaveFolder -Force
Remove-Item -Path $ZIPPath
Write-Host "Sucessfully downloaded Socrates"
exit 0