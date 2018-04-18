#pragma once

struct Rect {
	float x;
	float y;
	float width;
	float height;

	void setTo(float x=0, float y=0, float width=0, float height=0);
};

struct Matrix {
	float data[9];

	void setTo(float a, float b, float c, float d, float e, float f, float g, float h, float i);
	void setTo(float *array);

	void identity();
	void project(float width, float height);
	void multiply(float *array);
};

struct Point {
	float x;
	float y;

	void setTo(float x=0, float y=0);
};
