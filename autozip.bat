SET path=%CD%\addon\*
SET output=%CD%\release\Add-Ons\System_DiscordBL.zip
7za a -y -tzip  %output% %path% -mx5