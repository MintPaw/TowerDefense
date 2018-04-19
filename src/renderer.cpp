#include "renderer.h"
#include "mathTools.h"

void checkGlError(int lineNum);
#define CheckGlError() checkGlError(__LINE__);

GLuint buildShader(char *vertSrc, char *fragSrc);
void setGlViewport(int x, int y, int width, int height);
void setShaderProgram(GLuint program);
void setArrayBuffer(GLuint buffer);
void changeArrayBuffer(GLuint glBuffer, float x, float y, float width, float height);
GLuint genGlArrayBuffer(float x, float y, float width, float height);
void setTexture(GLuint texture);

struct SpriteProgram {
	GLuint program;
	GLuint a_position;
	GLuint a_texCoord;
	GLuint u_matrix;
	GLuint u_uv;
	GLuint u_tint;
	GLuint u_alpha;
};

struct CircleProgram {
	GLuint program;
	GLuint a_position;
	GLuint a_texCoord;
	GLuint u_colour;
	GLuint u_projection;
};

struct TilemapProgram {
	GLuint program;
	GLuint a_position;
	GLuint u_tilesetTexture;
	GLuint u_tilemapTexture;
	GLuint u_tilesetSize;
	GLuint u_tilemapSize;
	GLuint u_resultSize;
	GLuint u_pixelRatio;
};

struct Renderer {
	int errorCount;

	CircleProgram circleProgram;
	SpriteProgram spriteProgram;
	TilemapProgram tilemapProgram;

	GLuint tempVerts;
	GLuint tempTexCoords;

	Rect curViewport;
	GLuint curShaderProgram;
	GLuint curArrayBuffer;
	GLint currentTexture;
};

Renderer *renderer;

void initRenderer() {
	log("Initing renderer\n");

	renderer = (Renderer *)malloc(sizeof(Renderer));
	memset(renderer, 0, sizeof(Renderer));

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CheckGlError();

#ifdef GL_CORE
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	char *circleVertStr;
	char *circleFragStr;
	readFile("assets/shaders/circle.vert", (void **)&circleVertStr);
	readFile("assets/shaders/circle.frag", (void **)&circleFragStr);
	renderer->circleProgram.program = buildShader(circleVertStr, circleFragStr);
	free(circleFragStr);
	free(circleVertStr);

	renderer->circleProgram.a_position = glGetAttribLocation(renderer->circleProgram.program, "a_position");
	renderer->circleProgram.a_texCoord = glGetAttribLocation(renderer->circleProgram.program, "a_texCoord");
	renderer->circleProgram.u_colour = glGetUniformLocation(renderer->circleProgram.program, "u_colour");
	renderer->circleProgram.u_projection = glGetUniformLocation(renderer->circleProgram.program, "u_projection");

	char *spriteVertStr;
	char *spriteFragStr;
	readFile("assets/shaders/default.vert", (void **)&spriteVertStr);
	readFile("assets/shaders/default.frag", (void **)&spriteFragStr);
	renderer->spriteProgram.program = buildShader(spriteVertStr, spriteFragStr);
	free(spriteFragStr);
	free(spriteVertStr);

	renderer->spriteProgram.a_position = glGetAttribLocation(renderer->spriteProgram.program, "a_position");
	renderer->spriteProgram.a_texCoord = glGetAttribLocation(renderer->spriteProgram.program, "a_texCoord");
	renderer->spriteProgram.u_matrix = glGetUniformLocation(renderer->spriteProgram.program, "u_matrix");
	renderer->spriteProgram.u_uv = glGetUniformLocation(renderer->spriteProgram.program, "u_uv");
	renderer->spriteProgram.u_tint = glGetUniformLocation(renderer->spriteProgram.program, "u_tint");
	renderer->spriteProgram.u_alpha = glGetUniformLocation(renderer->spriteProgram.program, "u_alpha");

	char *tilemapVertStr;
	char *tilemapFragStr;
	readFile("assets/shaders/tilemap.vert", (void **)&tilemapVertStr);
	readFile("assets/shaders/tilemap.frag", (void **)&tilemapFragStr);
	renderer->tilemapProgram.program = buildShader(tilemapVertStr, tilemapFragStr);
	free(tilemapFragStr);
	free(tilemapVertStr);

	renderer->tilemapProgram.a_position = glGetAttribLocation(renderer->tilemapProgram.program, "a_position");
	renderer->tilemapProgram.u_tilesetTexture = glGetUniformLocation(renderer->tilemapProgram.program, "u_tilesetTexture");
	renderer->tilemapProgram.u_tilemapTexture = glGetUniformLocation(renderer->tilemapProgram.program, "u_tilemapTexture");
	renderer->tilemapProgram.u_tilesetSize = glGetUniformLocation(renderer->tilemapProgram.program, "u_tilesetSize");
	renderer->tilemapProgram.u_tilemapSize = glGetUniformLocation(renderer->tilemapProgram.program, "u_tilemapSize");
	renderer->tilemapProgram.u_resultSize = glGetUniformLocation(renderer->tilemapProgram.program, "u_resultSize");
	renderer->tilemapProgram.u_pixelRatio = glGetUniformLocation(renderer->tilemapProgram.program, "u_pixelRatio");

	CheckGlError();

	renderer->tempVerts = genGlArrayBuffer(0, 0, 0, 0);
	renderer->tempTexCoords = genGlArrayBuffer(0, 0, 0, 0);
	CheckGlError();
}

GLuint buildShader(char *vertSrc, char *fragSrc) {
#ifdef GL_ES
	const char *versionLine = "#version 300 es\n";
#else
	const char *versionLine = "#version 330\n";
#endif

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	char *realVert = (char *)malloc((strlen(vertSrc) + strlen(versionLine) + 1) * sizeof(char));
	strcpy(realVert, versionLine);
	strcat(realVert, vertSrc);
	int vertLen = strlen(realVert);
	glShaderSource(vert, 1, (const char **)&realVert, &vertLen);
	glCompileShader(vert);
	free(realVert);

	static const int errLogLimit = 1024;
	char errLog[errLogLimit];
	int errLogNum;

	int vertReturn;
	glGetShaderiv(vert, GL_COMPILE_STATUS, &vertReturn);
	if (!vertReturn) {
		glGetShaderInfoLog(vert, errLogLimit, &errLogNum, errLog);
		log("Vertex result is: %d\nError(%d):\n%s\n", vertReturn, errLogNum, errLog);
	}

	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	char *realFrag = (char *)malloc((strlen(fragSrc) + strlen(versionLine) + 1) * sizeof(char));
	strcpy(realFrag, versionLine);
	strcat(realFrag, fragSrc);
	int fragLen = strlen(realFrag);
	glShaderSource(frag, 1, (const char **)&realFrag, &fragLen);
	glCompileShader(frag);
	free(realFrag);

	int fragReturn;
	glGetShaderiv(frag, GL_COMPILE_STATUS, &fragReturn);
	if (!fragReturn) {
		glGetShaderInfoLog(frag, errLogLimit, &errLogNum, errLog);
		log("Fragment result is: %d\nError:\n%s\n", fragReturn, errLog);
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	int progReturn;
	glGetProgramiv(program, GL_LINK_STATUS, &progReturn);
	if (!progReturn) {
		glGetShaderInfoLog(program, errLogLimit, &errLogNum, errLog);
		log("Program result is: %d\nError:\n%s\n", progReturn, errLog);
	}

	CheckGlError();
	return program;
}

void drawCircle(float x, float y, float radius, int colour) {
	setGlViewport(0, 0, platform->windowWidth, platform->windowHeight);
	setShaderProgram(renderer->circleProgram.program);
	CheckGlError();

	x -= radius/2;
	y -= radius/2;

	glEnableVertexAttribArray(renderer->circleProgram.a_position);
	changeArrayBuffer(renderer->tempVerts, x, y, x+radius, y+radius);
	glVertexAttribPointer(renderer->circleProgram.a_position, 2, GL_FLOAT, false, 0, NULL);
	CheckGlError();

	glEnableVertexAttribArray(renderer->circleProgram.a_texCoord);
	changeArrayBuffer(renderer->tempTexCoords, 0, 0, 1, 1);
	glVertexAttribPointer(renderer->circleProgram.a_texCoord, 2, GL_FLOAT, false, 0, NULL);
	CheckGlError();

	Matrix projection;
	projection.identity();
	projection.project(platform->windowWidth, platform->windowHeight);
	glUniformMatrix3fv(renderer->circleProgram.u_projection, 1, false, (float *)projection.data);

	glUniform4f(
		renderer->circleProgram.u_colour,
		((colour >> 16) & 0xff)/255.0,
		((colour >> 8) & 0xff)/255.0,
		(colour & 0xff)/255.0,
		((colour >> 24) & 0xff)/255.0
	);
	CheckGlError();

	glDrawArrays(GL_TRIANGLES, 0, 2*3);

	glDisableVertexAttribArray(renderer->circleProgram.a_position);
	glDisableVertexAttribArray(renderer->circleProgram.a_texCoord);

	CheckGlError();
}

Texture *uploadPngTexturePath(char *path) {
	void *pngData;
	int pngSize = readFile(path, &pngData);
	Texture *tex = uploadPngTexture(pngData, pngSize);
	// free(pngData);

	return tex;
}

Texture *uploadPngTexture(void *data, int size) {
	int width, height, channels;
	stbi_uc *img = 0;
	img = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &channels, 4);
	Assert(img);

	Texture *tex = uploadTexture(img, width, height);
	// free(img);

	return tex;
}

Texture *uploadTexture(void *data, int width, int height) {
	GLuint texId;
	glGenTextures(1, &texId);
	setTexture(texId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (!data) {
		data = malloc(width * height * 4);
		memset(data, 0, width * height * 4);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	CheckGlError();

	Texture *tex = (Texture *)malloc(sizeof(Texture));
	tex->textureId = texId;
	tex->width = width;
	tex->height = height;

	return tex;
}

void destroyTexture(Texture *tex) {
	glDeleteTextures(1, &tex->textureId);
	free(tex);
}

void drawSprite(Texture *tex, float x, float y) {
	setGlViewport(0, 0, platform->windowWidth, platform->windowHeight);
	setShaderProgram(renderer->spriteProgram.program);
	CheckGlError();

	// x -= radius/2;
	// y -= radius/2;

	glEnableVertexAttribArray(renderer->spriteProgram.a_position);
	changeArrayBuffer(renderer->tempVerts, 0, 0, tex->width, tex->height);
	glVertexAttribPointer(renderer->spriteProgram.a_position, 2, GL_FLOAT, false, 0, NULL);
	CheckGlError();

	glEnableVertexAttribArray(renderer->spriteProgram.a_texCoord);
	changeArrayBuffer(renderer->tempTexCoords, 0, 0, 1, 1);
	glVertexAttribPointer(renderer->spriteProgram.a_texCoord, 2, GL_FLOAT, false, 0, NULL);
	CheckGlError();

	Matrix matrix;
	matrix.identity();
	matrix.project(platform->windowWidth, platform->windowHeight);
	matrix.translate(x, y);
	glUniformMatrix3fv(renderer->spriteProgram.u_matrix, 1, false, (float *)matrix.data);

	setTexture(tex->textureId);

	Matrix uv;
	uv.identity();
	glUniformMatrix3fv(renderer->spriteProgram.u_uv, 1, false, (float *)uv.data);

	int tint = 0x00000000;
	glUniform4f(
		renderer->spriteProgram.u_tint,
		((tint >> 16) & 0xff)/255.0,
		((tint >> 8) & 0xff)/255.0,
		(tint & 0xff)/255.0,
		((tint >> 24) & 0xff)/255.0
	);

	glUniform1f(renderer->spriteProgram.u_alpha, 1);

	CheckGlError();

	glDrawArrays(GL_TRIANGLES, 0, 2*3);
	CheckGlError();
}

void drawTiles(Texture *srcTexture, Texture *destTexture, int tileWidth, int tileHeight, int tilesWide, int tilesHigh, int *tiles) {
	unsigned char *texData = (unsigned char *)malloc(tilesWide * tilesHigh * 4);

	memset(texData, 0, tilesWide * tilesHigh * 4);
	for (int i = 0; i < tilesWide * tilesHigh; i++) {
		texData[i*4 + 0] = ((tiles[i]) & 0xff);
		texData[i*4 + 1] = ((tiles[i] >> 8) & 0xff);
		// texData[i*4 + 2] = ((tiles[i] >> 16) & 0xff);
		// texData[i*4 + 3] = ((tiles[i] >> 24) & 0xff);
		// printf("%d %d\n", texData[i*4 + 0], texData[i*4 + 1]);
	}

	Texture *tempTex = uploadTexture(texData, tilesWide, tilesHigh);
	// checkGlError(__LINE__, "generated tile data texture");
	// Free(texData);

	// Asset *tilesetTextureAsset = getTextureAsset(assetId);

	// setFramebuffer(renderer->textureFramebuffer);
	// checkGlError(__LINE__, "");
	// setFramebufferTexture(spr->texture);
	// checkGlError(__LINE__, "");
	// // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Framebuffer is not ready!\n");

	// setShaderProgram(&renderer->tilemapProgram);
	// checkGlError(__LINE__, "");
	// setViewport(0, 0, spr->width, spr->height);

	// glEnableVertexAttribArray(renderer->tm_a_position);
	// setArrayBuffer(spr->vertexBuffer);
	// glVertexAttribPointer(renderer->tm_a_position, 2, GL_FLOAT, false, 0, 0);
	// checkGlError(__LINE__, "");

	// Matrix projectMat;
	// matrixIdentity(&projectMat);
	// checkGlError(__LINE__, "");
	// matrixProject(&projectMat, renderer->currentViewport.width, renderer->currentViewport.height);
	// // matrixScale(&projectMat, engineWidth/engineCamera.width,  engineHeight/engineCamera.height);
	// // if (sprite->scrolls) matrixTranslate(&projectMat, -engineCamera.x, -engineCamera.y);
	// glUniformMatrix3fv(renderer->tm_u_matrix, 1, false, (float *)(&projectMat.data));
	// checkGlError(__LINE__, "");

	// setTexture(tempTex, 0);
	// glUniform1i(renderer->tm_u_tilemapTexture, 0);

	// setTexture(tilesetTextureAsset->glTexture, 1);
	// glUniform1i(renderer->tm_u_tilesetTexture, 1);

	// glUniform2i(renderer->tm_u_tilemapSize, tilesWide, tilesHigh);
	// glUniform2i(renderer->tm_u_tilesetSize, tilesetTextureAsset->width, tilesetTextureAsset->height);
	// glUniform2i(renderer->tm_u_resultSize, (float)tilesWide*tileWidth, (float)tilesHigh*tileHeight);
	// glUniform2i(renderer->tm_u_pixelRatio, tileWidth, tileHeight);

	// glDrawArrays(GL_TRIANGLES, 0, 2*3);
	destroyTexture(tempTex);
	// checkGlError(__LINE__, "end of drawTilesFast");

}

void clearRenderer() {
	// setFramebuffer(0);
	// setViewport(0, 0, engine->width, engine->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void setGlViewport(int x, int y, int width, int height) {
	if (
		renderer->curViewport.x == x &&
		renderer->curViewport.y == y &&
		renderer->curViewport.width == width &&
		renderer->curViewport.height == height
	) return;

	glViewport(x, y, width, height);
	renderer->curViewport.setTo(x, y, width, height);
}

void setShaderProgram(GLuint program) {
	if (renderer->curShaderProgram == program) return;

	glUseProgram(program);
	renderer->curShaderProgram = program;
}

void setArrayBuffer(GLuint buffer) {
	if (renderer->curArrayBuffer == buffer) return;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	renderer->curArrayBuffer = buffer;
}

void changeArrayBuffer(GLuint buffer, float x, float y, float width, float height) {
	setArrayBuffer(buffer);

	float bufferData[12] = {
		x, y,
		width, y,
		width, height,
		x, y,
		x, height,
		width, height
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_DYNAMIC_DRAW);
}

GLuint genGlArrayBuffer(float x, float y, float width, float height) {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	changeArrayBuffer(buffer, x, y, width, height);
	return buffer;
}

void setTexture(GLuint texture) {
	if (renderer->currentTexture == texture) return;
	renderer->currentTexture = texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	CheckGlError();
}


void checkGlError(int lineNum) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		if (renderer->errorCount < 10) {
			log("Gl error: %x(%d) at line %d\n", err, err, lineNum);
			renderer->errorCount++;

			if (renderer->errorCount == 10) {
				log("Max gl errors exceeded, no more will be shown\n");
				break;
			}
		}
	}
}
