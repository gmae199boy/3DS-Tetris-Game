#include "Polygon.h"
#include "main.h"


static int g_TextureNum = -1;


void Bg_Initialize(int texNum)
{
	g_TextureNum = texNum;
}

void Bg_DrawTop(void)
{
	Polygon_Draw(
		g_TextureNum,
		0,
		0,
		SCREEN_0_WIDTH,
		SCREEN_0_HEIGHT,
		0,
		0,
		SCREEN_0_WIDTH,
		SCREEN_0_HEIGHT
	);

	// É}ÉXÉN
	Polygon_Draw(
		g_TextureNum,
		0,
		0,
		200,
		40,
		300,
		200,
		200,
		40
	);
}

void Bg_DrawBottom(void)
{
	Polygon_Draw(
		g_TextureNum,
		0,
		0,
		SCREEN_1_WIDTH,
		SCREEN_1_HEIGHT,
		0,
		SCREEN_0_HEIGHT,
		SCREEN_1_WIDTH,
		SCREEN_1_HEIGHT
	);
}
