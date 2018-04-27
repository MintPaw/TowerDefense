#pragma once

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))
#define PI 3.14159
#define ArrayLength(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

struct Rect;
struct Matrix;
struct Point;

float roundToNearest(float num, float nearest=1);
bool pointInRect(float px, float py, float rx, float ry, float rw, float rh);

float rnd();
float rndFloat(float min, float max);
int rndInt(int min, int max);
bool rndBool();

float distanceBetween(float x1, float y1, float x2, float y2);
float radsBetween(float x1, float y1, float x2, float y2);
float toDeg(float rads);
float toRad(float degs);

struct Rect {
	float x;
	float y;
	float width;
	float height;

	void setTo(float x=0, float y=0, float width=0, float height=0);
	bool intersects(Rect *other);
	bool containsPoint(Point *point);
	bool containsPoint(float px, float py);
	void randomPoint(Point *point);
	void print();
};

struct Matrix {
	float data[9];

	void setTo(float a, float b, float c, float d, float e, float f, float g, float h, float i);
	void setTo(float *array);

	void identity();
	void project(float width, float height);
	void multiply(float *array);
	void translate(float x, float y);
	void rotate(float deg);
	void scale(float x, float y);
};

struct Point {
	float x;
	float y;

	void setTo(float x=0, float y=0);
	void normalize(float scale=1);
};
