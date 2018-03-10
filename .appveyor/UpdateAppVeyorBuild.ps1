msbuild /t:loader:ExportVersion /p:Configuration=Release /p:ExportFile=../../.version
$version = Get-Content .version
Update-AppveyorBuild -Version "$version"
