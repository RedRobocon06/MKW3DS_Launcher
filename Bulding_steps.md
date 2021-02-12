# How to build MKW3DS/CTGP7 Launcher:


# Windows
1. Get [Devkitpro installer](https://github.com/devkitPro/installer/releases)
2. Get [Bannertool](https://github.com/Steveice10/bannertool/releases/tag/1.2.0)
3. Get [Makerom](https://github.com/3DSGuy/Project_CTR/releases/tag/makerom-v0.17)
4. first install Devkitpro using Devkitpro installer you got earlier
5. Uncheck everything except DevkitARM
6. Install and wait
7. when the installation ends start MSYS2 on Devkitpro start menu
8. follow this [Small Guide](https://gbatemp.net/threads/help-falied-to-compile-fbi-with-devkitpro-3-0-3.514546/#post-8211852)
9. after this run `pacman -S 3ds-curl` and `pacman -S 3ds-libpng` and `pacman -S picasso` and finally `pacman -S zip` (optional)
10. now get **Bannertool** and **Makerom** files and put them on `C:\devkitPro\msys\usr\bin\` (select Win_x86_64, Win 32 bit cannot be used with Makerom so don't even try.)
11. now run `mkdir "MKW3DS_Build"` then `git clone https://github.com/Midou36O/MKW3DS_Launcher.git`
12. run `cd MKW3DS_Build` then `cd MKW3DS_Launcher`
13. run make and wait
14. when build ends check the output folder and go to 3ds-arm, you should find the .cia and a 3ds folder where contains the .3dsx to launch MKW3DS from homebrew.

# Linux
 **WIP**
