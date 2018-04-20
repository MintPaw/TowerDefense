/*
TODO:
*/
#include "platform.h"
#include "renderer.h"

#define TURRETS_MAX 1024

enum TurretType { TURRET_BASIC };
enum InvType { INV_START, INV_HANDS, INV_TURRET_BASIC, INV_END };

struct Turret {
	bool exists;
	TurretType type;
	Texture *baseTex;
	Texture *gunTex;
	float gunRotation;
	int x;
	int y;
};

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
	Player player;

	Frame *spriteFrames;
	int spriteFramesNum;
	int tileSize;

	Texture *playerTex;

	tinytiled_map_t *tiledMap; 
	Texture *tilesetTexture;
	Texture *mapTexture;

	Turret turrets[TURRETS_MAX];
	Texture *basicTurretBaseTex;
	Texture *basicTurretGunTex;

	InvType currentInv;
	Texture *selecterTexture;

	Turret *selectedTurret;

	Texture *upgradeOption1Texture;
	Texture *upgradeOption2Texture;
	Texture *upgradeOption3Texture;
	Texture *disassembleOptionTexture;

	BitmapFont *mainFont;
};

void update();
bool getKeyPressed(int key);
void buildTurret(int x, int y, InvType type);
Turret *isRectOverTurret(Rect *rect);
Turret *isPointOverTurret(Point *point);
Turret *isPointOverTurret(float px, float py);

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
	bool firstFrame = false;
	if (!game) {
		firstFrame = true;
		game = (Game *)malloc(sizeof(Game));
		memset(game, 0, sizeof(Game));

		game->player.x = 100;
		game->player.y = 100;

		game->playerTex = uploadPngTexturePath("assets/sprites/player.png");
		game->currentInv = INV_HANDS;

		game->upgradeOption1Texture = uploadPngTexturePath("assets/sprites/upgradeOption1.png");
		game->upgradeOption2Texture = uploadPngTexturePath("assets/sprites/upgradeOption2.png");
		game->upgradeOption3Texture = uploadPngTexturePath("assets/sprites/upgradeOption3.png");
		game->disassembleOptionTexture = uploadPngTexturePath("assets/sprites/disassembleOption.png");

		game->basicTurretBaseTex = uploadPngTexturePath("assets/sprites/basicTurretBase.png");
		game->basicTurretGunTex = uploadPngTexturePath("assets/sprites/basicTurretGun.png");

		game->mainFont = loadBitmapFontPath("assets/fonts/OpenSans-Regular_22.fnt");

		{ /// Setup map
			game->tilesetTexture = uploadPngTexturePath("assets/tilesets/tileset.png");

			void *mapData;
			long mapSize = readFile("assets/maps/map.json", &mapData);
			game->tiledMap = tinytiled_load_map_from_memory(mapData, mapSize, 0);
			game->tileSize = game->tiledMap->tilewidth;
			game->mapTexture = uploadTexture(NULL, game->tiledMap->width * game->tileSize, game->tiledMap->height * game->tileSize);

			tinytiled_layer_t *layer = game->tiledMap->layers;
			while (layer) {
				int *data = layer->data;
				int dataNum = layer->data_count;

				for (int i = 0; i < dataNum; i++) {
					// printf("Data: %d\n", data[i]);
				}
				drawTiles(game->tilesetTexture, game->mapTexture, game->tileSize, game->tileSize, game->tiledMap->width, game->tiledMap->height, data);
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
	Point playerCenter = {player->x + game->playerTex->width/2, player->y + game->playerTex->height * 0.90f};
	bool moveUp = false;
	bool moveDown = false;
	bool moveLeft = false;
	bool moveRight = false;
	bool invLeft = false;
	bool invRight = false;
	if (getKeyPressed('W')) moveUp = true;
	if (getKeyPressed('S')) moveDown = true;
	if (getKeyPressed('A')) moveLeft = true;
	if (getKeyPressed('D')) moveRight = true;

	if (platform->keys['Q'] == KEY_JUST_PRESSED) invLeft = true;
	if (platform->keys['E'] == KEY_JUST_PRESSED) invRight = true;

	{ /// Inventory
		InvType newInv = game->currentInv;
		if (invLeft) newInv = (InvType)(game->currentInv + 1);
		if (invRight) newInv = (InvType)(game->currentInv - 1);

		if (newInv <= INV_START) newInv = (InvType)(INV_END-1);
		if (newInv >= INV_END) newInv = (InvType)(INV_START+1);

		if (game->currentInv != newInv || firstFrame) {
			game->currentInv = newInv;
			Point selecterSize;

			if (game->selecterTexture != NULL) destroyTexture(game->selecterTexture);
			if (game->currentInv == INV_HANDS) selecterSize.setTo(1, 1);
			if (game->currentInv == INV_TURRET_BASIC) selecterSize.setTo(3, 3);

			if (selecterSize.x == 1 && selecterSize.y == 1) game->selecterTexture = uploadPngTexturePath("assets/sprites/1x1selecter.png");
			if (selecterSize.x == 3 && selecterSize.y == 3) game->selecterTexture = uploadPngTexturePath("assets/sprites/3x3selecter.png");
		}
	}

	{ /// Movement
		float moveSpeed = 5;
		Point playerMovePoint = {};
		if (moveUp) playerMovePoint.y -= moveSpeed;
		if (moveDown) playerMovePoint.y += moveSpeed;
		if (moveLeft) playerMovePoint.x -= moveSpeed;
		if (moveRight) playerMovePoint.x += moveSpeed;

		float newX = player->x + playerMovePoint.x;
		float newY = player->y + playerMovePoint.y;

		Point collPosX;
		collPosX.x = newX;
		collPosX.y = player->y;

		Point collPosY;
		collPosY.x = player->x;
		collPosY.y = newY;

		collPosX.x += game->playerTex->width/2;
		collPosX.y += game->playerTex->height * 0.90;

		collPosY.x += game->playerTex->width/2;
		collPosY.y += game->playerTex->height * 0.90;

		if (!isPointOverTurret(&collPosX)) player->x = newX;
		if (!isPointOverTurret(&collPosY)) player->y = newY;
	}

	{ /// Camera
		setCameraExtents(0, 0, game->mapTexture->width, game->mapTexture->height);
		setCameraPosition(player->x - platform->windowWidth/2 + game->playerTex->width/2, player->y - platform->windowHeight/2 + game->playerTex->height/2);
	}

	Point selecterPos;
	bool selecterValid = true;
	SpriteDef upgradeOption1;
	SpriteDef upgradeOption2;
	SpriteDef upgradeOption3;
	SpriteDef disassembleOption;
	{ /// Selecter
		Turret *selecterOverTurret = NULL;
		bool selecterOverPlayer = false;

		selecterPos.x = roundToNearest(platform->mouseX + renderer->camPos.x - game->selecterTexture->width/2, game->tileSize);
		selecterPos.y = roundToNearest(platform->mouseY + renderer->camPos.y - game->selecterTexture->height/2, game->tileSize);

		Rect selecterRect = {selecterPos.x, selecterPos.y, (float)game->selecterTexture->width, (float)game->selecterTexture->height};
		selecterOverTurret = isRectOverTurret(&selecterRect);

		if (selecterRect.containsPoint(&playerCenter)) selecterOverPlayer = true;

		if (game->currentInv == INV_HANDS) {
			if (selecterOverTurret) selecterValid = true;
			else selecterValid = false;
		} else {
			if (selecterOverTurret || selecterOverPlayer) selecterValid = false;
			else selecterValid = true;
		}

		defaultSpriteDef(&upgradeOption1);
		defaultSpriteDef(&upgradeOption2);
		defaultSpriteDef(&upgradeOption3);
		defaultSpriteDef(&disassembleOption);
		if (game->selectedTurret) {
			upgradeOption1.tex = game->upgradeOption1Texture;
			upgradeOption1.pos.x = game->selectedTurret->x + game->selectedTurret->baseTex->width + 10;
			upgradeOption1.pos.y = game->selectedTurret->y;

			upgradeOption2.tex = game->upgradeOption2Texture;
			upgradeOption2.pos.x = game->selectedTurret->x + game->selectedTurret->baseTex->width + 10;
			upgradeOption2.pos.y = game->selectedTurret->y + (game->upgradeOption2Texture->height + 10) * 1;

			upgradeOption3.tex = game->upgradeOption3Texture;
			upgradeOption3.pos.x = game->selectedTurret->x + game->selectedTurret->baseTex->width + 10;
			upgradeOption3.pos.y = game->selectedTurret->y + (game->upgradeOption3Texture->height + 10) * 2;

			disassembleOption.tex = game->disassembleOptionTexture;
			disassembleOption.pos.x = game->selectedTurret->x + game->selectedTurret->baseTex->width + 10;
			disassembleOption.pos.y = game->selectedTurret->y + (game->disassembleOptionTexture->height + 10) * 3;
		}

		if (platform->mouseJustDown) {
			if (game->selectedTurret) {
				Point worldMouse = {platform->mouseX + renderer->camPos.x, platform->mouseY + renderer->camPos.y};
				Rect up1 = {upgradeOption1.pos.x, upgradeOption1.pos.y, (float)upgradeOption1.tex->width, (float)upgradeOption1.tex->height};
				Rect up2 = {upgradeOption2.pos.x, upgradeOption2.pos.y, (float)upgradeOption2.tex->width, (float)upgradeOption2.tex->height};
				Rect up3 = {upgradeOption3.pos.x, upgradeOption3.pos.y, (float)upgradeOption3.tex->width, (float)upgradeOption3.tex->height};
				Rect disass = {disassembleOption.pos.x, disassembleOption.pos.y, (float)disassembleOption.tex->width, (float)disassembleOption.tex->height};

				if (up1.containsPoint(worldMouse.x, worldMouse.y)) {
				} else if (up2.containsPoint(worldMouse.x, worldMouse.y)) {
				} else if (up3.containsPoint(worldMouse.x, worldMouse.y)) {
				} else if (disass.containsPoint(worldMouse.x, worldMouse.y)) {
					game->selectedTurret->exists = false;
					game->selectedTurret = NULL;
				} else {
					game->selectedTurret = NULL;
				}
			}

			if (game->currentInv == INV_TURRET_BASIC) {
				if (selecterValid) buildTurret(selecterPos.x, selecterPos.y, INV_TURRET_BASIC);
			} else if (game->currentInv == INV_HANDS) {
				if (selecterValid) {
					game->selectedTurret = selecterOverTurret;
				}
			}
		}

	}

	/// Section: Render
	clearRenderer();

	SpriteDef def;
	{ /// Draw map
		defaultSpriteDef(&def);
		def.tex = game->mapTexture;
		drawSpriteEx(&def);
	}

	{ /// Draw turrets bases
		for (int i = 0; i < TURRETS_MAX; i++) {
			Turret *turret = &game->turrets[i];
			if (turret->exists) {
				defaultSpriteDef(&def);
				def.tex = turret->baseTex;
				def.pos.x = turret->x;
				def.pos.y = turret->y;
				drawSpriteEx(&def);
			}
		}
	}

	{ /// Draw turrets guns
		for (int i = 0; i < TURRETS_MAX; i++) {
			Turret *turret = &game->turrets[i];
			if (turret->exists) {
				defaultSpriteDef(&def);
				def.tex = turret->gunTex;
				def.pos.x = turret->x + turret->baseTex->width/2 - turret->gunTex->height/2;
				def.pos.y = turret->y + turret->baseTex->height/2 - turret->gunTex->height/2;
				def.rotation = turret->gunRotation;
				def.pivot.setTo(turret->gunTex->height/2, turret->gunTex->height/2);
				drawSpriteEx(&def);
			}
		}
	}

	{ /// Draw player
		defaultSpriteDef(&def);
		def.tex = game->playerTex;
		def.pos.x = player->x;
		def.pos.y = player->y;
		drawSpriteEx(&def);
	}

	{ /// Draw selecter
		defaultSpriteDef(&def);
		def.tex = game->selecterTexture;
		def.pos.x = selecterPos.x;
		def.pos.y = selecterPos.y;
		if (selecterValid) def.tint = 0x2200FF00;
		else def.tint = 0x22FF0000;
		drawSpriteEx(&def);
	}

	{ /// Draw upgrade menu
		drawSpriteEx(&upgradeOption1);
		drawSpriteEx(&upgradeOption2);
		drawSpriteEx(&upgradeOption3);
		drawSpriteEx(&disassembleOption);
	}

	swapBuffers();
}

void buildTurret(int x, int y, InvType type) {
	Turret *turret = NULL;

	for (int i = 0; i < TURRETS_MAX; i++) {
		if (!game->turrets[i].exists) {
			turret = &game->turrets[i];
			break;
		}
	}

	memset(turret, 0, sizeof(Turret));
	turret->exists = true;
	turret->x = x;
	turret->y = y;
	if (type == INV_TURRET_BASIC) {
		turret->type = TURRET_BASIC;
		turret->baseTex = game->basicTurretBaseTex;
		turret->gunTex = game->basicTurretGunTex;
	}
}

Turret *isRectOverTurret(Rect *rect) {
	Rect turretRect;
	for (int i = 0; i < TURRETS_MAX; i++) {
		Turret *turret = &game->turrets[i];
		if (turret->exists) {
			turretRect.setTo(turret->x, turret->y, turret->baseTex->width, turret->baseTex->height);

			if (turretRect.intersects(rect)) return turret;
		}
	}

	return NULL;
}

Turret *isPointOverTurret(Point *point) { return isPointOverTurret(point->x, point->y); }
Turret *isPointOverTurret(float px, float py) {
	Rect turretRect;
	for (int i = 0; i < TURRETS_MAX; i++) {
		Turret *turret = &game->turrets[i];
		if (turret->exists) {
			turretRect.setTo(turret->x, turret->y, turret->baseTex->width, turret->baseTex->height);
			if (turretRect.containsPoint(px, py)) return turret;
		}
	}

	return NULL;
}

bool getKeyPressed(int key) {
	if (key > KEY_LIMIT) return false;

	if (platform->keys[key] == KEY_PRESSED) return true;
	if (platform->keys[key] == KEY_JUST_PRESSED) return true;

	return false;
}
