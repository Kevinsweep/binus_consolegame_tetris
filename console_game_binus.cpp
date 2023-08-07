// console_game_binus.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>


int nScreenWidth = 80;			// Console Screen Size X (columns)
int nScreenHeight = 30;			// Console Screen Size Y (rows)
wstring tetromino[7];
int nFieldWidth = 17;
int nFieldHeight = 24;
unsigned char* pField = nullptr;
char symbols[11] = " ABCDEFG=#";

int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0: // 0 degrees			// 0  1  2  3
		pi = py * 4 + px;			// 4  5  6  7
		break;						// 8  9 10 11
									//12 13 14 15

	case 1: // 90 degrees			//12  8  4  0
		pi = 12 + py - (px * 4);	//13  9  5  1
		break;						//14 10  6  2
									//15 11  7  3

	case 2: // 180 degrees			//15 14 13 12
		pi = 15 - (py * 4) - px;	//11 10  9  8
		break;						// 7  6  5  4
									// 3  2  1  0

	case 3: // 270 degrees			// 3  7 11 15
		pi = 3 - py + (px * 4);		// 2  6 10 14
		break;						// 1  5  9 13
	}								// 0  4  8 12

	return pi;
}

bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {

			// get index into piece
			int pi = Rotate(px, py, nRotation);

			//get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < nFieldWidth){
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0) {
						return false; // fail on first hit
					}
				}
			}
		}
	}

	return true;
}


int main()
{

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	pField = new unsigned char[nFieldHeight * nFieldWidth]; // to create playfield
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	//game logic
	bool bGameOver = false;
	int nCurrentPiece = rand() % 7;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHold = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;

	vector<int> vLines;

	int nScore = 0;

	while (!bGameOver) {
		//game timing ==================================================
		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		//user input ===================================================
		for (int k = 0; k < 4; k++)								// R   L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		//game logic ===================================================
		nCurrentX += (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		if (bKey[3]){
			if (!bRotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) {
				nCurrentRotation += 1;
				bRotateHold = true;
			}
			else {
				for (int i = 1; i <= 3; i++) {
					if (!bRotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX + i, nCurrentY)) {
						nCurrentX += 1;
						nCurrentRotation += 1;
						bRotateHold = true;
					}
					else if (!bRotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX - i, nCurrentY)) {
						nCurrentX -= 1;
						nCurrentRotation += 1;
						bRotateHold = true;
					}
				}
			}
		}
		else
			bRotateHold = false;

		if (bForceDown) {

			nSpeedCounter = 0;

			if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
			else {
				//lock piece
				for (int px = 0; px < 4; px++)
					for(int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
				
				//check for lines

				
				//choose next piece
				nCurrentPiece = rand() %7;
				nCurrentRotation = 0;
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;

				//if piece does not fit
				bGameOver = !doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

			}

		}


		//render output ================================================


		// Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L".ABCDEFG=#"[pField[y * nFieldWidth + x]];

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = 4;

		swprintf(&screen[1 * nScreenWidth - 20], 16, L"SCORE: %8d", nScore);


		WriteConsoleOutputCharacter(hConsole, screen , nScreenWidth * nScreenHeight, { 30,0 }, &dwBytesWritten);
	}


}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
