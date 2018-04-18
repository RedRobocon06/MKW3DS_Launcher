#include "main.h"
#include "draw.h"
#include "button.h"
#include <time.h>
#include "plgLoader.h"
#include "clock.h"
#include "drawableObject.h"

extern drawableScreen_t* botScreen;
extern drawableScreen_t *topScreen;
extern bool				forceUpdate;
extern progressbar_t *updateProgBar;
extern appInfoObject_t         *appTop;
extern sprite_t         *topInfoSprite; 
extern sprite_t		 *topInfoSpriteUpd;

static button_t         *V32Button = NULL;
static button_t         *V33Button = NULL;
static sprite_t         *tinyButtonBGSprite = NULL;
static sprite_t         *selTinyButtonBGSprite = NULL;
static sprite_t         *pressExitSprite = NULL;
static bool             userTouch = false;
static bool				launchPlgLoader = false;

extern bool restartneeded;


static u32 getNextRbwColor(u32 counter) {
	#define RNBW_FREQ 0.05235987755
	#define RNBW_CENTER 128.0f
	#define RNBW_WIDTH 110.0f

	u32 red = (u32)(sin(RNBW_FREQ*counter + 2) * RNBW_WIDTH + RNBW_CENTER);
	u32 green = (u32)(sin(RNBW_FREQ*counter + 0) * RNBW_WIDTH * 0.75f + RNBW_CENTER) << 8;
	u32 blue = (u32)(sin(RNBW_FREQ*counter + 4) * RNBW_WIDTH * 0.75f + RNBW_CENTER) << 16;
	return (0xFF000000 | red | green | blue);
}

void greyBottomScreen(bool state) {
	botScreen->background->footerText->isGreyedOut = state;
	botScreen->background->background->isGreyedOut = state;
}
void greyExit() {
	if(V32Button) V32Button->isGreyedOut = true;
	if (V33Button) V33Button->isGreyedOut = true;
	if (botScreen->background->footerText) botScreen->background->footerText->isHidden = true;
	botScreen->background->background->isGreyedOut = true;
	topScreen->background->background->isGreyedOut = true;
	if (topInfoSprite) topInfoSprite->isGreyedOut = true;
	if (topInfoSpriteUpd) topInfoSpriteUpd->isGreyedOut = true;
	if (updateProgBar) updateProgBar->isHidden = true;
	if (pressExitSprite) pressExitSprite->isHidden = true;
}

void    selectVersion(u32 mode)
{
	V32Button->disable(V32Button);
	V33Button->disable(V33Button);
    switch(mode)
    {
        case 1:
			launchPlgLoader = true;
			userTouch = true;
			greyBottomScreen(true);
			V33Button->isGreyedOut = true;
            break;
        case 2:
			greyBottomScreen(true);
			V32Button->isGreyedOut = true;
			pressExitSprite->isHidden = true;
			UpdatesMenu();
			pressExitSprite->isHidden = false;
			greyBottomScreen(false);
			V32Button->isGreyedOut = false;
			if (restartneeded) userTouch = true;
			//TODO
            break;
        default:
            break;
    }
	V32Button->enable(V32Button);
	V33Button->enable(V33Button);
}

void    initMainMenu(void)
{
    
    sprite_t *sprite;
    
    newSpriteFromPNG(&tinyButtonBGSprite, "romfs:/sprites/tinyButtonBackground.png");
	newSpriteFromPNG(&selTinyButtonBGSprite, "romfs:/sprites/selTinyButtonBackground.png");


    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/launchText.png");
    V32Button = newButton(5.0f, 48.0f, selectVersion, 1, tinyButtonBGSprite, sprite, selTinyButtonBGSprite, 0xFF0000FF);
    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/updateText.png");

    V33Button = newButton(5.0f, 132.0f, selectVersion, 2, tinyButtonBGSprite, sprite, selTinyButtonBGSprite, 0xFF0000FF);

    V32Button->show(V32Button);
    V33Button->show(V33Button);
    addBottomObject(V32Button);
    addBottomObject(V33Button);       

    newSpriteFromPNG(&pressExitSprite, "romfs:/sprites/textSprites/pressBExit.png");

    setSpritePos(pressExitSprite, 180.0f, 217.0f);

    changeBottomFooter(pressExitSprite); 

}

void    exitMainMenu(void)
{
    destroyButton(V32Button);
    destroyButton(V33Button);
    deleteSprite(tinyButtonBGSprite);
    deleteSprite(pressExitSprite);
	V32Button = NULL;
	V33Button = NULL;
	tinyButtonBGSprite = NULL;
	pressExitSprite = NULL;
	clearUpdateSprites();
}

int     mainMenu(void)
{
    u32         keys;
	u32			exitkey;

    waitAllKeysReleased();
    appInfoDisableAutoUpdate();  
    keys = 0;
	exitkey = 0;
	u32 framecount = 0;
	u64 timer = Timer_Restart();
	u64 timer2 = Timer_Restart();
	V32Button->isSelected = true;
	bool neeedToUpdate = forceUpdate;
    while (userTouch == false)
    {
		if (Timer_HasTimePassed(17, timer)) {
			timer = Timer_Restart();
			framecount++;
			if (framecount >= 120) {
				framecount = 0;
			}
		}
		V32Button->selTextColor = getNextRbwColor(framecount);
		if (neeedToUpdate) {
			V33Button->isSelected = true;
			V32Button->isSelected = false;
			selectVersion(2);
			neeedToUpdate = false;
		}
		if (keys & (KEY_CPAD_UP | KEY_CPAD_DOWN | KEY_DUP | KEY_DDOWN)) {
			if (Timer_HasTimePassed(250, timer2)) {
				timer2 = Timer_Restart();
				V32Button->isSelected = !V32Button->isSelected;
				V33Button->isSelected = !V33Button->isSelected;
			}
		}
		if (exitkey & KEY_B) {
			userTouch = true;
		}
		updateUI();
		keys = hidKeysDown() | hidKeysHeld();
		exitkey = hidKeysDown();
    }
	if (launchPlgLoader) {
		u32 prog = 0;
		u32 progtot;
		s64 lumaver;
		FILE* showFlag = NULL;
		appTop->sprite = topInfoSpriteUpd;
		if (osGetKernelVersion() < SYSTEM_VERSION(2, 54, 0) || R_FAILED(svcGetSystemInfo(&lumaver, 0x10000, 0)) || GET_VERSION_MAJOR((u32)lumaver) < 9) {
			bool errorloop = true;
			u32 keys = 0;
			clearTop(false);
			newAppTop(COLOR_RED, MEDIUM | BOLD | CENTER, "Failed to launch CTGP-7");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\n\nSystem firmware or Luma CFW");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "versions are outdated.");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nFirmware 11.4 and Luma v9.0");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "or avobe are needed to play.");
			while (errorloop) {
				updateUI();
				keys = hidKeysDown();
				if (keys & KEY_B) {
					errorloop = false;
				}
			}
		}
		else {
			launchPluginLoader();
			updateProgBar->rectangle->amount = 0;
			updateProgBar->isHidden = false;
			pressExitSprite->isHidden = true;
			clearTop(false);
			newAppTop(DEFAULT_COLOR, MEDIUM | BOLD | CENTER, "Launching CTGP-7");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\n\n\nOpen Mario Kart 7");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "from the home menu.");
			showFlag = fopen("/CTGP-7/config/waitBoot.flag", "rb");
			if (!showFlag) {
				showFlag = fopen("/CTGP-7/config/waitBoot.flag", "w");
				fwrite("dummy", 1, 5, showFlag);
				progtot = 250;
			}
			else {
				progtot = 17;
			}
			fclose(showFlag);
			while (prog <= progtot) {
				updateUI();
				svcSleepThread(10000000);
				updateProgBar->rectangle->amount = ((float)prog) / ((float)progtot);
				prog++;
			}
		}
	}
	greyExit();
	updateUI();
	svcSleepThread(500000000);
	return (1);
}