#include "font.h"
#include <stdlib.h>
#include <stdio.h>
#include "sdl_extra.h"
#include <string.h>
#include "editor.h"

extern SDL_Renderer* renderer;

static unsigned char* createTtfBuffer(const char* fileName) {
	size_t fileSize;
	FILE* fontFile;

	if (fopen_s(&fontFile, fileName, "rb") != 0) {
		fprintf(stderr, "Error opening font file");
	}

	fseek(fontFile, 0, SEEK_END);
	fileSize = ftell(fontFile);
	rewind(fontFile);

	unsigned char* ttf_buffer = (unsigned char*)malloc(sizeof(unsigned char) * fileSize);
	size_t result = fread(ttf_buffer, 1, fileSize, fontFile);

	if (result != fileSize) {
		fprintf(stderr, "Couldn't read entire font file");
		exit(3);
	}

	fclose(fontFile);

	return ttf_buffer;
}

Font loadFontFromFile(const char* fileName, const float fontSize) {
	Font font;

	unsigned char* bitmap = (unsigned char*)malloc(sizeof(unsigned char) * PALETTE_HEIGHT * PALETTE_WIDTH);
	unsigned char* ttf_buffer = createTtfBuffer(fileName);

	stbtt_BakeFontBitmap(ttf_buffer, 0, fontSize, bitmap, PALETTE_WIDTH, PALETTE_HEIGHT, ASCII_LOW, ASCII_HIGH, font.charData);
	
	free(ttf_buffer);

	SDL_Surface* bakedFontSurface = createSurfaceFromPalette(bitmap);
	
	font.texture = scp(SDL_CreateTextureFromSurface(renderer, bakedFontSurface));

	SDL_SetTextureBlendMode(font.texture, SDL_BLENDMODE_ADD);

	free(bitmap);

	bakedFontSurface->pixels = NULL;
	//SDL_FreeSurface(bakedFontSurface);

	return font;
}

void drawCaret(const Font* const font, const float scale, int cursorCol, int cursorRow,
               SDL_Rect *camera) {

#define RIGHT_PADDING 1

    int width = (int) (font->charData[0].xadvance + RIGHT_PADDING);
    int height = (int) scale;
    
    SDL_Rect caret = {
        .x = (int) cursorCol * width,
        .y = (int) (cursorRow * height) + (height / 4), 
        .w = width,
        .h = height,
    };

    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 100);
    SDL_RenderFillRect(renderer, &caret);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    SDL_RenderDrawRect(renderer, &caret);

    if(caret.x + width> camera->x + SCREEN_WIDTH)
    {
        camera->x += SCREEN_WIDTH / 3;
    }
    if(caret.x < camera->x)
    {
        camera->x = (int) caret.x;
    }
    if(caret.y + height > camera->y + SCREEN_HEIGHT)
    {
        camera->y = SCREEN_HEIGHT / 3;
    }
    if(caret.y < camera->y)
    {
        camera->y = caret.y;
    }

#undef RIGHT_PADDING
}

void drawChar(const Font* const font, const char c, const float scale, Vec2f* pos) {

	stbtt_aligned_quad cquad;
	stbtt_GetBakedQuad(font->charData, PALETTE_WIDTH, PALETTE_HEIGHT, c - ASCII_LOW, &pos->x, &pos->y, &cquad, 0);

	SDL_Rect src = {
		.x = (int) (cquad.s0 * PALETTE_WIDTH),
		.y = (int) (cquad.t0 * PALETTE_HEIGHT),
		.w = (int) (cquad.x1 - cquad.x0),
		.h = (int) (cquad.y1 - cquad.y0)
	};
	SDL_Rect dst = {
		.x = (int) cquad.x0,
		.y = (int) (cquad.y0 + scale), //We have to add the scale to the y position here because SDL and openGl have different coord systems. 
		.w = src.w,
		.h = src.h,
	};


	SDL_RenderCopy(renderer, font->texture, &src, &dst);


}

void drawString(const Font* const font, const char* string, const float scale, Vec2f* startPos) {
	while (*string) {
        if(*string == '\n') {
            startPos->x = STARTING_X_POS;
            break;
        }
        
		drawChar(font, *string, scale, startPos);
        
        string++;
	}
}
