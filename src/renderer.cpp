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

struct SpriteProgram {
};

struct CircleProgram {
	GLuint program;
	GLuint a_position;
	GLuint a_texCoord;
	GLuint u_colour;
	GLuint u_projection;
};

struct Renderer {
	GLuint spriteProgram;
	int errorCount;

	CircleProgram circleProgram;

	GLuint tempVerts;
	GLuint tempTexCoords;

	Rect curViewport;
	GLuint curShaderProgram;
	GLuint curArrayBuffer;
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
	renderer->circleProgram.a_texCoord = glGetAttribLocation(renderer->circleProgram.program, "a_texCoord");
	renderer->circleProgram.a_position = glGetAttribLocation(renderer->circleProgram.program, "a_position");
	renderer->circleProgram.u_colour = glGetUniformLocation(renderer->circleProgram.program, "u_colour");
	renderer->circleProgram.u_projection = glGetUniformLocation(renderer->circleProgram.program, "u_projection");
	free(circleFragStr);
	free(circleVertStr);

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
	CheckGlError();
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
