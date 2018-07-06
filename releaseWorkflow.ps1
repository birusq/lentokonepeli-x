$version_string = Read-Host -Prompt "Input version string without 'v'-prefix, eg. '0.8-alpha'"

Remove-Item "E:\Documents\Visual Studio 2017\Projects\lentokonepeli\Release\res\*" -Recurse
Copy-Item "E:\Google Drive\Lentokonepeli_resources\*" -Destination "E:\Documents\Visual Studio 2017\Projects\lentokonepeli\Release\res" -Recurse

set-alias 7z "$env:ProgramFiles\7-Zip\7z.exe"
$archiveTarget = "E:\Documents\Visual Studio 2017\Projects\lentokonepeli\releases\v" + $version_string + "-pc-lkp.zip"
7z a -tzip $archiveTarget @$PSScriptRoot\7zListfile.txt

Copy-Item $archiveTarget -Destination "E:\Documents\EeroLWebpage\eerolsite\public\downloads\lentokonepeli-x"
Copy-Item $archiveTarget -Destination "E:\Google Drive\Lentokonepeli_releases"

git add --all
git commit
git tag "v$version_string"
git push origin master