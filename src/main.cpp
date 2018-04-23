/*
			TODO:
			Make diagonal movement not OP
			Make gold texture and untint
			Make sure bullets are firing directly out of turrets
			*/
#include "platform.h"
#include "renderer.h"

#define TURRETS_MAX 1024
#define COLLS_MAX 1024
#define SPAWNERS_MAX 1024
#define ENEMY_MAX 8192
#define ENEMIES_PER_SPAWNER_MAX 256
#define BULLETS_MAX 2048
#define ITEMS_MAX 2048

enum TurretType { TURRET_BASIC };
enum InvType { INV_START, INV_HANDS, INV_TURRET_BASIC, INV_END };
enum SpawnerType { SPAWNER_BATS };
enum EnemyType { ENEMY_BAT };
enum EnemyState { STATE_IDLE=0, STATE_MOVING, STATE_CHASING, STATE_ATTACKING };
enum BulletType { BULLET_BASIC };
enum ItemType { ITEM_GOLD };

struct Turret;

struct Item {
	bool exists;
	float x;
	float y;
	float type;
	Texture *tex;
};

struct Bullet {
	bool exists;
	float x;
	float y;
	float rotation;
	float damage;
	BulletType type;
	Texture *tex;
};

struct Enemy {
	bool exists;
	float x;
	float y;
	float spawnTime;
	EnemyType type;
	Texture *tex;

	EnemyState state;
	float stateTime;
	Point nextPos;
	Turret *targetTurret;
	bool chasingPlayer;
	Rect chaseRect;

	float hp;
	float maxHp;
	float attackTime;
};

struct Spawner {
	bool exists;
	Rect rect;
	SpawnerType type;
	float interval;
	int max;

	float timeLeft;
	Enemy *enemies[ENEMIES_PER_SPAWNER_MAX];
	int enemyCount;
};

struct Turret {
	bool exists;
	int x;
	int y;
	TurretType type;
	float gunRotation;
	Texture *baseTex;
	Texture *gunTex;

	float hp;
	float maxHp;
	float attackTime;
};

// struct Frame {
// 	char *name;
// 	int x;
// 	int y;
// 	int width;
// 	int height;
// 	int sourceWidth;
// 	int sourceHeight;
// 	bool rotated;
// };

struct Player {
	float x;
	float y;
	Texture *tex;

	float hp;
	float maxHp;
};

struct Game {
	Texture *tilesetTexture;

	Texture *basicTurretBaseTexture;
	Texture *basicTurretGunTexture;

	Texture *upgradeOption1Texture;
	Texture *upgradeOption2Texture;
	Texture *upgradeOption3Texture;
	Texture *disassembleOptionTexture;

	Texture *enemyBatTexture;

	Texture *bulletBasicTexture;

	Texture *goldTexture;

	BitmapFont *mainFont;
	BitmapFont *smallFont;

	Player player;

	// Frame *spriteFrames;
	// int spriteFramesNum;
	int tileSize;

	tinytiled_map_t *tiledMap; 
	Texture *mapTexture;

	Turret turrets[TURRETS_MAX];
	Turret *selectedTurret;

	InvType currentInv;
	Texture *selecterTexture;

	Texture *frameTimeText;

	Rect colls[COLLS_MAX];
	Spawner spawners[SPAWNERS_MAX];
	Enemy enemies[ENEMY_MAX];
	Bullet bullets[BULLETS_MAX];
	Item items[ITEMS_MAX];

	int gold;
	Texture *goldText;
};

void update();
bool getKeyPressed(int key);
void buildTurret(int x, int y, InvType type);

Enemy *spawnEnemy(float x, float y, EnemyType type);

Turret *isRectOverTurret(Rect *rect);
Turret *isPointOverTurret(float px, float py);
bool isPointOverColl(float px, float py);
Turret *getClosestTurret(float px, float py);
Enemy *getClosestEnemy(float px, float py);

Bullet *shootBullet(float x, float y, BulletType type, float degrees, float startDist);
Item *createItem(float x, float y, ItemType type);

void drawHpBar(float x, float y, float value, float total);

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

			game->player.tex = uploadPngTexturePath("assets/sprites/player.png");
			game->player.maxHp = game->player.hp = 100;

			game->currentInv = INV_HANDS;

			game->tilesetTexture = uploadPngTexturePath("assets/tilesets/tileset.png");

			game->upgradeOption1Texture = uploadPngTexturePath("assets/sprites/upgradeOption1.png");
			game->upgradeOption2Texture = uploadPngTexturePath("assets/sprites/upgradeOption2.png");
			game->upgradeOption3Texture = uploadPngTexturePath("assets/sprites/upgradeOption3.png");
			game->disassembleOptionTexture = uploadPngTexturePath("assets/sprites/disassembleOption.png");

			game->basicTurretBaseTexture = uploadPngTexturePath("assets/sprites/basicTurretBase.png");
			game->basicTurretGunTexture = uploadPngTexturePath("assets/sprites/basicTurretGun.png");

			game->enemyBatTexture = uploadPngTexturePath("assets/sprites/enemyBat.png");

			game->bulletBasicTexture = uploadPngTexturePath("assets/sprites/bulletBasic.png");

			game->goldTexture = uploadPngTexturePath("assets/sprites/bulletBasic.png");

			game->mainFont = loadBitmapFontPath("assets/fonts/OpenSans-Regular_22.fnt");
			game->smallFont = loadBitmapFontPath("assets/fonts/OpenSans-Regular_16.fnt");

			game->frameTimeText = uploadTexture(NULL, 512, 256);

			game->goldText = uploadTexture(NULL, 512, 256);

			{ /// Setup map
				void *mapData;
				long mapSize = readFile("assets/maps/map.json", &mapData);
				game->tiledMap = tinytiled_load_map_from_memory(mapData, mapSize, 0);
				game->tileSize = game->tiledMap->tilewidth;
				game->mapTexture = uploadTexture(NULL, game->tiledMap->width * game->tileSize, game->tiledMap->height * game->tileSize);

				tinytiled_layer_t *layer = game->tiledMap->layers;
				while (layer) {
					if (streq(layer->name.ptr, "visual")) {
						drawTiles(game->tilesetTexture, game->mapTexture, game->tileSize, game->tileSize, game->tiledMap->width, game->tiledMap->height, layer->data);
					}

					tinytiled_object_t *object = layer->objects;
					while (object) {
						if (streq(layer->name.ptr, "coll")) {
							for (int i = 0; i < COLLS_MAX; i++) {
								if (game->colls[i].width == 0) {
									game->colls[i].setTo(object->x, object->y, object->width, object->height);
									break;
								}
							}
						}

						if (streq(layer->name.ptr, "meta")) {
							if (streq(object->name.ptr, "spawner")) {
								for (int i = 0; i < SPAWNERS_MAX; i++) {
									Spawner *spawner = &game->spawners[i];
									if (spawner->exists) continue;
									memset(spawner, 0, sizeof(Spawner));
									spawner->exists = true;
									spawner->rect.setTo(object->x, object->y, object->width, object->height);
									for (int j = 0; j < object->property_count; j++) {
										tinytiled_property_t *prop = &object->properties[j];
										float value;
										if (prop->type == TINYTILED_PROPERTY_INT) value = prop->data.integer;
										if (prop->type == TINYTILED_PROPERTY_FLOAT) value = prop->data.floating;

										if (streq(prop->name.ptr, "interval")) spawner->interval = value;
										if (streq(prop->name.ptr, "max")) spawner->max = value;
									}
									break;
								}
							}
						}

						object = object->next;
					}

					layer = layer->next;
				}

				game->player.x = game->mapTexture->width/2 + game->tileSize/2 - game->player.tex->width/2;
				game->player.y = game->mapTexture->height/2 + game->tileSize/2 - game->player.tex->height/2;
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
		Point playerCenter = {player->x + player->tex->width/2, player->y + player->tex->height * 0.90f};
		Rect playerRect = {player->x, player->y, (float)player->tex->width, (float)player->tex->height};

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
			float moveSpeed = 3;
			Point playerMovePoint = {};
			if (moveUp) playerMovePoint.y -= moveSpeed;
			if (moveDown) playerMovePoint.y += moveSpeed;
			if (moveLeft) playerMovePoint.x -= moveSpeed;
			if (moveRight) playerMovePoint.x += moveSpeed;

			float collX = playerCenter.x + playerMovePoint.x;
			float collY = playerCenter.y + playerMovePoint.y;

			bool canMoveX = true;
			bool canMoveY = true;

			if (isPointOverTurret(collX, playerCenter.y)) canMoveX = false;
			if (isPointOverTurret(playerCenter.x, collY)) canMoveY = false;
			if (isPointOverColl(collX, playerCenter.y)) canMoveX = false;
			if (isPointOverColl(playerCenter.x, collY)) canMoveY = false;

			if (canMoveX) player->x += playerMovePoint.x;
			if (canMoveY) player->y += playerMovePoint.y;
		}

		{ /// Camera
			setCameraExtents(0, 0, game->mapTexture->width, game->mapTexture->height);
			setCameraPosition(player->x - platform->windowWidth/2 + game->player.tex->width/2, player->y - platform->windowHeight/2 + game->player.tex->height/2);
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

		{ /// Spawners
			for (int i = 0; i < SPAWNERS_MAX; i++) {
				Spawner *spawner = &game->spawners[i];
				if (!spawner->exists) continue;
				spawner->timeLeft -= platform->elapsed;
				if (spawner->timeLeft <= 0 && spawner->enemyCount < spawner->max) {
					Point spawnPoint;
					spawner->rect.randomPoint(&spawnPoint);
					spawnEnemy(spawnPoint.x, spawnPoint.y, ENEMY_BAT);
					spawner->enemyCount++;
					spawner->timeLeft = spawner->interval;
				}

				for (int j = 0; j < ENEMIES_PER_SPAWNER_MAX; j++) {
					Enemy *enemy = spawner->enemies[j];

					if (enemy && !enemy->exists) {
						spawner->enemies[j] = NULL;
						spawner->enemyCount--;
					}
				}
			}
		}

		{ /// Enemies
			for (int i = 0; i < ENEMY_MAX; i++) {
				Enemy *enemy = &game->enemies[i];
				if (!enemy->exists) continue;

				Point enemyCenter = {enemy->x + enemy->tex->width/2, enemy->y + enemy->tex->height/2};
				Rect enemyRect = {enemy->x, enemy->y, (float)enemy->tex->width, (float)enemy->tex->height};

				float idleLimit, moveSpeed, chaseSpeed, moveDistMin, moveDistMax, aggroRange, attackRate, attackDamage, goldGiven;
				if (enemy->type == ENEMY_BAT) {
					idleLimit = 3;
					moveSpeed = 1;
					chaseSpeed = 3;
					moveDistMin = 32;
					moveDistMax = 96;
					aggroRange = 96;
					attackRate = 1;
					attackDamage = 3;
					goldGiven = 10;
				}

				enemy->stateTime += platform->elapsed;
				if (enemy->state == STATE_IDLE) {
					if (enemy->stateTime > idleLimit) {
						enemy->state = STATE_MOVING;
						enemy->stateTime = 0;
						enemy->nextPos.x = enemyCenter.x + (rndFloat(moveDistMin, moveDistMax) * (rnd() > 0.5 ? -1 : 1));
						enemy->nextPos.y = enemyCenter.y + (rndFloat(moveDistMin, moveDistMax) * (rnd() > 0.5 ? -1 : 1));
					}
				}

				if (enemy->state == STATE_MOVING) {
					float angle = radsBetween(enemyCenter.x, enemyCenter.y, enemy->nextPos.x, enemy->nextPos.y);
					enemy->x += cos(angle) * moveSpeed;
					enemy->y += sin(angle) * moveSpeed;

					if (distanceBetween(enemyCenter.x, enemyCenter.y, enemy->nextPos.x, enemy->nextPos.y) < 10) {
						enemy->state = STATE_IDLE;
						enemy->stateTime = 0;
					}
				}

				float playerDist = distanceBetween(enemyCenter.x, enemyCenter.y, playerCenter.x, playerCenter.y);

				Turret *closestTurret = getClosestTurret(enemyCenter.x, enemyCenter.y);
				float turretDist = 9999999999;
				if (closestTurret) {
					turretDist = distanceBetween(
						enemyCenter.x, enemyCenter.y,
						closestTurret->x + closestTurret->baseTex->width/2, closestTurret->y + closestTurret->baseTex->height/2
					);
				}

				//@cleanup Do these vars even need to be inside the enemy struct? Same with chaseRect
				enemy->chasingPlayer = false;
				enemy->targetTurret = NULL;

				bool targetInRange = false;
				if (playerDist < turretDist && playerDist < aggroRange) {
					enemy->chasingPlayer = true;
					enemy->chaseRect.setTo(player->x, player->y, player->tex->width, player->tex->height);
					targetInRange = true;
				} else if (turretDist < playerDist && turretDist < aggroRange) {
					enemy->targetTurret = closestTurret;
					enemy->chaseRect.setTo(closestTurret->x, closestTurret->y, closestTurret->baseTex->width, closestTurret->baseTex->height);
					targetInRange = true;
				}

				if (targetInRange) {
					enemy->state = STATE_CHASING;
					if (enemyRect.intersects(&enemy->chaseRect)) {
						enemy->state = STATE_ATTACKING;
					} else {
						Point chaseCenter = {enemy->chaseRect.x + enemy->chaseRect.width/2, enemy->chaseRect.y + enemy->chaseRect.height/2};
						float angle = radsBetween(enemyCenter.x, enemyCenter.y, chaseCenter.x, chaseCenter.y);
						enemy->x += cos(angle) * chaseSpeed;
						enemy->y += sin(angle) * chaseSpeed;
					}
				}

				if (!targetInRange && (enemy->state == STATE_CHASING || enemy->state == STATE_ATTACKING)) {
					enemy->state = STATE_IDLE;
				}

				if (enemy->state == STATE_ATTACKING) {
					enemy->attackTime += platform->elapsed;
					if (enemy->attackTime > attackRate) {
						if (enemy->chasingPlayer) player->hp -= attackDamage;
						if (enemy->targetTurret) enemy->targetTurret->hp -= attackDamage;
						enemy->attackTime = 0;
					}
				}

				if (enemy->hp <= 0) {
					for (int goldIndex = 0; goldIndex < goldGiven; goldIndex++) {
						Point itemPoint;
						enemyRect.randomPoint(&itemPoint);
						createItem(itemPoint.x, itemPoint.y, ITEM_GOLD);
					}
					enemy->exists = false;
				}
			}
		}

		{ /// Turrets
			for (int i = 0; i < TURRETS_MAX; i++) {
				Turret *turret = &game->turrets[i];
				if (!turret->exists) continue;
				Point turretCenter = {turret->x + turret->baseTex->width/2.0f, turret->y + turret->baseTex->height/2.0f};

				float turretRange, turretRate, turretDamage;
				if (turret->type == TURRET_BASIC) {
					turretRange = 320;
					turretRate = 1;
					turretDamage = 5;
				}

				Enemy *closestEnemy = getClosestEnemy(turretCenter.x, turretCenter.y);
				float enemyDist = 999999;

				Point enemyCenter = {};
				if (closestEnemy) {
					enemyCenter.setTo(closestEnemy->x + closestEnemy->tex->width/2, closestEnemy->y + closestEnemy->tex->height/2);
					enemyDist = distanceBetween(turretCenter.x, turretCenter.y, enemyCenter.x, enemyCenter.y);
				}

				if (enemyDist < turretRange) {
					turret->gunRotation = toDeg(radsBetween(turretCenter.x, turretCenter.y, enemyCenter.x, enemyCenter.y));

					turret->attackTime += platform->elapsed;
					if (turret->attackTime > turretRate) {
						Bullet *bullet = shootBullet(turretCenter.x, turretCenter.y, BULLET_BASIC, turret->gunRotation, turret->gunTex->width);
						bullet->damage = turretDamage;
						turret->attackTime = 0;
					}
				}

				if (turret->hp <= 0) turret->exists = false;
			}
		}

		{ /// Bullets
			for (int i = 0; i < BULLETS_MAX; i++) {
				Bullet *bullet = &game->bullets[i];
				if (!bullet->exists) continue;

				float bulletSpeed;

				if (bullet->type == BULLET_BASIC) {
					bulletSpeed = 5;
				}

				bullet->x += cos(toRad(bullet->rotation)) * bulletSpeed;
				bullet->y += sin(toRad(bullet->rotation)) * bulletSpeed;

				Rect bulletRect = {bullet->x, bullet->y, (float)bullet->tex->width, (float)bullet->tex->height};

				for (int i = 0; i < ENEMY_MAX; i++) {
					Enemy *enemy = &game->enemies[i];
					if (!enemy->exists) continue;
					Rect enemyRect = {enemy->x, enemy->y, (float)enemy->tex->width, (float)enemy->tex->height};

					if (bulletRect.intersects(&enemyRect)) {
						bullet->exists = false;
						enemy->hp -= bullet->damage;
					}
				}
			}
		}

		{ /// Items
			for (int i = 0; i < ITEMS_MAX; i++) {
				Item *item = &game->items[i];
				if (!item->exists) continue;

				Rect itemRect = {item->x, item->y, (float)item->tex->width, (float)item->tex->height}; 
				if (playerRect.intersects(&itemRect)) {
					game->gold++;
					item->exists = false;
				}
			}
		}

		TextProps goldTextProps;
		{ /// Hud
			drawText(game->frameTimeText, game->smallFont, "Frame time: %d", NULL, platform->frameTime);
			drawText(game->goldText, game->mainFont, "Gold: %d", &goldTextProps, game->gold);
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
				if (!turret->exists) continue;
				defaultSpriteDef(&def);
				def.tex = turret->baseTex;
				def.pos.x = turret->x;
				def.pos.y = turret->y;
				drawSpriteEx(&def);
			}
		}

		{ /// Draw turrets guns
			for (int i = 0; i < TURRETS_MAX; i++) {
				Turret *turret = &game->turrets[i];
				if (!turret->exists) continue;
				defaultSpriteDef(&def);
				def.tex = turret->gunTex;
				def.pos.x = turret->x + turret->baseTex->width/2 - turret->gunTex->height/2;
				def.pos.y = turret->y + turret->baseTex->height/2 - turret->gunTex->height/2;
				def.rotation = turret->gunRotation;
				def.pivot.setTo(turret->gunTex->height/2, turret->gunTex->height/2);
				drawSpriteEx(&def);
			}
		}

		{ /// Draw items
			for (int i = 0; i < ITEMS_MAX; i++) {
				Item *item = &game->items[i];
				if (!item->exists) continue;
				defaultSpriteDef(&def);
				def.tex = item->tex;
				def.pos.x = item->x;
				def.pos.y = item->y;
				def.tint = 0xFFFF00FF;
				drawSpriteEx(&def);
			}
		}

		{ /// Draw player
			defaultSpriteDef(&def);
			def.tex = game->player.tex;
			def.pos.x = player->x;
			def.pos.y = player->y;
			drawSpriteEx(&def);
		}

		{ /// Draw enemies
			for (int i = 0; i < ENEMY_MAX; i++) {
				Enemy *enemy = &game->enemies[i];
				if (!enemy->exists) continue;
				defaultSpriteDef(&def);
				def.tex = enemy->tex;
				def.pos.x = enemy->x;
				def.pos.y = enemy->y;

				if (enemy->type == ENEMY_BAT) {
					float floatSpeedX;
					float floatSpeedY;
					float floatDistX;
					float floatDistY;

					if (enemy->state == STATE_ATTACKING) {
						floatSpeedX = 20;
						floatSpeedY = 10;
						floatDistX = 3;
						floatDistY = 3;
					} else {
						floatSpeedX = 0;
						floatSpeedY = 2;
						floatDistX = 0;
						floatDistY = 3;
					}

					def.pos.x += cos((platform->time - enemy->spawnTime)*floatSpeedX) * floatDistX;
					def.pos.y += cos((platform->time - enemy->spawnTime)*floatSpeedY) * floatDistY;
				}

				if (enemy->state == STATE_CHASING) {
					def.tint = 0x88FF0000;
				}

				drawSpriteEx(&def);
			}
		}

		{ /// Draw bullets
			for (int i = 0; i < BULLETS_MAX; i++) {
				Bullet *bullet = &game->bullets[i];
				if (!bullet->exists) continue;
				defaultSpriteDef(&def);
				def.tex = bullet->tex;
				def.pos.x = bullet->x;
				def.pos.y = bullet->y;
				def.rotation = bullet->rotation;
				drawSpriteEx(&def);
			}
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

		{ /// Draw hud
			defaultSpriteDef(&def);
			def.tex = game->frameTimeText;
			def.scrollFactor.setTo(0, 0);
			drawSpriteEx(&def);

			defaultSpriteDef(&def);
			def.tex = game->goldText;
			def.pos.x = platform->windowWidth - goldTextProps.width;
			def.pos.y = platform->windowHeight - goldTextProps.height;
			def.scrollFactor.setTo(0, 0);
			drawSpriteEx(&def);
		}

		{ /// Draw hp bars
			drawHpBar(player->x + player->tex->width/2, player->y + player->tex->height + 5, player->hp, player->maxHp);

			for (int i = 0; i < TURRETS_MAX; i++) {
				Turret *turret = &game->turrets[i];
				if (!turret->exists) continue;
				drawHpBar(turret->x + turret->baseTex->width/2, turret->y + turret->baseTex->height - 10, turret->hp, turret->maxHp);
			}

			for (int i = 0; i < ENEMY_MAX; i++) {
				Enemy *enemy = &game->enemies[i];
				if (!enemy->exists) continue;
				drawHpBar(enemy->x + enemy->tex->width/2, enemy->y + enemy->tex->height + 5, enemy->hp, enemy->maxHp);
			}
		}

		// drawRect(player->x + 100 - renderer->camPos.x, player->y + 100 - renderer->camPos.y, 100, 100, 0xFFFFFFFF);

		swapBuffers();
	}

	void drawHpBar(float x, float y, float value, float total) {
		float width = total/1.5;

		drawRect(x - width/2 - renderer->camPos.x, y - renderer->camPos.y, width, 1, 0xFFFF0000);
		drawRect(x - width/2 - renderer->camPos.x, y - renderer->camPos.y, value/total * width, 1, 0xFF00FF00);
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
			turret->baseTex = game->basicTurretBaseTexture;
			turret->gunTex = game->basicTurretGunTexture;

			turret->maxHp = turret->hp = 100;
		}
	}

	Enemy *spawnEnemy(float x, float y, EnemyType type) {
		for (int i = 0; i < ENEMY_MAX; i++) {
			Enemy *enemy = &game->enemies[i];
			if (enemy->exists) continue;
			memset(enemy, 0, sizeof(Enemy));
			enemy->exists = true;
			enemy->spawnTime = platform->time;

			if (type == ENEMY_BAT) {
				enemy->tex = game->enemyBatTexture;
				enemy->maxHp = enemy->hp = 20;
			}

			enemy->x = x - enemy->tex->width/2;
			enemy->y = y - enemy->tex->height/2;
			enemy->type = type;

			return enemy;
		}

		return NULL;
	}

	Turret *isRectOverTurret(Rect *rect) {
		Rect turretRect;
		for (int i = 0; i < TURRETS_MAX; i++) {
			Turret *turret = &game->turrets[i];
			if (!turret->exists) continue;
			turretRect.setTo(turret->x, turret->y, turret->baseTex->width, turret->baseTex->height);

			if (turretRect.intersects(rect)) return turret;
		}

		return NULL;
	}

	Turret *isPointOverTurret(float px, float py) {
		Rect turretRect;
		for (int i = 0; i < TURRETS_MAX; i++) {
			Turret *turret = &game->turrets[i];
			if (!turret->exists) continue;
			turretRect.setTo(turret->x, turret->y, turret->baseTex->width, turret->baseTex->height);
			if (turretRect.containsPoint(px, py)) return turret;
		}

		return NULL;
	}

	bool isPointOverColl(float px, float py) {
		for (int i = 0; i < COLLS_MAX; i++) {
			Rect *coll = &game->colls[i];
			if (coll->width != 0) {
				if (coll->containsPoint(px, py)) return true;
			}
		}

		return false;
	}

	Turret *getClosestTurret(float px, float py) {
		Turret *closest = NULL;
		float dist = 0;

		for (int i = 0; i < TURRETS_MAX; i++) {
			Turret *turret = &game->turrets[i];
			if (!turret->exists) continue;
			float curDist = distanceBetween(px, py, turret->x + turret->baseTex->width/2, turret->y + turret->baseTex->height/2);
			if (curDist < dist || closest == NULL) {
				dist = curDist;
				closest = turret;
			}
		}

		return closest;
	}

	Enemy *getClosestEnemy(float px, float py) {
		Enemy *closest = NULL;
		float dist = 0;

		for (int i = 0; i < TURRETS_MAX; i++) {
			Enemy *enemy = &game->enemies[i];
			if (!enemy->exists) continue;
			float curDist = distanceBetween(px, py, enemy->x + enemy->tex->width/2, enemy->y + enemy->tex->height/2);
			if (curDist < dist || closest == NULL) {
				dist = curDist;
				closest = enemy;
			}
		}

		return closest;
	}

	Bullet *shootBullet(float x, float y, BulletType type, float degrees, float startDist) {
		for (int i = 0; i < BULLETS_MAX; i++) {
			Bullet *bullet = &game->bullets[i];
			if (bullet->exists) continue;
			memset(bullet, 0, sizeof(Bullet));
			bullet->exists = true;
			bullet->type = type;
			bullet->rotation = degrees;

			if (type == BULLET_BASIC) {
				bullet->tex = game->bulletBasicTexture;
			}

			bullet->x = x + cos(toRad(degrees)) * startDist;
			bullet->y = y + sin(toRad(degrees)) * startDist;

			return bullet;
		}

		return NULL;
	}

	Item *createItem(float x, float y, ItemType type) {
		for (int i = 0; i < ITEMS_MAX; i++) {
			Item *item = &game->items[i];
			if (item->exists) continue;
			memset(item, 0, sizeof(Item));
			item->exists = true;
			item->type = type;
			item->x = x;
			item->y = y;
			item->tex = game->goldTexture;

			return item;
		}

		return NULL;
	}

	bool getKeyPressed(int key) {
		if (key > KEY_LIMIT) return false;

		if (platform->keys[key] == KEY_PRESSED) return true;
		if (platform->keys[key] == KEY_JUST_PRESSED) return true;

		return false;
	}
