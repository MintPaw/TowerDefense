#include "platform.h"
#include "renderer.h"

void update();
bool getKeyPressed(int key);

struct Player {
	float x;
	float y;
};

struct Game {
	Player player;
};

Game *game;

#ifdef _MSC_VER
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char **argv) {
#endif
	log("Main Init\n");

	game = NULL;

	initPlatform();
	initRenderer();
	platformUpdateLoop(update);

	return 0;
}

void update() {
	/// Section: Init Game
	if (!game) {
		game = (Game *)malloc(sizeof(Game));
		memset(game, 0, sizeof(Game));

		game->player.x = 100;
		game->player.y = 100;
	}

	/// Section: Update
	Player *player = &game->player;
	bool moveUp = false;
	bool moveDown = false;
	bool moveLeft = false;
	bool moveRight = false;
	if (getKeyPressed('W')) moveUp = true;
	if (getKeyPressed('S')) moveDown = true;
	if (getKeyPressed('A')) moveLeft = true;
	if (getKeyPressed('D')) moveRight = true;

	float moveSpeed = 5;
	Point playerMovePoint = {};
	if (moveUp) playerMovePoint.y -= moveSpeed;
	if (moveDown) playerMovePoint.y += moveSpeed;
	if (moveLeft) playerMovePoint.x -= moveSpeed;
	if (moveRight) playerMovePoint.x += moveSpeed;

	player->x += playerMovePoint.x;
	player->y += playerMovePoint.y;

	/// Section: Render
	clearRenderer();
	drawCircle(player->x, player->y, 100, 0x2200FF00);
	swapBuffers();
}

bool getKeyPressed(int key) {
	if (key > KEY_LIMIT) return false;

	if (platform->keys[key] == KEY_PRESSED) return true;
	if (platform->keys[key] == KEY_JUST_PRESSED) return true;

	return false;
}
