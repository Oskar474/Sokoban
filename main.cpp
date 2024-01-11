#include <ncursesw/curses.h>
//#include <ncurses.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>

void ncursesConfig() {
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);
    noecho();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); //wall
    init_pair(2, COLOR_WHITE, COLOR_BLACK); //empty
    init_pair(3, COLOR_RED, COLOR_BLACK); //target
    init_pair(4, COLOR_CYAN, COLOR_BLACK); //win
    init_pair(5, COLOR_YELLOW, COLOR_BLACK); //box
    init_pair(6, COLOR_WHITE, COLOR_BLACK); //default
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK); //player
}

namespace symbols {
    const char playerSymbol = 'P';
    const char boxSymbol = 'B';
    const char targetSymbol = 'T';
    const char emptySymbol = '.';
    const char wallSymbol = '#';
    const char winSymbol = '!';
}
using namespace symbols;
int ch;

class GameData {
private:
    int winCondition = 0;
    int winCounter = 0;
    int level = 5;
    int tryCounter = 1;
    int lastLevel = 5;
public:
	int getLastLevel(){
		return lastLevel;
	}
    void setWinCondition(int win) {
        winCondition = win;
    }

    void setWinCounter(int count) {
        winCounter = count;
    }

    void incrementLevel() {
        level++;
    }

    void setTryCounter(int count) {
        tryCounter = count;
    }

    int getWinCondition() {
        return winCondition;
    }

    int getWinCounter() {
        return winCounter;
    }

    int getLevel() {
        return level;
    }

    int getTryCounter() {
        return tryCounter;
    }

};

class Board {
private:
    int rows;
    int cols;
    char board[32][32];
    char sample[32][32];
    GameData &gameData;

    void printInColour(char symbol) {
        int colourNumber;
        switch (symbol) {
            case wallSymbol:
                attron(COLOR_PAIR(1));
                colourNumber = 1;
                break;
            case emptySymbol:
                attron(COLOR_PAIR(2));
                colourNumber = 2;
                break;
            case targetSymbol:
                attron(COLOR_PAIR(3));
                colourNumber = 3;
                break;
            case winSymbol:
                attron(COLOR_PAIR(4));
                colourNumber = 4;
                break;
            case boxSymbol:
                attron(COLOR_PAIR(5));
                colourNumber = 5;
                break;
            case playerSymbol:
                attron(COLOR_PAIR(7));
                colourNumber = 7;
                break;
            default:
                attron(COLOR_PAIR(6));
                colourNumber = 6;
                break;
        }
        printw("%c ", symbol);
        attroff(colourNumber);
    }

public:

    Board(GameData &GameData) : gameData(GameData) {}

    char (&getBoard())[32][32] {
        return board;
    }

    int getRows() {
        return rows;
    }

    int getCols() {
        return cols;
    }

    void setRows(int row) {
        rows = row;
    }

    void setCols(int col) {
        cols = col;
    }

    void setBoard(int i, int o, char symbol) {
        board[i][o] = symbol;
    }

    void setSample(int i, int o, char symbol) {
        sample[i][o] = symbol;
    }


    void printBoard() {
        clear();
        printw("%c",'\n');
        for (int i = 0; i < rows; ++i) {
        	printw("%c ",  ' ');
            for (int j = 0; j < cols; ++j) {
                printInColour(board[i][j]);
            }
            attron(COLOR_PAIR(6));
            if (i == 2) {
                printw("     level: ");
                printw("%d", gameData.getLevel());
            }
            if (i == 3) {
                printw("     try: ");
                printw("%d", gameData.getTryCounter());
            }
            if (i == 4) {
                if (gameData.getTryCounter() >= 10 && gameData.getTryCounter() < 20)
                    printw("     Are you playing without a screen? ");
                if (gameData.getTryCounter() >= 20)
                    printw("     I'm tired, boss... ");
            }
            printw("\n");
            attroff(COLOR_PAIR(6));
            refresh();
        }
    }

    void printTargets() {
        for (int i = 0; i < rows; ++i)
            for (int o = 0; o < cols; ++o)
                if (sample[i][o] == targetSymbol && board[i][o] != playerSymbol && board[i][o] != winSymbol)
                    board[i][o] = sample[i][o];
    }

    void checkWin() {
        int tempCounter = 0;
        int winOld = 0;
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j) {
                if (board[i][j] == winSymbol)
                    tempCounter++;
            }
        if (tempCounter != winOld) {
            gameData.setWinCounter(tempCounter);
            winOld = tempCounter;
        }
        if (gameData.getWinCounter() == gameData.getWinCondition()) {
            gameData.incrementLevel();
            gameData.setWinCounter(0);
        }
    }
};


class Player {
private:
    int playerPositionY = 0;
    int playerPositionX = 0;
    Board &gameBoard;

    void moveBox(int X, int Y, int &newPositionY, int &newPositionX) {
        if (gameBoard.getBoard()[newPositionY + Y][newPositionX + X] == targetSymbol)
            gameBoard.getBoard()[newPositionY + Y][newPositionX + X] = winSymbol;
        else if (gameBoard.getBoard()[newPositionY + Y][newPositionX + X] != wallSymbol &&
                 gameBoard.getBoard()[newPositionY + Y][newPositionX + X] != boxSymbol &&
                 gameBoard.getBoard()[newPositionY + Y][newPositionX + X] != winSymbol)
            gameBoard.getBoard()[newPositionY + Y][newPositionX + X] = boxSymbol;
        else
            switch (X) {
                case -1:
                    newPositionX++;
                    break;
                case 0:
                    if (Y == 1)
                        newPositionY--;
                    else
                        newPositionY++;
                    break;
                case 1:
                    newPositionX--;
                    break;
            }
    }

    void finalizeMovement(int newPositionY, int newPositionX) {
        gameBoard.getBoard()[playerPositionY][playerPositionX] = emptySymbol;
        playerPositionY = newPositionY;
        playerPositionX = newPositionX;
        gameBoard.getBoard()[newPositionY][newPositionX] = playerSymbol;
        gameBoard.printTargets();
    }

    bool isWall(int newPositionY, int newPositionX) {
        if (gameBoard.getBoard()[newPositionY][newPositionX] != '#')
            return false;
        return true;
    }

    void movePlayer(int newPositionY, int newPositionX) {

        if (!isWall(newPositionY, newPositionX)) {
            if (gameBoard.getBoard()[newPositionY][newPositionX] == boxSymbol or
                gameBoard.getBoard()[newPositionY][newPositionX] == winSymbol) {

                //down
                if (playerPositionY < newPositionY)
                    moveBox(0, 1, newPositionY, newPositionX);

                //up
                if (playerPositionY > newPositionY)
                    moveBox(0, -1, newPositionY, newPositionX);

                //left
                if (playerPositionX > newPositionX)
                    moveBox(-1, 0, newPositionY, newPositionX);

                //right
                if (playerPositionX < newPositionX)
                    moveBox(1, 0, newPositionY, newPositionX);
            }
            finalizeMovement(newPositionY, newPositionX);
        }
    }

public:
    Player(Board &board) : gameBoard(board) {
    }

    void setplayerPositionY(int Y) {
        playerPositionY = Y;
    }

    void setplayerPositionX(int X) {
        playerPositionX = X;
    }

    void moveUp() {
        movePlayer(playerPositionY - 1, playerPositionX);
    }

    void moveDown() {
        movePlayer(playerPositionY + 1, playerPositionX);
    }

    void moveRight() {
        movePlayer(playerPositionY, playerPositionX + 1);
    }

    void moveLeft() {
        movePlayer(playerPositionY, playerPositionX - 1);
    }

};

class MessagePrinter {
private:
    GameData &gameData;

public:
    MessagePrinter(GameData &gameData) : gameData(gameData) {}

    void printer() {
        clear();

        if (gameData.getLevel() + 1 >= gameData.getLastLevel()) {
            printw("\n\n\n\n\n      You Won!");
            printw("\n\n\n\n\n  Press q to quit...");
            while ((ch = getch()) != 'q') {
            }
            exit(0);
        }
        printw("\n\n\n\n\n          Good Job!");
        printw("\n\n\n\n  Press Enter to continue...");
    }
};


class fileHandling {
private:
    char board2[32][32];
    int board2sizeX = 0;
    int board2sizeY = 0;
    bool isCounted = false;

    Board &gameBoard;
    GameData &gameData;
    Player &player;

    void loadBoard(char boardx[32][32]) {
        gameBoard.setRows(board2sizeX);
        gameBoard.setCols(board2sizeY);
        for (int i = 0; i <= gameBoard.getRows(); i++)
            for (int o = 0; o <= gameBoard.getCols(); o++) {
                if (boardx[i][o] == targetSymbol && !isCounted)
                    gameData.setWinCondition(gameData.getWinCondition() + 1);
                if (boardx[i][o] == playerSymbol) {
                    player.setplayerPositionX(o);
                    player.setplayerPositionY(i);
                    gameBoard.setSample(i, o, emptySymbol);
                    gameBoard.setBoard(i, o, playerSymbol);
                } else {
                    gameBoard.setBoard(i, o, boardx[i][o]);
                    gameBoard.setSample(i, o, boardx[i][o]);
                }
            }
        isCounted = true;
    }

    void getMap(std::string name) {

        std::string X = "";
        std::string Y = "";
        char testline;
        std::ifstream file(name);

        while (file >> testline)
            if (testline == 'C') {
                while (testline != '-') {
                    file >> testline;
                    X += testline;
                }
                board2sizeX = std::stoi(X);
                while (testline != 'C') {
                    file >> testline;
                    Y += testline;
                }
                board2sizeY = std::stoi(Y);
            } else
                continue;

        char current[board2sizeX][board2sizeY];

        file.close();
        file.open(name);

        if (!file) {
            return;
        }

        int x = 0, y = 0;

        while (file >> testline) {
            current[x][y] = testline;
            y++;
            if (testline == ' ')
                x++;

            for (int i = 0; i < 32; i++)
                for (int j = 0; j < 32; j++)
                    board2[i][j] = current[i][j];
        }
        file.close();
    }

public:

    fileHandling(Board &board, GameData &gameData, Player &player) : gameBoard(board), gameData(gameData), player(player) {}


    void loader() {
        isCounted = false;
        gameData.setWinCondition(0);
        gameData.setWinCounter(0);

        std::string temp = std::to_string(gameData.getLevel());
        std::string fileName = "map" + temp + ".txt";

        getMap(fileName);
        loadBoard(board2);
    }
};

int main() {
    WINDOW *mainwin = initscr();
    ncursesConfig();
    GameData gameData;
    Board boardObj(gameData);
    MessagePrinter printer(gameData);
    Player player(boardObj);
    fileHandling map(boardObj, gameData, player);
    map.loader();
    int levOld;

    while ((ch = getch()) != 'q') {
        levOld = gameData.getLevel();
        switch (ch) {
            case 'r':
                map.loader();
                gameData.setTryCounter(gameData.getTryCounter() + 1);
                break;
            case KEY_UP:
                player.moveUp();
                break;
            case KEY_DOWN:
                player.moveDown();
                break;
            case KEY_LEFT:
                player.moveLeft();
                break;
            case KEY_RIGHT:
                player.moveRight();
                break;
        }
        boardObj.printBoard();
        boardObj.checkWin();
        if (levOld != gameData.getLevel()) {
            printer.printer();
            while ((ch = getch()) != 10);
            gameData.setTryCounter(1);
            map.loader();
        }
    }
    endwin();
    return 0;
}
