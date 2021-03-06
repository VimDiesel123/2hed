#include "editor.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "util.h"

//void grow(int capacity, size_t elementSize, int sizeToGrow, void** data) {
//
//	int newCapacity = capacity ? (capacity * 2) + sizeToGrow : 8 + sizeToGrow;
//
//	void* pointer;
//	if (*data) {
//		pointer = realloc(buf_header(*data), (newCapacity * elementSize) + sizeof(buffer_header));
//	}
//	else {
//		pointer = realloc(0, (newCapacity * elementSize) + sizeof(buffer_header));
//		((buffer_header*)pointer)->count = 0;
//	}
//	*data = (void*)((buffer_header*)pointer + 1);
//	buf_cap_raw(*data) = newCapacity;
//}

static Line* getCurrentLine(const Editor* editor) {
	Line* line = editor->lines;
	for (size_t i = 0; i < editor->cursorRow; ++i) {
		line = line->next;
	}
	return line;
}

static Line* createLine(void)
{
	Line* result = (Line*)malloc(sizeof(Line));
	result->next = NULL;
	result->prev = NULL;

	result->text[0] = '\0';
	return result;
}



static inline size_t getCurrentLineLength(const Editor* editor) {
	return strlen(getCurrentLine(editor)->text);
}



static inline unsigned char* getCharacterUnderCursor(const Editor* editor) {
	Line* currentLine = getCurrentLine(editor);
	return (unsigned char*)currentLine->text + editor->cursorCol;
}


void initEditor(Editor* editor) {
	editor->lines = NULL;
	editor->cursorCol = 0;
	editor->cursorRow = 0;
	editor->lastLine = editor->lines;
	editor->lineCount = 0;
}

void freeEditor(Editor* editor) {

	Line* line = editor->lastLine;
	while (line)
	{
		Line* temp = line;
		line = line->prev;
		free(temp);
	}


}



void addTextAtCursor(Editor* editor, char* text) {
	char* temp = (char*)getCharacterUnderCursor(editor);
	size_t bytesToShift = strlen(temp) + 1; //+1 because strlen doesn't count null terminator.
	editor->cursorCol++;
	memmove((void*)getCharacterUnderCursor(editor),
		(void*)temp,
		bytesToShift);
	*temp = *text;
}

static void moveCurrentLineUp(Editor* editor) {
	Line* currentLine = getCurrentLine(editor);
	Line* previous = currentLine->prev;
	strncat_s(previous->text, sizeof(previous->text), currentLine->text, strlen(currentLine->text));
	if (currentLine == editor->lastLine) {
		previous->next = NULL;
		editor->lastLine = previous;
	}
	else {
		previous->next = currentLine->next;
		currentLine->next->prev = previous;
	}
	//TODO: free the line you don't need anymore!
	free(currentLine);
}

void backspace(Editor* editor) {
	if (editor->cursorCol <= 0 && editor->cursorRow <= 0) return;
	char* temp = (char*)getCharacterUnderCursor(editor);
	if (editor->cursorCol <= 0)
	{
		//TODO: move this to it's own function?
		Line* currentLine = getCurrentLine(editor);
		if (currentLine->prev == NULL) {
			editor->cursorCol = 0;
		}
		else {
			editor->cursorCol = (int)strlen(currentLine->prev->text);
		}
		moveCurrentLineUp(editor);
		editor->cursorRow--;
		editor->lineCount--;
		return;
	}
	else
	{
		editor->cursorCol--;
		size_t bytesToShift = strlen(temp) + 1;
		memmove((void*)getCharacterUnderCursor(editor),
			(void*)temp,
			bytesToShift);
	}
}

void moveCursorLeft(Editor* editor) {
	if (editor->cursorCol <= 0 && editor->cursorRow <= 0) return;
	if (editor->cursorCol <= 0) {
		editor->cursorRow--;
		editor->cursorCol = getCurrentLineLength(editor);
	}
	else {
		editor->cursorCol--;
	}
}

void moveCursorRight(Editor* editor) {

	if ((editor->cursorCol + 1) > getCurrentLineLength(editor)) {
		if (editor->cursorRow + 1 < editor->lineCount) {
			editor->cursorRow++;
			editor->cursorCol = 0;
		}
		else {
			return;
		}
	}
	else {
		editor->cursorCol++;
	}
}


void moveCursorUp(Editor* editor) {
	if (editor->cursorRow <= 0) return; // If you're at the top line, return.
	editor->cursorRow--;
	if (!isalnum(*getCharacterUnderCursor(editor))) {
		editor->cursorCol = getCurrentLineLength(editor);
	}
}


void moveCursorDown(Editor* editor) {
	if ((editor->cursorRow + 1) >= editor->lineCount) return; // If you're at the bottom line, return.
	size_t nextLineLength = strlen(getCurrentLine(editor)->next->text);
	if (editor->cursorCol > nextLineLength) {
		editor->cursorCol = (int)nextLineLength;
	}
	editor->cursorRow++;


}


static void insertNewLine(Editor* editor) {

	Line* newLine = createLine();
	Line* currentLine = getCurrentLine(editor);

	newLine->prev = currentLine;
	newLine->next = currentLine->next;

	currentLine->next = newLine;

	if (currentLine == editor->lastLine) {
		editor->lastLine = newLine;
	}

	memmove((void*)newLine->text,
		(void*)getCharacterUnderCursor(editor),
		strlen((char*)getCharacterUnderCursor(editor)) + 1);

	addTextAtCursor(editor, "\0");

	editor->lineCount++;

}


void carraigeReturn(Editor* editor) {

	if (getCurrentLine(editor) == editor->lastLine &&
		editor->cursorCol >= getCurrentLineLength(editor)) {
		appendLine(editor, createLine());
	}
	else {
		insertNewLine(editor);
	}

	editor->cursorRow++;
	editor->cursorCol = 0;
}

void createEditorFromFile(Editor* editor, char* buffer, const char* fileName) {
	loadFileIntoBuffer(fileName, buffer);
	while (*buffer != EOF) {
		Line* line = (Line*)malloc(sizeof(Line));
		int i = 0;
		for (; *buffer != '\r' && *buffer != '\n' && *buffer != EOF; ++buffer, ++i) {
			line->text[i] = *buffer;
		}
		line->text[i++] = '\0';
		while (*buffer == '\r' || *buffer == '\n') ++buffer;
		appendLine(editor, line);
	}
}

void appendLine(Editor* editor, Line* line) {
	if (editor->lastLine == NULL) {
		editor->lines = line;
		editor->lastLine = line;
		line->next = NULL;
		line->prev = NULL;
	}
	else {
		editor->lastLine->next = line;
		line->prev = editor->lastLine;
		editor->lastLine = line;
		line->next = NULL;
	}

	editor->lineCount++;
}

