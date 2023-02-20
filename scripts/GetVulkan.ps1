Invoke-WebRequest -Uri "https://sdk.lunarg.com/sdk/download/1.3.239.0/windows/VulkanSDK-1.3.239.0-Installer.exe" -OutFile "VulkanSDK.exe"
$installer = Start-Process -FilePath VulkanSDK.exe -Wait -PassThru -ArgumentList @("/S");
$installer.WaitForExit();
Remove-Item "VulkanSDK.exe"