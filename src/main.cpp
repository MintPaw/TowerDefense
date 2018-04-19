#include "platform.h"
#include "renderer.h"

void update();
bool getKeyPressed(int key);

struct Frame {
	char *name;
	int x;
	int y;
	int width;
	int height;
	int sourceWidth;
	int sourceHeight;
	bool rotated;
};

struct Player {
	float x;
	float y;
};

struct Game {
	Texture *playerTexture;

	tinytiled_map_t *tiledMap; 
	Texture *tilesetTexture;
	Texture *mapTexture;

	Player player;

	Frame *spriteFrames;
	int spriteFramesNum;
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

		game->playerTexture = uploadPngTexturePath("assets/sprites/player.png");

		{ /// Setup map
			game->tilesetTexture = uploadPngTexturePath("assets/tilesets/tileset.png");

			void *mapData;
			long mapSize = readFile("assets/maps/map.json", &mapData);
			game->tiledMap = tinytiled_load_map_from_memory(mapData, mapSize, 0);

			int w = game->tiledMap->width;
			int h = game->tiledMap->height;

			game->mapTexture = uploadTexture(NULL, game->tiledMap->width * game->tiledMap->tilewidth, game->tiledMap->height * game->tiledMap->tileheight);

			tinytiled_layer_t *layer = game->tiledMap->layers;
			while (layer) {
				int *data = layer->data;
				int dataNum = layer->data_count;

				for (int i = 0; i < dataNum; i++) {
					// printf("Data: %d\n", data[i]);
				}
				drawTiles(game->tilesetTexture, game->mapTexture, game->tiledMap->tilewidth, game->tiledMap->tileheight, game->tiledMap->width, game->tiledMap->height, data);
				layer = layer->next;
			}
		}

		{ /// Parse frames
#if 0
			void *jsonData;
			long jsonSize = readFile("assets/sprites.json", &jsonData);
			json_value_s *json = json_parse(jsonData, jsonSize);
			free(jsonData);

			json_object_s *rootObject = (json_object_s *)json->payload;
			game->spriteFrames = (Frame *)malloc(sizeof(Frame) * rootObject->length);

			json_object_element_s *curEle = rootObject->start;
			while (curEle) {
				Frame *frame = &game->spriteFrames[game->spriteFramesNum++];
				frame->name = stringClone(curEle->name->string);

				json_object_element_s *spriteEle = ((json_object_s *)curEle->value->payload)->start;
				while (spriteEle) {
					// printf("Sprite prop name is: %s\n", spriteEle->name->string);

					json_object_s *posObject = (json_object_s *)spriteEle->value->payload;
					if (posObject) {
						json_object_element_s *posEle = posObject->start;
						while (posEle) {
							const char *numStr = ((json_number_s *)posEle->value->payload)->number;
							int num = atoi(numStr);

							if (streq("x", posEle->name->string)) frame->x = num;
							if (streq("y", posEle->name->string)) frame->y = num;

							if (streq("frame", spriteEle->name->string)) {
								if (streq("width", posEle->name->string)) frame->width = num;
								if (streq("height", posEle->name->string)) frame->height = num;
							} else if (streq("sourceSize", spriteEle->name->string)) {
								if (streq("width", posEle->name->string)) frame->sourceWidth = num;
								if (streq("height", posEle->name->string)) frame->sourceHeight = num;
							}

							posEle = posEle->next;
						}
					}

					if (streq("rotated", spriteEle->name->string)) {
						if (spriteEle->value->type == json_type_true) frame->rotated = true;
						if (spriteEle->value->type == json_type_false) frame->rotated = false;
					}

					spriteEle = spriteEle->next;
				}

				curEle = curEle->next;
			}

			for (int i = 0; i < game->spriteFramesNum; i++) {
				Frame *frame = &game->spriteFrames[i];
				// printf("Frame is: %s %d, %d, %d, %d (%d, %d)\n", frame->name, frame->x, frame->y, frame->width, frame->height, frame->sourceWidth, frame->sourceHeight);
			}
#endif
		}
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
	// setRendererCameraPosition(

	{ /// Draw map
		SpriteDef def;
		defaultSpriteDef(&def);
		def.tex = game->mapTexture;
		drawSpriteEx(&def);
	}

	{ /// Draw player
		SpriteDef def;
		defaultSpriteDef(&def);
		def.tex = game->playerTexture;
		def.pos.x = player->x;
		def.pos.y = player->y;
		drawSpriteEx(&def);
	}
	swapBuffers();
}

bool getKeyPressed(int key) {
	if (key > KEY_LIMIT) return false;

	if (platform->keys[key] == KEY_PRESSED) return true;
	if (platform->keys[key] == KEY_JUST_PRESSED) return true;

	return false;
}
