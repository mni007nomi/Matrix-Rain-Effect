#include<Windows.h>
#include<chrono>
#include<vector>
#include<string>

using namespace std;

int screenWidth = 180;
int screenHeight = 50;

int TOTAL_RAIN_DROPS = 180;

CHAR_INFO* screen;
SMALL_RECT rect;
COORD bufferSize;
HANDLE hConsole;
DWORD dwBytesWritten = 0;

#define BLACK 0
#define WHITE 15
#define GRAY 8
#define GREEN 2
#define LIGHT_GREEN 10

struct MatrixRainDrop {
	int xPosition;
	float yPosition;
	float dropSpeed;
	wstring text;
};

vector<MatrixRainDrop> drops;

void initializeRainDrop(MatrixRainDrop* drop) {
	float minDropSpeed = 5.0f;
	int rangeDropSpeed = 40;
	int minTextLength = 5;
	int rangeTextLength = 20;
	int minASCIICharacter = 0x30A0; //Japanese Katakana Characters
	int rangeASCIICharacter = 0x30FF - 0x30A0; //Katakana Characters Range

	int currentTextLength = minTextLength + rand() % rangeTextLength;

	drop->xPosition = rand() % screenWidth;
	drop->yPosition = 0.0f;

	drop->dropSpeed = minDropSpeed + rand() % 40;

	for (int i = 0; i < currentTextLength; i++) {
		wchar_t randomCharacter = (wchar_t)minASCIICharacter + rand() % rangeASCIICharacter;
		drop->text.append(1, randomCharacter);
	}

	//drop->text = L"0123456789";
}

void createScreenBuffer() {
	screen = new CHAR_INFO[screenWidth * screenHeight];
	rect = { 0, 0, (SHORT)(screenWidth - 1), (SHORT)(screenHeight - 1) };
	bufferSize = { (SHORT)screenWidth, (SHORT)screenHeight };

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
}

void drawCharacterOnScreen(int x, int y, wchar_t character, int color = WHITE) {
	int index = y * screenWidth + x;
	screen[index].Char.UnicodeChar = character;
	screen[index].Attributes = color;
}

void clearScreen() {
	for (int i = 0; i < screenWidth * screenHeight; i++) {
		screen[i].Char.UnicodeChar = L' ';
		screen[i].Attributes = BLACK;
	}
}

void initializeMatrix() {
	for (int i = 0; i < TOTAL_RAIN_DROPS; i++) {
		MatrixRainDrop drop;
		initializeRainDrop(&drop);
		drops.push_back(drop);
	}
}

void update() {
	auto startTime = chrono::system_clock::now();

	while (true) {
		//Calculate Elapsed Time between Frames
		auto timeStamp = chrono::system_clock::now();
		chrono::duration<float> timeDifference = timeStamp - startTime;
		startTime = timeStamp;
		float elapsedTime = timeDifference.count();

		clearScreen();
		//Display Logic
		for (auto& drop : drops) {
			drop.yPosition += drop.dropSpeed * elapsedTime;

			if (drop.yPosition - drop.text.size() > screenHeight)
				initializeRainDrop(&drop);

			for (int i = 0; i < drop.text.size(); i++) {
				int y = (int)drop.yPosition - i;
				if (y >= 0 && y < screenHeight) {
					int characterIndex = abs(i - (int)drop.yPosition) % drop.text.size();
					int color = WHITE;
					if (i == 0) color = WHITE;
					else if (i > 0 && i <= 2) color = GRAY;
					else if (i > 2 && drop.dropSpeed > 20.0f) color = GREEN;
					else if (i > 2 && drop.dropSpeed <= 20.0f) color = LIGHT_GREEN;
					else color = GREEN;

					drawCharacterOnScreen(drop.xPosition, y, drop.text[characterIndex], color);
				}
			}
		}
		//Write the Console Data Once Per Frame
		WriteConsoleOutput(hConsole, screen, bufferSize, { 0, 0 }, &rect);
		//Display FPS (Optional)
		float fFPS = 1.0f / elapsedTime;
		wstring title = L"Matrix Rain | FPS: " + to_wstring((int)fFPS);
		SetConsoleTitle(title.c_str());
	}
}

int main() {
	createScreenBuffer();
	initializeMatrix();
	update();
	return 0;
}