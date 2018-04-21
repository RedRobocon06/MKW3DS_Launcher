#include "main.h"
#include "draw.h"
#include "button.h"
#include <time.h>

u8                *tmpBuffer;
char              *g_primary_error = NULL;
char              *g_secondary_error = NULL;
char              *g_third_error = NULL;
bool              g_exit = false;
bool			  forceUpdate;
bool			  restartneeded = false;
char			  g_modversion[15] = { '\0' };
u8				  g_launcherversion[3] = { '\0' };
u8				  hmac[0x20];
extern sprite_t*  updaterControlsText;
extern char*      versionList[100];
extern char textVersion[20];
extern u64 launchAppID;
extern FS_MediaType launchAppMedtype;

void getModVersion() {
	memset(g_modversion, 0, 15);
	FILE* file = NULL;
	file = fopen("/CTGP-7/config/version.bin", "r");
	if (!file) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x0);
	fread(g_modversion, 1, 15, file);
	fclose(file);
	file = NULL;
	textVersion[0] = '\0';
	memset(g_launcherversion, 0, 3);
	file = fopen("/CTGP-7/config/expectedVer.bin", "r");
	if (!file) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x1);
	fread(g_launcherversion, 1, 3, file);
	if (!g_modversion[0] || !g_launcherversion[0]) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x2);
	fclose(file);
	file = NULL;
}

void finishUpdate() {
	// Sigh... This is the only way to properly finish the 3dsx installation
	FILE* tmpFile = NULL;
	tmpFile = fopen(TOINSTALL_3DSX_PATH, "r");
	if (tmpFile) {
		fclose(tmpFile);
		FILE* endBrewFile = fopen_mkdir(FINAL_3DSX_PATH, "w"); // Generate path
		if (endBrewFile) {
			fclose(endBrewFile);
			remove(FINAL_3DSX_PATH);
			copy_file(TOINSTALL_3DSX_PATH, TEMPORAL_3DSX_PATH);
			rename(TOINSTALL_3DSX_PATH, FINAL_3DSX_PATH);
		}
	}
}

bool isExpectedVersion() {
	if (APP_VERSION_MAJOR < g_launcherversion[0]) {
		return false;
	}
	else if (APP_VERSION_MAJOR == g_launcherversion[0]) {
		if (APP_VERSION_MINOR < g_launcherversion[1]) {
			return false;
		}
		else if ((APP_VERSION_MINOR == g_launcherversion[1])) {
			if (APP_VERSION_MICRO < g_launcherversion[2]) {
				return false;
			}
		}
	}
	u32 vervar = (APP_VERSION_MAJOR) | (APP_VERSION_MINOR << 8) | (APP_VERSION_MICRO << 16);
	FILE* file = NULL;
	file = fopen("/CTGP-7/config/expectedVer.bin", "w");
	if (!file) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x3);
	fwrite(&vervar, 1, 3, file);
	fclose(file);
	return true;
}

int main(void)
{
    //u32         kernelVersion;

    gfxInitDefault();
    romfsInit();
    ptmSysmInit();
	acInit();

	drawInit();

	getModVersion();
	bool isProperVer = isExpectedVersion();
    initUI();
	InitUpdatesUI();
	if (!isProperVer) {
		changeTopFooter(updaterControlsText);
		updaterControlsText->isHidden = false;
		setControlsMode(2);
		clearTop(false);
		newAppTop(COLOR_RED, BOLD | MEDIUM | CENTER, "Version mismatch error");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "This app version is older");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "than a previous launched one.");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "App version: %d.%d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_MICRO);
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Expected version: %d.%d.%d", g_launcherversion[0], g_launcherversion[1], g_launcherversion[2]);
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER | BOLD, "\nTo fix the error:");
		if (!envIsHomebrew()) {
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "- Install the following cia");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"FINAL_CIA_PATH);
		}
		else {
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "- Copy this file");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"TEMPORAL_3DSX_PATH);
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "to this location");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"FINAL_3DSX_PATH);
		}
		u32 keys = 0;
		bool errorLoop = true;
		while (errorLoop) {
			if (keys & KEY_B) {
				errorLoop = false;
			}
			updateUI();
			keys = hidKeysDown();
		}
		greyExit();
		updateUI();
		svcSleepThread(500000000);
	}
	else {
		newAppTop(DEFAULT_COLOR, BOLD | MEDIUM | CENTER, "Checking for updates...");
		updateUI();
		finishUpdate();
		forceUpdate = updateAvailable();
		initMainMenu();
		removeAppTop();
		//
		mainMenu();
		//
	}
	drawEndFrame();
	exitMainMenu();
	exitUI();
	drawExit();
    acExit();
    amExit();
    httpcExit();
    romfsExit();
    gfxExit();
    ptmSysmExit();
	if (launchAppID && launchAppMedtype != 0xFF) {
		APT_PrepareToDoApplicationJump(0, launchAppID, launchAppMedtype);
		APT_DoApplicationJump(NULL, 0, hmac);
		for (;;);
	}
    return (0);
}
