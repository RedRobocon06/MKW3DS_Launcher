# MKW3DS Launcher (forked from CTGP-7 Launcher)
Launcher app used with MKW3DS to launch the mod. This repository also has a modification of the plugin loader used in the Luma3DS with plugin loading support fork but specific for MK7 title IDs. If you came here only for the plugin loader I recommend checking that repository instead.

# Building
Building MKW3DS Launcher is handled through buildtools. You have to have the following installed:
- [devkitPro with devkitARM](https://sourceforge.net/projects/devkitpro/files/Automated%20Installer/)
- ctrulib (installed automatically with the devkitARM script)
- [citro3d](https://github.com/fincs/citro3d)
- [portlibs](https://github.com/devkitPro/3ds_portlibs)

Once you have installed all the dependencies simply run `make` in the BootNTR directory and if you set it all up correctly it should build.


# Credits
- [@Nanquitas](https://github.com/Nanquitas) : BootNTRSelector, from which this app is forked.
- [@44670](https://github.com/44670) : Initial BootNTR and NTR CFW developer.


# NOTE
The launcher was broken so i decided to restart from scratch, so a lot of features may be similar to CTGP-7, this is also a good thing because the launcher will be easier to update, the problem is that since i didn't improved or followed CTGP-7's launcher code i am a bit confused on how to edit them, but progressivelly i will improve myself, so wait a month or two and i will know how to edit the launcher proprelly