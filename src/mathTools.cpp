#include <math.h>

#include "mathTools.h"

float roundToNearest(float num, float nearest) {
	return roundf(num / nearest) * nearest;
}

bool pointInRect(float px, float py, float rx, float ry, float rw, float rh) {
	return px >= rx && px <= rx+(rw-1) && py >= ry && py <= ry+(rh-1);
}

float rnd() {
	return (float)rand()/(float)RAND_MAX;
}

float rndFloat(float min, float max) {
	return min + rnd() * (max - min);
}

int rndInt(int min, int max) {
	return round((rndFloat(min, max)));
}

float distanceBetween(float x1, float y1, float x2, float y2) {
	float a = x2 - x1;
	float b = y2 - y1;

	float c = sqrt(a*a + b*b);
	return c;
}

float radsBetween(float x1, float y1, float x2, float y2) {
	return atan2(y2 - y1, x2 - x1);
}

float toDeg(float rads) {
	return rads * 57.2958;
}

float toRad(float degs) {
	return degs * 0.0174533;
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

bool Rect::containsPoint(Point *point) { return pointInRect(point->x, point->y, this->x, this->y, this->width, this->height); }
bool Rect::containsPoint(float px, float py) { return pointInRect(px, py, this->x, this->y, this->width, this->height); }


float Rect::distanceToPerimeter(float px, float py) {
	Rect *rect = this;
	float l = rect->x;
	float t = rect->y;
	float r = rect->x + rect->width;
	float b = rect->y + rect->height;

	float x = Clamp(px, l, r);
	float y = Clamp(py, t, b);

	float dl = fabs(x-l);
	float dr = fabs(x-r);
	float dt = fabs(y-t);
	float db = fabs(y-b);

	float m;
	m = Min(dl, dr);
	m = Min(m, dt);
	m = Min(m, db);

	float minX;
	float minY;

	if (m == dt) {
		minX = x;
		minY = t;
	} else if (m == db) {
		minX = x;
		minY = b;
	} else if (m == dl) {
		minX = l;
		minY = y;
	} else {
		minX = r;
		minY = y;
	}

	return distanceBetween(minX, minY, px, py);
}

void Rect::randomPoint(Point *point) {
	point->x = rndFloat(this->x, this->x + this->width);
	point->y = rndFloat(this->y, this->y + this->height);
}

void Rect::print() {
	printf("Rect: {%f, %f, %f, %f}\n", this->x, this->y, this->width, this->height);
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


void Matrix::scale(float x, float y) {
	float array[9] = {
		x, 0, 0,
		0, y, 0,
		0, 0, 1
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

void Point::normalize(float scale) {
	float norm = sqrt(this->x*this->x + this->y*this->y);
	if (norm != 0) {
		this->x = scale * this->x / norm;
		this->y = scale * this->y / norm;
	}
}
