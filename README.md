# MKW3DS Launcher
Launcher app used with MKW3DS to launch the mod. This repository also has a modification of the plugin loader used in the Luma3DS with plugin loading support fork but specific for MK7 title IDs. If you came here only for the plugin loader I recommend checking that repository instead.

# Warning
Since the v1.0, the CTGP7 launcher became closed source due to code containing anticheat (which i find dumb ngl), this project will not only be dedicated to MKW3DS but also an attempt to maintain an open source CTGP7 launcher so people can make new mods with it (like mine). This project will keep this goal till a way to avoid implementing anticheat code in the CTGP7 launcher will be implemented, and (hopefully) will be able to come back as an open source project.

# Building
Building MKW3DS Launcher is handled through buildtools. You have to have the following installed:
- [devkitPro with devkitARM](https://github.com/devkitPro/installer/releases)
- ctrulib (installed automatically with the devkitARM script)
**for a step to step build guide open Building_steps.md**


Once you have installed all the dependencies simply run `make` in the BootNTR directory and if you set it all up correctly it should build.


# Credits
- [@Nanquitas](https://github.com/Nanquitas) : BootNTRSelector, from which this app is forked.
- [@44670](https://github.com/44670) : Initial BootNTR and NTR CFW developer.
