# How to build MKW3DS/CTGP7 Launcher:


# Windows
1. Get [Devkitpro installer](https://github.com/devkitPro/installer/releases)
2. Get [Bannertool](https://github.com/Steveice10/bannertool/releases/tag/1.2.0)
3. Get [Makerom](https://github.com/3DSGuy/Project_CTR/releases/tag/makerom-v0.17)
4. First install Devkitpro using Devkitpro installer you got earlier
5. Uncheck everything except DevkitARM
6. Install and wait
7. When the installation ends start MSYS2 on Devkitpro start menu
8. Follow this [Small Guide](https://gbatemp.net/threads/help-falied-to-compile-fbi-with-devkitpro-3-0-3.514546/#post-8211852)
9. After this run `pacman -S 3ds-curl` and `pacman -S 3ds-libpng` and `pacman -S picasso` and finally `pacman -S zip` (optional)
10. Now get **Bannertool** and **Makerom** files and put them on `C:\devkitPro\msys\usr\bin\` (select Win_x86_64, Win 32 bit cannot be used with Makerom so don't even try.)
11. Now run `mkdir "MKW3DS_Build"` then `git clone https://github.com/Midou36O/MKW3DS_Launcher.git`
12. Run `cd MKW3DS_Build` then `cd MKW3DS_Launcher`
13. Run make and wait
14. when build ends check the output folder and go to 3ds-arm, you should find the .cia and a 3ds folder where contains the .3dsx to launch MKW3DS from homebrew.

# Linux
 **Note:** This has been tested on WSL 1.0 on Windows 10 LTSC running Ubuntu 20.04 but it still should work as expected with minor differences
 
 1. Install [Devkitpro pacman](https://devkitpro.org/wiki/devkitPro_pacman) following your current distro (currently Arch, Fedora, Debian (includes Ubuntu), Gentoo), if you don't know what to choose **Get Debian/Ubuntu**
    - Assuming you got Debian/Ubuntu get [the binary here](https://github.com/devkitPro/pacman/releases/tag/v1.0.2) (generally you'd want devkitpro-pacman.amd64.deb unless it's a raspberry pi or a mobile device.)
    - Make sure make is installed (debian/ubuntu should tell you that it needs them to install devkitpro anyway)
    - If you are either a arch user or gentoo or fedora just follow the guide in step 1 and look for your distro
 2. After installing devkitpro on your distro logout and log back in.
 3. Get [Bannertool](https://github.com/Steveice10/bannertool/releases/tag/1.2.0) ( choose bannertool/linux-x86_64/ if it's 64 bit (you don't have choice anyway)
 4. Get [Makerom](https://github.com/3DSGuy/Project_CTR/releases/tag/makerom-v0.17) (download the file called ubuntu, it should work on other distros too)
 5. Run these commands: `sudo cp ~/Downloads/bannertool/linux-x86_64/bannertool /usr/bin/ && sudo cp ~/Downloads/makerom-v0.17-ubuntu_x86_64/makerom /usr/bin`
    - If you downloaded these files somewhere else then change the path to the folder you downloaded them
 6. Run `sudo dkp-pacman -S 3ds-dev` `sudo dkp-pacman -S 3ds-curl` `sudo dkp-pacman -S 3ds-libpng`
    - If you're on arch remove `dkp-` as devkitpro uses pacman itself to install its packages
 7. Now run `mkdir MKW3DS_Build` then `git clone https://github.com/Midou36O/MKW3DS_Launcher.git`
 8. Run `cd MKW3DS_Build` then `cd MKW3DS_Launcher`
 9. Run make and wait
 10. If it whines about zip not being found ingore that
 
     ![image](https://user-images.githubusercontent.com/45198486/125319993-3069a800-e333-11eb-8633-8bacc51ec059.png)

 11. check the ./output/3ds-arm/ and you should get all the build files ready to be installed.
