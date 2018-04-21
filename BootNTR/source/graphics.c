#include "graphics.h"
#include "drawableObject.h"
#include "button.h"

sprite_t         *bottomSprite = NULL;
sprite_t         *topSprite = NULL;

sprite_t         *topInfoSprite = NULL;
sprite_t		 *topInfoSpriteUpd = NULL;
sprite_t         *launchControlsSprite = NULL;
drawableScreen_t *botScreen = NULL;
drawableScreen_t *topScreen = NULL;
appInfoObject_t         *appTop = NULL;

extern char g_modversion[15];

char textVersion[20] = { 0 };


void    initUI(void)
{
    backgroundScreen_t *bg;

    newSpriteFromPNG(&topSprite, "romfs:/sprites/topBackground.png");
    newSpriteFromPNG(&bottomSprite, "romfs:/sprites/bottomBackground.png");    

    newSpriteFromPNG(&topInfoSprite, "romfs:/sprites/topInfoBackground.png");
	newSpriteFromPNG(&topInfoSpriteUpd, "romfs:/sprites/topInfoBackgroundUpd.png");

	newSpriteFromPNG(&launchControlsSprite, "romfs:/sprites/textSprites/launchControlsSprite.png");

    setSpritePos(topSprite, 0, 0);
    setSpritePos(bottomSprite, 0, 0);
	setSpritePos(launchControlsSprite, 0.0f, 220.0f);
    
    bg = newBackgroundObject(bottomSprite, NULL, NULL);
    botScreen = newDrawableScreen(bg);
    bg = newBackgroundObject(topSprite, NULL, NULL);
    topScreen = newDrawableScreen(bg);

    setSpritePos(topInfoSprite, 50, 20);
	setSpritePos(topInfoSpriteUpd, 50, 20);
    appTop = newAppInfoObject(topInfoSprite, 14, 62.0f, 30.0f);
    appInfoSetTextBoundaries(appTop, 338.0f, 210.0f);
	
    updateUI();
}

void    exitUI(void)
{
    deleteAppInfoObject(appTop);
    deleteSprite(bottomSprite);
    deleteSprite(topSprite);
    deleteSprite(topInfoSprite);
	deleteSprite(launchControlsSprite);
}

static inline void drawUITop(void)
{
    setScreen(GFX_TOP);
    
    topScreen->draw(topScreen);
    setTextColor(COLOR_BLANK);
	if (textVersion[0] == '\0') {
		sprintf(textVersion, "Ver. %s", g_modversion);
	}
    renderText(1.0f, 1.0f, 0.4f, 0.45f, false, textVersion, NULL, 0.0f);
    drawAppInfo(appTop);
}

static inline void drawUIBottom(void)
{
    setScreen(GFX_BOTTOM);
    
    botScreen->draw(botScreen);
}

int   updateUI(void)
{
    hidScanInput();
    drawUITop();
	
    drawUIBottom();
    updateScreen();
    return (1);
}

void    addTopObject(void *object)
{
    addObjectToScreen(topScreen, object);
}

void    setExTopObject(void *object)
{
	setExTopObjectToScreen(topScreen, object);
}

void    addBottomObject(void *object)
{
    addObjectToScreen(botScreen, object);
}

void    changeTopFooter(sprite_t *footer)
{
    changeBackgroundFooter(topScreen->background, footer);
}

void    changeTopHeader(sprite_t *header)
{
    changeBackgroundHeader(topScreen->background, header);
}

void    changeBottomFooter(sprite_t *footer)
{
    changeBackgroundFooter(botScreen->background, footer);
}

void    changeBottomHeader(sprite_t *header)
{
    changeBackgroundHeader(botScreen->background, header);
}

void    clearTopScreen(void)
{
    clearObjectListFromScreen(topScreen);
}

void    clearBottomScreen(void)
{
    clearObjectListFromScreen(botScreen);
}