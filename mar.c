#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>

#pragma warning(disable:4996)

//colors
#define RED 12
#define BLUE 3
#define GREEN 10
#define YELLOW 14
#define GRAY 8
#define PINK 13
#define WHITE 15
#define WAIT_TIME_MILI_SEC 100
//directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
// general
#define BOARD_SIZE 40
#define INITIAL_SNAKE_LENGTH 3
#define MINIMUM_SNAKE_LENGTH 2
#define MAX_LEN_SNAKES 30
#define NUMBER_OF_MOUSES 20
#define N 2
//board_characters
#define EMPTY '0'
#define MOUSE 'm'
#define PLAYER1_SNAKE_HEAD '1'
#define PLAYER2_SNAKE_HEAD '2'
#define PLAYER1_SNAKE_BODY 'a'
#define PLAYER2_SNAKE_BODY 'b'
//Bernard, Poison and golden star
#define BERNARD_CLOCK 'c' //on the board character
#define GOLDEN_STAR '*' //on the board character
#define POISON 'x' //on the board character
#define NUMBER_OF_POISONS 5
#define NUMBER_OF_GOLDEN_STARS 3
#define BERNARD_CLOCK_APPEARANCE_CHANCE_PERCENT 20
#define BERNARD_CLOCK_APPEARANCE_CHECK_PERIOD_MILI_SEC 2000
#define BERNARD_CLOCK_FROZEN_TIME_MILI_SEC 4000

CONSOLE_FONT_INFOEX former_cfi;
CONSOLE_CURSOR_INFO former_info;
COORD former_screen_size;

void reset_console() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(consoleHandle, CONSOLE_WINDOWED_MODE, &former_screen_size);
	SetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
	SetConsoleCursorInfo(consoleHandle, &former_info);
}

void hidecursor(HANDLE consoleHandle)
{
	GetConsoleCursorInfo(consoleHandle, &former_info);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void set_console_font_and_font_size(HANDLE consoleHandle) {
	former_cfi.cbSize = sizeof(former_cfi);
	GetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 20;
	cfi.dwFontSize.Y = 20;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy(cfi.FaceName, L"Courier");
	SetCurrentConsoleFontEx(consoleHandle, FALSE, &cfi);
}

void set_full_screen_mode(HANDLE consoleHandle) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	former_screen_size.X = csbi.dwSize.X; former_screen_size.Y = csbi.dwSize.Y;
	COORD coord;
	SetConsoleDisplayMode(consoleHandle, CONSOLE_FULLSCREEN_MODE, &coord);
}

void init_screen()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	set_full_screen_mode(consoleHandle);
	hidecursor(consoleHandle);
	set_console_font_and_font_size(consoleHandle);

}

void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction) {
	DWORD64 start_time, check_time;
	start_time = GetTickCount64();
	check_time = start_time + WAIT_TIME_MILI_SEC; //GetTickCount returns time in miliseconds
	char key = 0;
	char player1_key_hit = 0;
	char player2_key_hit = 0;

	while (check_time > GetTickCount64()) {
		if (_kbhit()) {
			key = _getch();
			if (key == 0)
				key = _getch();
			if (key == 'w' || key == 'a' || key == 's' || key == 'd')
				player1_key_hit = key;
			if (key == 'i' || key == 'j' || key == 'k' || key == 'l')
				player2_key_hit = key;
		}
	}

	switch (player1_key_hit) {
	case 'w': if (*player1_snake_direction != DOWN) *player1_snake_direction = UP; break;
	case 'a': if (*player1_snake_direction != RIGHT) *player1_snake_direction = LEFT; break;
	case 's': if (*player1_snake_direction != UP) *player1_snake_direction = DOWN; break;
	case 'd': if (*player1_snake_direction != LEFT) *player1_snake_direction = RIGHT; break;
	default: break;
	}

	switch (player2_key_hit) {
	case 'i': if (*player2_snake_direction != DOWN) *player2_snake_direction = UP; break;
	case 'j': if (*player2_snake_direction != RIGHT) *player2_snake_direction = LEFT; break;
	case 'k': if (*player2_snake_direction != UP) *player2_snake_direction = DOWN; break;
	case 'l': if (*player2_snake_direction != LEFT) *player2_snake_direction = RIGHT; break;
	default: break;
	}
}

void draw_point(char point_content) {
	switch (point_content) {
	case PLAYER1_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED); printf("@"); break;
	case PLAYER2_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("@"); break;
	case PLAYER1_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED);  printf("o"); break;
	case PLAYER2_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("o"); break;
	case MOUSE: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); printf("m"); break;
	case GOLDEN_STAR: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW); printf("*"); break;
	case POISON: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN); printf("x"); break;
	case BERNARD_CLOCK: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), PINK); printf("c"); break;
	default: printf(" ");
	}
}

void draw_horizonatal_walls() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
	for (int i = 0; i < BOARD_SIZE + 2; ++i)
		printf("-");
	printf("\n");
}

void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]) {
	system("cls");
	draw_horizonatal_walls();
	for (int i = 0; i < BOARD_SIZE; i++) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		printf("|"); // vertical wall 
		for (int j = 0; j < BOARD_SIZE; j++)
			draw_point(board_content[i][j]);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		printf("|\n"); // vertical wall
	}
	draw_horizonatal_walls();
}

void position_mouses_function(char board_content[BOARD_SIZE][BOARD_SIZE], int position_mouses_arry[NUMBER_OF_MOUSES][N]) {
	int i, j, x = 0, y = 0;
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			if (board_content[i][j] == MOUSE) {
				position_mouses_arry[y][x] = j; /*dar zakhire haye makane in barname x hara samte chap arraye va y hara dar samte rast araye tarif mikonim*/
				position_mouses_arry[y][x + 1] = i;
				y++;
			}
		}
	}
}
void position_stars_function(char board_content[BOARD_SIZE][BOARD_SIZE], int position_stars_arry[NUMBER_OF_GOLDEN_STARS][N]) {
	int i, j, x = 0, y = 0;
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			if (board_content[i][j] == GOLDEN_STAR) {
				position_stars_arry[y][x] = j; /*dar zakhire haye makane in barname x hara samte chap arraye va y hara dar samte rast araye tarif mikonim*/
				position_stars_arry[y][x + 1] = i;
				y++;
			}
		}
	}
}
void body_position_calculator(int snake1_length, int position_snake[MAX_LEN_SNAKES][N]) {
	/*in tabe baraye jelo bordane mar estefade mishavad va makan mar haye jeloii ra dar yeki aghab tar mirizad*/
	int i, j;
	if (snake1_length == MAX_LEN_SNAKES) {
		for (i = 0; i < 2; i++) {
			for (j = snake1_length - 2; j >= 0; j--) {
				position_snake[j + 1][i] = position_snake[j][i];
			}
		}
	}
	else {
		for (i = 0; i < 2; i++) {
			for (j = snake1_length - 1; j >= 0; j--) {
				position_snake[j + 1][i] = position_snake[j][i];
			}
		}
	}
}


int result(int position_snake1[MAX_LEN_SNAKES][N], int position_snake2[MAX_LEN_SNAKES][N], int snake1_length, int snake2_length) {
	int i, who_wins = 3;
	if ((position_snake1[0][0] == position_snake2[0][0]) && (position_snake1[0][1] == position_snake2[0][1])) {
		/* choon har ozve 'position_snake' ya x makan ya y makan ra neshan midahad bayad baraye makan az do taye an estefade kard*/
		if (snake1_length == snake2_length) {
			who_wins = 0;
		}
		else if (snake1_length > snake2_length) {
			who_wins = 1;
		}
		else {
			who_wins = 2;
		}
	}
	for (i = 1; i < snake1_length; i++) {
		if ((position_snake1[i][0] == position_snake2[0][0]) && (position_snake1[i][1] == position_snake2[0][1])) {
			who_wins = 1;
			break;
		}
		if ((position_snake1[i][0] == position_snake1[0][0]) && (position_snake1[i][1] == position_snake1[0][1])) {
			who_wins = 2;
			break;
		}
	}
	for (i = 1; i < snake2_length; i++) {
		if ((position_snake2[i][0] == position_snake1[0][0]) && (position_snake2[i][1] == position_snake1[0][1])) {
			who_wins = 2;
			break;
		}
		if ((position_snake2[i][0] == position_snake2[0][0]) && (position_snake2[i][1] == position_snake2[0][1])) {
			who_wins = 1;
			break;
		}
	}
	return(who_wins);
}


// prototypes
void init_screen();
void reset_console();
void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction);
void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]);
void position_mouses_function(char board_content[BOARD_SIZE][BOARD_SIZE], int position_mouses_arry[NUMBER_OF_MOUSES][N]);
void body_position_calculator(int snake1_length, int position_snake[MAX_LEN_SNAKES][N]);
int result(int position_snake1[MAX_LEN_SNAKES][N], int position_snake2[MAX_LEN_SNAKES][N], int snake1_length, int snake2_length);
int check_mouse_and_star(int position_snake[MAX_LEN_SNAKES][N], int position_mouses_arry[NUMBER_OF_MOUSES][N], char board_content[BOARD_SIZE][BOARD_SIZE], int position_stars_arry[NUMBER_OF_GOLDEN_STARS][N], int snake_length1, int snake_length2);

int main() {
	int i = 0, j = 0, x = 0, y = 0;
	int snake1_length = 3;
	int snake2_length = 3;
	int position_snake1[MAX_LEN_SNAKES][N] = { { 0,2 },{ 0,1 },{ 0,0 } };/*baraye zakhire va taghiir x va y marhast*/
	int position_snake2[MAX_LEN_SNAKES][N] = { { 39,37 },{ 39,38 },{ 39,39 } };
	int position_mouses_arry[NUMBER_OF_MOUSES][N];
	int position_stars_arry[NUMBER_OF_GOLDEN_STARS][N];
	char board_content[BOARD_SIZE][BOARD_SIZE];
	int player1_snake_direction, player2_snake_direction;/*baraye zakhire va taghiir jahat*/
	player1_snake_direction = DOWN;
	player2_snake_direction = UP;
	printf("Welcome to domar game!\nplayer1 is red and it's keys are 'w' 'a' 's' 'd'\nplayer 2 is blue and it's keys are 'i' 'j' 'k' 'l'\npress a key to start game.");
	getch(); /*dar kole in code 'getch()' baraye in ast ke barname ghabl az aghaz ya edame ya etmam montazere karbar shavad ta kelidi ra bezanad*/
	init_screen();
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			board_content[i][j] = EMPTY;
		}
	}
	board_content[0][0] = PLAYER1_SNAKE_BODY, board_content[1][0] = PLAYER1_SNAKE_BODY, board_content[2][0] = PLAYER1_SNAKE_HEAD;
	board_content[39][39] = PLAYER2_SNAKE_BODY, board_content[38][39] = PLAYER2_SNAKE_BODY, board_content[37][39] = PLAYER2_SNAKE_HEAD;
	for (i = 0; i < NUMBER_OF_MOUSES; i++) {  /*inkar baraye taiin moosh haye tasadofi ast*/
		x = rand() % BOARD_SIZE;
		y = rand() % BOARD_SIZE;
		if (board_content[y][x] == EMPTY) {
			board_content[y][x] = MOUSE;
		}
		else {
			i--;
		}
	}
	for (i = 0; i < NUMBER_OF_GOLDEN_STARS; i++) {
		x = rand() % BOARD_SIZE;
		y = rand() % BOARD_SIZE;
		if (board_content[y][x] == EMPTY) {
			board_content[y][x] = GOLDEN_STAR;
		}
		else {
			i--;
		}
	}
	position_mouses_function(board_content, position_mouses_arry);/*baraye tashkhise makan mosh haye tasadofi ast*/
	position_stars_function(board_content, position_stars_arry);/*baraye tashkhise makan setare haye tasadofi ast */
	while (TRUE) { /*hadafe aslie in 'while' harekate mar hast*/
		draw_board(board_content);
		int check_player1 = player1_snake_direction;
		int check_player2 = player2_snake_direction;
		wait_and_get_direction(&player1_snake_direction, &player2_snake_direction);
		body_position_calculator(snake1_length, position_snake1);
		if (check_player1 == DOWN) {/*ba estefade az 'check_player' ta ghabl az taghiir jahat tavasote mar be harekate khod edame midahad*/
			if (player1_snake_direction == LEFT) {
				if (position_snake1[0][0] == 0) { /* dar hameye in if haii ke makane sare mar ba ozve akhar ya avale  x ya y safhe(board_content) barabar ast  mar ra az tarafe digar edame midahim*/
					position_snake1[0][0] = 39;
				}
				else
					position_snake1[0][0]--;
			}
			else if (player1_snake_direction == RIGHT) {
				if (position_snake1[0][0] == 39) {
					position_snake1[0][0] = 0;
				}
				else
					position_snake1[0][0]++;
			}
			else {
				if (position_snake1[0][1] == 39) {
					position_snake1[0][1] = 0;
				}
				else
					position_snake1[0][1]++;
			}
		}
		if (check_player1 == UP) { /*hadafe digare 'check_player' in ast ke mar natavanad yoho be jahate mokhalef beravad*/
			if (player1_snake_direction == RIGHT) {
				if (position_snake1[0][0] == 39) {
					position_snake1[0][0] = 0;
				}
				else
					position_snake1[0][0]++;
			}
			else if (player1_snake_direction == LEFT) {
				if (position_snake1[0][0] == 0) {
					position_snake1[0][0] = 39;
				}
				else
					position_snake1[0][0]--;
			}
			else {
				if (position_snake1[0][1] == 0) {
					position_snake1[0][1] = 39;
				}
				else
					position_snake1[0][1]--;
			}
		}
		if (check_player1 == LEFT) {
			if (player1_snake_direction == UP) {
				if (position_snake1[0][1] == 0) {
					position_snake1[0][1] = 39;
				}
				else
					position_snake1[0][1]--;
			}
			else if (player1_snake_direction == DOWN) {
				if (position_snake1[0][1] == 39) {
					position_snake1[0][1] = 0;
				}
				else
					position_snake1[0][1]++;
			}
			else {
				if (position_snake1[0][0] == 0) {
					position_snake1[0][0] = 39;
				}
				else
					position_snake1[0][0]--;
			}
		}
		if (check_player1 == RIGHT) {
			if (player1_snake_direction == UP) {
				if (position_snake1[0][1] == 0) {
					position_snake1[0][1] = 39;
				}
				else
					position_snake1[0][1]--;
			}
			else if (player1_snake_direction == DOWN) {
				if (position_snake1[0][1] == 39) {
					position_snake1[0][1] = 0;
				}
				else
					position_snake1[0][1]++;
			}
			else {
				if (position_snake1[0][0] == 39) {
					position_snake1[0][0] = 0;
				}
				else
					position_snake1[0][0]++;
			}
		}
		int  check = -1;
		for (i = 0; i < NUMBER_OF_MOUSES; i++) {
			if (position_snake1[0][0] == position_mouses_arry[i][0] && position_snake1[0][1] == position_mouses_arry[i][1]) {
				for (j = 0; j < 1; j++) {
					x = rand() % BOARD_SIZE; /*inkar baraye tolide mooshe jadid bejaye mooshe khorde shode ast*/
					y = rand() % BOARD_SIZE;
					if (board_content[y][x] == EMPTY) {
						board_content[y][x] = MOUSE;
						position_mouses_arry[i][0] = x;
						position_mouses_arry[i][1] = y;
						break;
					}
					else {
						j--;
					}
				}
				if (snake1_length < MAX_LEN_SNAKES) {
					snake1_length++;
					check = 0;
					break;
				}
				else {
					board_content[position_snake1[snake1_length][1]][position_snake1[snake1_length][0]] = EMPTY;
					/*choon ma faghat makanhaye jadid ra dar 'board_contect' migozarim makane dome mar baghi mimanad ke baese afzaieshe toole mar mishavad*/
					check = 0;
					break;
				}
			}
		}
		for (i = 0; i < NUMBER_OF_GOLDEN_STARS; i++) {
			if (position_snake1[0][0] == position_stars_arry[i][0] && position_snake1[0][1] == position_stars_arry[i][1]) {
				for (j = 0; j < 1; j++) {
					x = rand() % BOARD_SIZE;
					y = rand() % BOARD_SIZE;
					if (board_content[y][x] == EMPTY) {
						board_content[y][x] = GOLDEN_STAR;
						position_stars_arry[i][0] = x;
						position_stars_arry[i][1] = y;
						break;
					}
					else {
						j--;
					}
				}
				if (snake2_length > N) {
					board_content[position_snake2[snake2_length - 1][1]][position_snake2[snake2_length - 1][0]] = EMPTY;
					snake2_length--;
					board_content[position_snake1[snake1_length][1]][position_snake1[snake1_length][0]] = EMPTY;
					check = 0;
					break;
				}
				else {
					check = 1;
					break;
				}
			}
		}
		if (check == -1) {
			board_content[position_snake1[snake1_length][1]][position_snake1[snake1_length][0]] = EMPTY;
		}
		else if (check == 1) {
			printf("player 1 won!\nsee you!\n");
			getch();
			break;
		}
		body_position_calculator(snake2_length, position_snake2);/*daghighan haman cod hara baraye mare dovom tekrar mikonim*/
		if (check_player2 == DOWN) {
			if (player2_snake_direction == LEFT) {
				if (position_snake2[0][0] == 0) {
					position_snake2[0][0] = 39;
				}
				else
					position_snake2[0][0]--;
			}
			else if (player2_snake_direction == RIGHT) {
				if (position_snake2[0][0] == 39) {
					position_snake2[0][0] = 0;
				}
				else
					position_snake2[0][0]++;
			}
			else {
				if (position_snake2[0][1] == 39) {
					position_snake2[0][1] = 0;
				}
				else
					position_snake2[0][1]++;
			}
		}
		if (check_player2 == UP) {
			if (player2_snake_direction == RIGHT) {
				if (position_snake2[0][0] == 39) {
					position_snake2[0][0] = 0;
				}
				else
					position_snake2[0][0]++;
			}
			else if (player2_snake_direction == LEFT) {
				if (position_snake2[0][0] == 0) {
					position_snake2[0][0] = 39;
				}
				else
					position_snake2[0][0]--;
			}
			else {
				if (position_snake2[0][1] == 0) {
					position_snake2[0][1] = 39;
				}
				else
					position_snake2[0][1]--;
			}
		}
		if (check_player2 == LEFT) {
			if (player2_snake_direction == UP) {
				if (position_snake2[0][1] == 0) {
					position_snake2[0][1] = 39;
				}
				else
					position_snake2[0][1]--;
			}
			else if (player2_snake_direction == DOWN) {
				if (position_snake2[0][1] == 39) {
					position_snake2[0][1] = 0;
				}
				else
					position_snake2[0][1]++;
			}
			else {
				if (position_snake2[0][0] == 0) {
					position_snake2[0][0] = 39;
				}
				else
					position_snake2[0][0]--;
			}
		}
		if (check_player2 == RIGHT) {
			if (player2_snake_direction == UP) {
				if (position_snake2[0][1] == 0) {
					position_snake2[0][1] = 39;
				}
				else
					position_snake2[0][1]--;
			}
			else if (player2_snake_direction == DOWN) {
				if (position_snake2[0][1] == 39) {
					position_snake2[0][1] = 0;
				}
				else
					position_snake2[0][1]++;
			}
			else {
				if (position_snake2[0][0] == 39) {
					position_snake2[0][0] = 0;
				}
				else
					position_snake2[0][0]++;
			}
		}
		for (i = 1; i < snake1_length; i++) {
			board_content[position_snake1[i][1]][position_snake1[i][0]] = PLAYER1_SNAKE_BODY;
		}
		board_content[position_snake1[0][1]][position_snake1[0][0]] = PLAYER1_SNAKE_HEAD;

		for (i = 1; i < snake2_length; i++) {
			board_content[position_snake2[i][1]][position_snake2[i][0]] = PLAYER2_SNAKE_BODY;
		}
		board_content[position_snake2[0][1]][position_snake2[0][0]] = PLAYER2_SNAKE_HEAD;
		check = -1; /*hamchenan haman karhaye marboot be mare aval ra tekrar mikinim*/
		for (i = 0; i < NUMBER_OF_MOUSES; i++) {
			if (position_snake2[0][0] == position_mouses_arry[i][0] && position_snake2[0][1] == position_mouses_arry[i][1]) {
				for (j = 0; j < 1; j++) {
					x = rand() % BOARD_SIZE;
					y = rand() % BOARD_SIZE;
					if (board_content[y][x] == EMPTY) {
						board_content[y][x] = MOUSE;
						position_mouses_arry[i][0] = x;
						position_mouses_arry[i][1] = y;
						break;
					}
					else {
						j--;
					}
				}
				if (snake2_length < MAX_LEN_SNAKES) {
					snake2_length++;
					check = 0;
					break;
				}
				else {
					board_content[position_snake2[snake2_length][1]][position_snake2[snake2_length][0]] = EMPTY;
					check = 0;
					break;
				}
			}
		}
		for (i = 0; i < NUMBER_OF_GOLDEN_STARS; i++) {

			if (position_snake2[0][0] == position_stars_arry[i][0] && position_snake2[0][1] == position_stars_arry[i][1]) {
				for (j = 0; j < 1; j++) {
					x = rand() % BOARD_SIZE;
					y = rand() % BOARD_SIZE;
					if (board_content[y][x] == EMPTY) {
						board_content[y][x] = GOLDEN_STAR;
						position_stars_arry[i][0] = x;
						position_stars_arry[i][1] = y;
						break;
					}
					else {
						j--;
					}
				}
				if (snake1_length > N) {
					board_content[position_snake1[snake1_length - 1][1]][position_snake1[snake1_length - 1][0]] = EMPTY;
					snake1_length--;
					board_content[position_snake2[snake2_length][1]][position_snake2[snake2_length][0]] = EMPTY;
					check = 0;
					break;
				}
				else {
					check = 1;
					break;
				}
			}
		}
		if (check == -1) { /*in halati ast ke sar mar be khane ye EMPTY beresad*/

			board_content[position_snake2[snake2_length][1]][position_snake2[snake2_length][0]] = EMPTY;
		}
		else if (check == 1) {
			printf("player 2 won!\nsee you!\n"); /* deghat shavad agar bad az '\n' space bezanim va sepas edame harfeman ra benevisim neveshte ha dar yek khat nemibashand*/
			getch();
			break;
		}
		result(position_snake1, position_snake2, snake1_length, snake2_length);
		if (result(position_snake1, position_snake2, snake1_length, snake2_length) == 0) {
			printf("The game equalised\nsee you!\n");
			getch();
			break;
		}
		else if (result(position_snake1, position_snake2, snake1_length, snake2_length) == 1) {
			printf("player 1 won!\nsee you!\n");
			getch();/*in kar baraye in ast ke shayad yek lahze bad az etmame bazi  karbaran kelidy ra bezanad va motavajeh etmam nashavand va ingoone jeloye baste shodan nakhasteye game ra migirim*/
			break;
		}
		else if (result(position_snake1, position_snake2, snake1_length, snake2_length) == 2) {
			printf("player 2 won!\nsee you!\n");
			getch();
			break;
		}
		else {
			continue;/*agar hichkodam az sharthaye etmam bargharar nabood in halghe edame miabad*/
		}
	}
	return 0;
}