AdeptivePantyhose

What it Does
AdeptivePantyhose is an SKSE plugin that dynamically adjusts your character's body based on the equipped footwear. By utilizing [b]BodySlide[/b] sliders created by Caenarvon, it works with the [b]RaceMenu[/b] interface (or its underlying [code]skee64.dll[/code]) to seamlessly morph your character's body. This eliminates any unsightly seams or clipping, ensuring your character's feet and toes are perfectly fitted to their pantyhose, especially when wearing high heels.
[line]
Requirements
[list]
[*][b]RaceMenu[/b]
[*][b]Cosplaypack[/b]
[/list][line]
Installation
[list=1]
[*]Install the mod using your preferred mod manager, just like any other mod.

[*]Ensure this mod [b]overwrites[/b] "Cosplay Basics - (CBBE 3BA)".

[*]After installation, run [b]BodySlide[/b].

[/list][line]
Upgrading from the [code].esp[/code] version
No special steps are needed. Simply install the new version.
[line]
Uninstallation
[list=1]
[*]First, ensure your character is wearing a pair of high heels.

[*]Save your game and exit.

[*]Uninstall the mod by deleting its files.

[*]Run [b]BodySlide[/b] after uninstallation.

[/list][line]
Choosing Between Versions
[list]
[*][b]Always[/b] choose the [b]release[/b] version for a stable experience.
[*]The debug version is only intended for troubleshooting. It generates a significant number of log files, which may impact performance and disk space. Only use it if you are helping with bug reports.
[/list][line]
How to Find the Log File
The log file is located at [code]C:\Users\Your User Name\Documents\My Games\Skyrim\SKSE\AdeptivePantyhose.log[/code].
For a quicker way, press [b]Win + R[/b] and paste the following command:
[code]explorer "%USERPROFILE%\Documents\My Games\Skyrim\SKSE\AdeptivePantyhose.log"[/code]
[line]
Who is Affected by the Mod?
This mod affects [b]female playable characters，both PC and NPCs[/b].
[line]
What Qualifies as a High Heel?
The mod determines what is considered a high heel based on a definition file.
[list]
[*]You can find the default definition at [code]ModOrganizer2\mods\AdeptivePantyhose\SKSE\AdeptivePantyhose\DefineHighHeel.json[/code].
[*]By default, items with the [code]SLA_KillerHeels[/code] keyword or a specific range of FormIDs are considered high heels.
[*]You can easily expand this definition by adding more items to the [code].json[/code] file.
[/list][line]
Special Thanks
[list]
[*][b]Caenarvon[/b], for his original mod and for the BodySlide slider implementation.
[*][b]Mrowrpurr[/b], for their video tutorials that guided my modding journey.
[*][b]The testers[/b], for their invaluable help in making this mod a reality.
[/list]
