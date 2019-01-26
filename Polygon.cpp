#include <nn/gx.h>
#include <nn/math.h>
#include <math.h>
#include "textureManager.h"
#include "Polygon.h"


struct VERTEX_3D
{
	nn::math::VEC4 Position;
	nn::math::VEC4 Color;
	nn::math::VEC2 Texcoord;
};


static VERTEX_3D g_PolygonVertex[4];
static nn::math::VEC4 g_PolygonColor(1.0f, 1.0f, 1.0f, 1.0f);


// É|ÉäÉSÉìï`âÊÇÃèâä˙âª
void Polygon_Initialize(void)
{
	g_PolygonVertex[0].Color = g_PolygonColor;
	g_PolygonVertex[1].Color = g_PolygonColor;
	g_PolygonVertex[2].Color = g_PolygonColor;
	g_PolygonVertex[3].Color = g_PolygonColor;
}

// É|ÉäÉSÉìï`âÊÇÃèIóπèàóù
void Polygon_Finalize(void)
{

}

// É|ÉäÉSÉìÉJÉâÅ[ÇÃê›íË
void Polygon_SetColor(float r, float g, float b, float a)
{
	g_PolygonColor.x = r;
	g_PolygonColor.y = g;
	g_PolygonColor.z = b;
	g_PolygonColor.w = a;
}

// É|ÉäÉSÉìï`âÊ
void Polygon_Draw(int texNum, float dx, float dy, float dw, float dh)
{
	g_PolygonVertex[0].Position = nn::math::VEC4(dx     , dy     , 0.0f, 1.0f);
	g_PolygonVertex[1].Position = nn::math::VEC4(dx + dw, dy     , 0.0f, 1.0f);
	g_PolygonVertex[2].Position = nn::math::VEC4(dx     , dy + dh, 0.0f, 1.0f);
	g_PolygonVertex[3].Position = nn::math::VEC4(dx + dw, dy + dh, 0.0f, 1.0f);

	g_PolygonVertex[0].Color = g_PolygonColor;
	g_PolygonVertex[1].Color = g_PolygonColor;
	g_PolygonVertex[2].Color = g_PolygonColor;
	g_PolygonVertex[3].Color = g_PolygonColor;

	g_PolygonVertex[0].Texcoord = nn::math::VEC2(0.0f, 1.0f);
	g_PolygonVertex[1].Texcoord = nn::math::VEC2(1.0f, 1.0f);
	g_PolygonVertex[2].Texcoord = nn::math::VEC2(0.0f, 0.0f);
	g_PolygonVertex[3].Texcoord = nn::math::VEC2(1.0f, 0.0f);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Position);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Color);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Texcoord);

	GLuint n = TextureManager_GetTexture(texNum);
	glBindTexture(GL_TEXTURE_2D, n);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// É|ÉäÉSÉìï`âÊ(ÉeÉNÉXÉ`ÉÉÇÃêÿÇËéÊÇËéwíËïtÇ´)
void Polygon_Draw(
	int texNum,
	float dx, float dy, float dw, float dh,
	int tcx, int tcy, int tcw, int tch)
{
	g_PolygonVertex[0].Position = nn::math::VEC4(dx, dy, 0.0f, 1.0f);
	g_PolygonVertex[1].Position = nn::math::VEC4(dx + dw, dy, 0.0f, 1.0f);
	g_PolygonVertex[2].Position = nn::math::VEC4(dx, dy + dh, 0.0f, 1.0f);
	g_PolygonVertex[3].Position = nn::math::VEC4(dx + dw, dy + dh, 0.0f, 1.0f);

	g_PolygonVertex[0].Color = g_PolygonColor;
	g_PolygonVertex[1].Color = g_PolygonColor;
	g_PolygonVertex[2].Color = g_PolygonColor;
	g_PolygonVertex[3].Color = g_PolygonColor;

	int tw = TextureManager_GetTextureWidth(texNum);
	int th = TextureManager_GetTextureHeight(texNum);

	float u0 = (float)tcx / tw;
	float v0 = 1.0f - (float)tcy / th;
	float u1 = (float)(tcx + tcw) / tw;
	float v1 = 1.0f - (float)(tcy + tch) / th;

	g_PolygonVertex[0].Texcoord = nn::math::VEC2(u0, v0);
	g_PolygonVertex[1].Texcoord = nn::math::VEC2(u1, v0);
	g_PolygonVertex[2].Texcoord = nn::math::VEC2(u0, v1);
	g_PolygonVertex[3].Texcoord = nn::math::VEC2(u1, v1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Position);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Color);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Texcoord);

	GLuint n = TextureManager_GetTexture(texNum);
	glBindTexture(GL_TEXTURE_2D, n);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// É|ÉäÉSÉìï`âÊ(âÒì]ïtÇ´)
void Polygon_Draw(int texNum, float dx, float dy, float dw, float dh, float cx, float cy, float sx, float sy, float ang)
{
	float x0 = -cx;
	float y0 = -cy;
	float x1 = -cx + dw;
	float y1 = -cy;
	float x2 = -cx;
	float y2 = -cy + dh;
	float x3 = -cx + dw;
	float y3 = -cy + dh;

	float dx0 = x0 * sx * cosf(ang) - y0 * sy * sinf(ang) + dx + cx;
	float dy0 = x0 * sx * sinf(ang) + y0 * sy * cosf(ang) + dy + cy;
	float dx1 = x1 * sx * cosf(ang) - y1 * sy * sinf(ang) + dx + cx;
	float dy1 = x1 * sx * sinf(ang) + y1 * sy * cosf(ang) + dy + cy;
	float dx2 = x2 * sx * cosf(ang) - y2 * sy * sinf(ang) + dx + cx;
	float dy2 = x2 * sx * sinf(ang) + y2 * sy * cosf(ang) + dy + cy;
	float dx3 = x3 * sx * cosf(ang) - y3 * sy * sinf(ang) + dx + cx;
	float dy3 = x3 * sx * sinf(ang) + y3 * sy * cosf(ang) + dy + cy;

	g_PolygonVertex[0].Position = nn::math::VEC4(dx0, dy0, 0.0f, 1.0f);
	g_PolygonVertex[1].Position = nn::math::VEC4(dx1, dy1, 0.0f, 1.0f);
	g_PolygonVertex[2].Position = nn::math::VEC4(dx2, dy2, 0.0f, 1.0f);
	g_PolygonVertex[3].Position = nn::math::VEC4(dx3, dy3, 0.0f, 1.0f);

	g_PolygonVertex[0].Color = g_PolygonColor;
	g_PolygonVertex[1].Color = g_PolygonColor;
	g_PolygonVertex[2].Color = g_PolygonColor;
	g_PolygonVertex[3].Color = g_PolygonColor;

	g_PolygonVertex[0].Texcoord = nn::math::VEC2(0.0f, 1.0f);
	g_PolygonVertex[1].Texcoord = nn::math::VEC2(1.0f, 1.0f);
	g_PolygonVertex[2].Texcoord = nn::math::VEC2(0.0f, 0.0f);
	g_PolygonVertex[3].Texcoord = nn::math::VEC2(1.0f, 0.0f);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Position);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Color);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX_3D), (GLvoid*)&g_PolygonVertex[0].Texcoord);

	GLuint n = TextureManager_GetTexture(texNum);
	glBindTexture(GL_TEXTURE_2D, n);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
