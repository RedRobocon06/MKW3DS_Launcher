#include "main.h"
#include "draw.h"
#include "button.h"
#include <time.h>
#include "plgLoader.h"
#include "clock.h"
#include "drawableObject.h"
#include "Unicode.h"
#include "sound.h"

static u64 MK7tids[] = { 0x0004000000030600ULL, 0x0004000000030700ULL, 0x0004000000030800ULL };
static const u64 updtid = 0x0000000E00000000ULL;
u64 launchAppID = 0;
FS_MediaType launchAppMedtype = 0xFF;
bool alreadyCancelled = false;

extern cwav_t                  *sfx_sound;


typedef struct TitleWithUpd_s
{
	AM_TitleEntry game;
	AM_TitleEntry update;
} TitleWithUpd_t;

typedef struct MK7GameInfo_s
{
	TitleWithUpd_t digital[sizeof(MK7tids)];
	TitleWithUpd_t cartridge;
} MK7GameInfo_t;

typedef struct SaveOpts_s
{
	u8 alwaysFE;
	bool gameCard;
	u8 gameReg;
}SaveOpts_t;

extern sprite_t         *pressExitSprite;
extern progressbar_t *updateProgBar;
extern appInfoObject_t         *appTop;
extern sprite_t         *topInfoSprite;
extern sprite_t		 *topInfoSpriteUpd;
extern sprite_t *updaterControlsText;
extern sprite_t *launchControlsSprite;
extern drawableScreen_t* botScreen;
extern drawableScreen_t *topScreen;


void setLaunchControlsMode(int mode) {
	switch (mode) {
	case 0:
	default:
		launchControlsSprite->posX = 0.0f;
		launchControlsSprite->amount = 1.f;
		break;
	case 1:
		launchControlsSprite->posX = 55.f;
		launchControlsSprite->amount = 0.7225f;
		break;
	case 2:
		launchControlsSprite->posX = 134.f;
		launchControlsSprite->amount = 0.33f;
		break;
	}
}

int getOptFromSave(TitleWithUpd_t** opts, u32 numberOpts, bool gameCard, bool* autoLaunch) {
	FILE* optFile = NULL;
	SaveOpts_t saveOpt = { 0 };
	optFile = fopen(LAUNCH_OPT_SAVE, "rb");
	u32 fileSize = 0;
	int ret = 0;
	if (!optFile) {
		*autoLaunch = false;
		return 0;
	}
	fseek(optFile, 0, SEEK_END);
	fileSize = ftell(optFile);
	if (fileSize != sizeof(SaveOpts_t)) {
		fclose(optFile);
		*autoLaunch = false;
		return 0;
	}
	fseek(optFile, 0, SEEK_SET);
	fread(&saveOpt, 1, sizeof(SaveOpts_t), optFile);
	if (saveOpt.alwaysFE != 0xFE) {
		*autoLaunch = false;
		fclose(optFile);
		return 0;
	}
	if (saveOpt.gameCard) {
		*autoLaunch = gameCard;
		fclose(optFile);
		return 0;
	}
	for (int i = (gameCard ? 1 : 0) ; i < numberOpts; i++) {
		u32 tid = (u32)(opts[i]->game.titleID);
		if ((tid == 0x00030600 && saveOpt.gameReg == 0) || (tid == 0x00030700 && saveOpt.gameReg == 1) || (tid == 0x00030800 && saveOpt.gameReg == 2)) {
			ret = i;
			*autoLaunch = true;
			break;
		}
	}
	fclose(optFile);
	return ret;
}

void saveOptToSave(TitleWithUpd_t** opts, u32 curropt, bool gameCard) {
	FILE* optFile = NULL;
	SaveOpts_t saveOpt;
	memset(&saveOpt, 0xFF, sizeof(SaveOpts_t));
	optFile = fopen(LAUNCH_OPT_SAVE, "w");
	if (!optFile) return;
	saveOpt.alwaysFE = 0xFE;
	if (curropt == 0 && gameCard) {
		saveOpt.gameCard = true;
		saveOpt.gameReg = 0xFF;
		fwrite(&saveOpt, 1, sizeof(SaveOpts_t), optFile);
		fclose(optFile);
		return;
	}
	saveOpt.gameCard = false;
	switch ((u32)(opts[curropt]->game.titleID)) {
	case 0x00030600:
		saveOpt.gameReg = 0;
		break;
	case 0x00030700:
		saveOpt.gameReg = 1;
		break;
	case 0x00030800:
		saveOpt.gameReg = 2;
		break;
	default:
		saveOpt.gameReg = 0xFF;
		break;
	}
	fwrite(&saveOpt, 1, sizeof(SaveOpts_t), optFile);
	fclose(optFile);
	return;
}

u32 getNumberOfGames(MK7GameInfo_t* gameList) {
	u32 ret = 0;
	if (!gameList) return ret;
	if ((u32)(gameList->cartridge.game.titleID)) ret++;
	for (int i = 0; i < sizeof(MK7tids); i++) {
		if ((u32)(gameList->digital[i].game.titleID)) ret++;
	}
	return ret;
}

void freeGameList(MK7GameInfo_t* gameList) {
	if (!gameList) return;
	free(gameList);
}

MK7GameInfo_t* newGameInfo() {
	MK7GameInfo_t* ret = calloc(1, sizeof(MK7GameInfo_t));
	return ret;
}

TitleWithUpd_t** getGameArray(MK7GameInfo_t* gameList, bool* gameCard) {
	TitleWithUpd_t** ret = malloc(4 * 4);
	if (!ret) return NULL;
	int cnt = 0;
	if ((u32)(gameList->cartridge.game.titleID)) {
		*gameCard = true;
		ret[cnt++] = &(gameList->cartridge);
	}
	for (int i = 0; i < sizeof(MK7tids); i++) {
		if ((u32)(gameList->digital[i].game.titleID))
			ret[cnt++] = &(gameList->digital[i]);
	}
	return ret;
}

MK7GameInfo_t* getAllPossibleGames() {
	
	AM_TitleEntry tmpEntry = { 0 };
	MK7GameInfo_t* gameList = newGameInfo();
	if (!gameList) return NULL;
	amInit();
	for (int i = 0; i < sizeof(MK7tids); i++) {
		if (AM_GetTitleInfo(MEDIATYPE_GAME_CARD, 1, &MK7tids[i], &tmpEntry) >= 0 && (u32)(tmpEntry.titleID)) {
			memcpy(&(gameList->cartridge.game), &tmpEntry, sizeof(AM_TitleEntry));
			memset(&tmpEntry, 0, sizeof(AM_TitleEntry));
			u64 updTid = MK7tids[i] | updtid;
			if (AM_GetTitleInfo(MEDIATYPE_SD, 1, &updTid, &tmpEntry) >= 0 && (u32)(tmpEntry.titleID)) {
				memcpy(&(gameList->cartridge.update), &tmpEntry, sizeof(AM_TitleEntry));
				memset(&tmpEntry, 0, sizeof(AM_TitleEntry));
			}
			break;
		}
	}
	memset(&tmpEntry, 0, sizeof(AM_TitleEntry));
	for (int i = 0; i < sizeof(MK7tids); i++) {
		if (AM_GetTitleInfo(MEDIATYPE_SD, 1, &MK7tids[i], &tmpEntry) >= 0 && (u32)(tmpEntry.titleID)) {
			memcpy(&(gameList->digital[i].game), &tmpEntry, sizeof(AM_TitleEntry));
			memset(&tmpEntry, 0, sizeof(AM_TitleEntry));
			u64 updTid = MK7tids[i] | updtid;
			if (AM_GetTitleInfo(MEDIATYPE_SD, 1, &updTid, &tmpEntry) >= 0 && (u32)(tmpEntry.titleID)) {
				memcpy(&(gameList->digital[i].update), &tmpEntry, sizeof(AM_TitleEntry));
				memset(&tmpEntry, 0, sizeof(AM_TitleEntry));
			}
		}
	}
	amExit();
	return gameList;
}

bool drawLaunchText(TitleWithUpd_t** opts, int curropt, bool gameCard, int totalOpt, bool* canAutoLaunch) {
	u32 ver;
	bool ret1 = false;
	bool ret2 = false;
	clearTop(false);
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM | BOLD, "Select game to launch");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Option %d / %d", curropt + 1, totalOpt);
	if (curropt == 0 && gameCard) {
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Media: Gamecard");
	}
	else {
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Media: Digital");
	}
	switch ((u32)(opts[curropt]->game.titleID))
	{
	case 0x00030600:
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Region: JPN");
		ret1 = true;
		break;
	case 0x00030700:
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Region: EUR");
		ret1 = true;
		break;
	case 0x00030800:
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Region: USA");
		ret1 = true;
		break;
	default:
		newAppTop(COLOR_ORANGE, CENTER | MEDIUM, "Region: ??? (0x%08X)", (u32)(opts[curropt]->game.titleID));
		ret1 = false;
	}
	ver = fmax(opts[curropt]->game.version, opts[curropt]->update.version);
	switch (ver)
	{
	case 0:
		newAppTop(COLOR_ORANGE, MEDIUM | CENTER, "Version: Rev0");
		ret2 = false;
		break;
	case 1040:
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Version: Rev0 1.1");
		ret2 = true;
		break;
	case 2112:
	case 2048:
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Version: Rev1");
		ret2 = true;
		break;
	default:
		newAppTop(COLOR_ORANGE, MEDIUM | CENTER, "Version: ???? (%d)", ver);
		ret2 = false;
		break;
	}
	if (!ret1) {
		newAppTop(COLOR_ORANGE, MEDIUM | CENTER, "\nUnknown region, only");
		newAppTop(COLOR_ORANGE, MEDIUM | CENTER, "EUR/USA/JPN are supported.");
	} else if (!ret2) {
		newAppTop(COLOR_ORANGE, MEDIUM | CENTER, "\nDownload the 1.1 patch from");
		newAppTop(COLOR_ORANGE, MEDIUM | CENTER, "the eshop to use this game.");
	}
	if (!(ret1 && ret2)) *canAutoLaunch = false;
	if (*canAutoLaunch) {
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nPress " FONT_DLR " to cancel.");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Press " FONT_A " to launch.");
	}
	return ret1 && ret2;
}

void launchMod() {
	s64 lumaver;
	MK7GameInfo_t* gameList;
	appTop->sprite = topInfoSpriteUpd;
	changeTopFooter(launchControlsSprite);
	setLaunchControlsMode(2);
	if (osGetKernelVersion() < SYSTEM_VERSION(2, 54, 0) || R_FAILED(svcGetSystemInfo(&lumaver, 0x10000, 0)) || GET_VERSION_MAJOR((u32)lumaver) < 9) {
		bool errorloop = true;
		u32 keys = 0;
		clearTop(false);
		PLAYBOOP();
		newAppTop(COLOR_RED, MEDIUM | BOLD | CENTER, "Failed to launch CTGP-7");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nSystem firmware or Luma CFW");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "versions are outdated.");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nFirmware 11.4 and Luma v9.0");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "or avobe are needed to play.");
		while (errorloop) {
			updateUI(); 
			keys = hidKeysDown();
			if (keys & KEY_B) {
				PLAYBOOP();
				errorloop = false;
			}
		}
		clearTop(false);
		removeAppTop();
		changeTopFooter(NULL);
		return;
	}
	gameList = getAllPossibleGames();
	u32 gameListCount = getNumberOfGames(gameList);
	if (!gameListCount) {
		freeGameList(gameList);
		bool errorloop = true;
		u32 keys = 0;
		clearTop(false);
		PLAYBOOP();
		newAppTop(COLOR_RED, MEDIUM | BOLD | CENTER, "Failed to launch CTGP-7");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Mario Kart 7 was not detected.");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nIf you are playing");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "from cartridge make sure");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "it shows up in the home menu.");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nOnly EUR / USA / JPN games");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "are supported.");
		while (errorloop) {
			updateUI();
			keys = hidKeysDown();
			if (keys & KEY_B) {
				PLAYBOOP();
				errorloop = false;
			}
		}
		clearTop(false);
		removeAppTop();
		freeGameList(gameList);
		changeTopFooter(NULL);
		return;
	}
	bool launchLoop = true;
	bool gameCard = false;
	bool autoLaunch = false;
	u32 keys = 0;
	u32 counter = 0;
	u64 timer = Timer_Restart();
	TitleWithUpd_t** opts = getGameArray(gameList, &gameCard);
	int curropt = getOptFromSave(opts, gameListCount, gameCard, &autoLaunch);
	if (alreadyCancelled) autoLaunch = false;
	else alreadyCancelled = true;
	bool canLaunch = drawLaunchText(opts, curropt, gameCard, gameListCount, &autoLaunch);
	if (autoLaunch) {
		updateProgBar->isHidden = false;
		updateProgBar->rectangle->amount = 0;
		launchControlsSprite->isHidden = true;
	}
	setLaunchControlsMode(canLaunch ? 0 : 1);
	while (launchLoop) {
		if (keys & KEY_RIGHT) {
			autoLaunch = false;
			updateProgBar->isHidden = true;
			updateProgBar->rectangle->amount = 0;
			launchControlsSprite->isHidden = false;
			curropt++;
			if (curropt >= gameListCount) curropt = gameListCount - 1;
			else PLAYCLICK();
			canLaunch = drawLaunchText(opts, curropt, gameCard, gameListCount, &autoLaunch);
			setLaunchControlsMode(canLaunch ? 0 : 1);
		}
		if (keys & KEY_LEFT) {
			autoLaunch = false;
			updateProgBar->isHidden = true;
			updateProgBar->rectangle->amount = 0;
			launchControlsSprite->isHidden = false;
			curropt--;
			if (curropt < 0) curropt = 0;
			else PLAYCLICK();
			canLaunch = drawLaunchText(opts, curropt, gameCard, gameListCount, &autoLaunch);
			setLaunchControlsMode(canLaunch ? 0 : 1);
		}
		updateUI();
		if (keys & KEY_A) {
			updateProgBar->isHidden = true;
			updateProgBar->rectangle->amount = 0;
			launchControlsSprite->isHidden = false;
			autoLaunch = false;
			if (canLaunch) { 
				PLAYBEEP();
				saveOptToSave(opts, curropt, gameCard);
				launchPluginLoader();
				launchAppMedtype = (gameCard && curropt == 0) ? MEDIATYPE_GAME_CARD : MEDIATYPE_SD;
				launchAppID = opts[curropt]->game.titleID;
				launchLoop = false;
			}
		}
		if (keys & KEY_B) {
			PLAYBOOP();
			updateProgBar->isHidden = true;
			updateProgBar->rectangle->amount = 0;
			launchControlsSprite->isHidden = false;
			autoLaunch = false;
			launchLoop = false;
		}
		keys = hidKeysDown();
		if (Timer_HasTimePassed(6.0f, timer) && autoLaunch){
			timer = Timer_Restart();
			counter++;
		}
		if (autoLaunch) {
			updateProgBar->rectangle->amount = ((float)(counter) / 160.f);
			if (counter >= 160) {
				if (canLaunch) {
					keys |= KEY_A;
				}
			}
		}
	}
	clearTop(false);
	removeAppTop();
	free(opts);
	freeGameList(gameList);
	changeTopFooter(NULL);
	updateProgBar->rectangle->amount = 0;
	updateProgBar->isHidden = true;
	appTop->sprite = topInfoSprite;
	return;
}