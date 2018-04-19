#include "mathTools.h"

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

void Rect::setTo(float x, float y, float width, float height) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
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
