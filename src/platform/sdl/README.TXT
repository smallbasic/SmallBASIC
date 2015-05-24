1. Install fonts (optional)

Download the font files:

http://damieng.com/?s=envy
https://github.com/adobe-fonts/source-sans-pro

Unzip the package and copy the .ttf files to ~/.fonts.

Now run:

 sudo fc-cache -fv

2. Create a desktop file (optional)

cat > ./.local/share/applications/smallbasic.desktop

paste the following contents, the enter CTRL+D

Note: change <install-location> to the installation location

------cut-here---------
[Desktop Entry]
Encoding=UTF-8
Name=SmallBASIC
Comment=SmallBASIC
Exec=<install-location>sbasicg
Icon=<install-location>ic_launcher.png
Terminal=false
Type=Application
Categories=Development
MimeType=application/bas
StartupNotify=true
NoDisplay=false
------cut-here---------

