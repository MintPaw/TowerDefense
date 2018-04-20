#include <math.h>

#include "mathTools.h"

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))
#define PI 3.14159

float roundToNearest(float num, float nearest) {
	return roundf(num / nearest) * nearest;
}

void Rect::setTo(float x, float y, float width, float height) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

bool Rect::intersects(Rect *other) {
	float r1x = this->x;
	float r1y = this->y;
	float r1w = this->width;
	float r1h = this->height;

	float r2x = other->x;
	float r2y = other->y;
	float r2w = other->width;
	float r2h = other->height;

	return r1x < r2x + r2w &&
		r1x + r1w > r2x &&
		r1y < r2y + r2h &&
		r1h + r1y > r2y;

	bool intercects = !(
		r2x > r1x+r1w ||
		r2x+r2w < r1x ||
		r2y > r1y+r1h ||
		r2y+r2h < r1y);

	//@cleanup Not sure this contains formula does what I think it does
	bool contains = 
		(r1x+r1w) < (r1x+r1w) &&
		(r2x) > (r1x) &&
		(r2y) > (r1y) &&
		(r2y+r2h) < (r1y+r1h);

	return intercects || contains;
}

bool Rect::containsPoint(Point *point) { return this->containsPoint(point->x, point->y); }
bool Rect::containsPoint(float px, float py) {
	float rx = this->x;
	float ry = this->y;
	float rw = this->width;
	float rh = this->height;
	return px >= rx && px <= rx+(rw-1) && py >= ry && py <= ry+(rh-1);
}

void Matrix::identity() {
	this->setTo(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
}

void Matrix::project(float width, float height) {
	float array[9] = {
		2/width,  0,          0,
		0,        -2/height,  0,
		-1,       1,          1
	};
	this->multiply(array);
}

void Matrix::setTo(float *array) {
	this->data[0] = array[0];
	this->data[1] = array[1];
	this->data[2] = array[2];
	this->data[3] = array[3];
	this->data[4] = array[4];
	this->data[5] = array[5];
	this->data[6] = array[6];
	this->data[7] = array[7];
	this->data[8] = array[8];
}

void Matrix::setTo(float a, float b, float c, float d, float e, float f, float g, float h, float i) {
	this->data[0] = a;
	this->data[1] = b;
	this->data[2] = c;
	this->data[3] = d;
	this->data[4] = e;
	this->data[5] = f;
	this->data[6] = g;
	this->data[7] = h;
	this->data[8] = i;
}

void Matrix::translate(float x, float y) {
	float array[9] = {
		1, 0, 0,
		0, 1, 0,
		x, y, 1
	};
	this->multiply(array);
}

void Matrix::rotate(float deg) {
	float s = sin(deg*PI/180);
	float c = cos(deg*PI/180);
	float array[9] = {
		c, -s, 0,
		s,  c, 0,
		0,  0, 1
	};
	this->multiply(array);
}

void Matrix::multiply(float *array) {
	float temp[9] = {};

	temp[0] += this->data[0] * array[0] + this->data[3] * array[1] + this->data[6] * array[2];
	temp[1] += this->data[1] * array[0] + this->data[4] * array[1] + this->data[7] * array[2];
	temp[2] += this->data[2] * array[0] + this->data[5] * array[1] + this->data[8] * array[2];
	temp[3] += this->data[0] * array[3] + this->data[3] * array[4] + this->data[6] * array[5];
	temp[4] += this->data[1] * array[3] + this->data[4] * array[4] + this->data[7] * array[5];
	temp[5] += this->data[2] * array[3] + this->data[5] * array[4] + this->data[8] * array[5];
	temp[6] += this->data[0] * array[6] + this->data[3] * array[7] + this->data[6] * array[8];
	temp[7] += this->data[1] * array[6] + this->data[4] * array[7] + this->data[7] * array[8];
	temp[8] += this->data[2] * array[6] + this->data[5] * array[7] + this->data[8] * array[8];

	this->setTo(temp);
}

void Point::setTo(float x, float y) {
	this->x = x;
	this->y = y;
}
