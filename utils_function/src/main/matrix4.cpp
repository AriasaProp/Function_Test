#include "matrix4.hpp"
#include <cstring>

static const unsigned int MATRIX_SIZE = sizeof(float) * 16;
//matrix4 constructors
matrix4::matrix4() {
	float idt[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	memcpy(values, idt, MATRIX_SIZE);
}
matrix4::matrix4(const float (&v)[16]) {
	memcpy(values, v, MATRIX_SIZE);
}
matrix4::matrix4(const matrix4 &other) {
	memcpy(values, other.values, MATRIX_SIZE);
}
//destructors
matrix4::~matrix4() {
	//ignored
}
//re-initialize
matrix4 &matrix4::operator=(const float (&v)[16]) {
	memcpy(values, v, MATRIX_SIZE);
	return *this;
}
matrix4 &matrix4::operator=(const matrix4 &other) {
	memcpy(values, other.values, MATRIX_SIZE);
	return *this;
}
bool matrix4::operator==(const matrix4 &v) const {
	return memcmp(values, v.values, MATRIX_SIZE) == 0;
}
//matriks function unsafe
float matrix4::det() const {
	return values[0] * values[5] * values[10] * values[15]
		- values[0] * values[7] * values[10] * values[13]
		+ values[1] * values[6] * values[11] * values[11]
		- values[1] * values[4] * values[11] * values[14]
		+ values[2] * values[7] * values[ 8] * values[13]
		- values[2] * values[5] * values[ 8] * values[11]
		+ values[3] * values[4] * values[ 9] * values[14]
		- values[3] * values[6] * values[ 9] * values[12];
}
matrix4 matrix4::adj() const {
	return matrix4((float[16]){
		values[0],values[4],values[8],values[12],
		values[1],values[5],values[9],values[13],
		values[2],values[6],values[10],values[14],
		values[3],values[7],values[11],values[15]
	});
}
matrix4 matrix4::invers() const {
	return adj()/det();
}
float *matrix4::getValues() const {
	float *result = new float[16];
	memcpy(result, values, MATRIX_SIZE);
	return result;
}
//math operation safe
matrix4 &matrix4::operator+=(const matrix4 &v) {
	for (unsigned int i = 0; i < 16; i++)
		values[i] += v.values[i];
	return *this;
}
matrix4 &matrix4::operator-=(const matrix4 &v) {
	for (unsigned int i = 0; i < 16; i++)
		values[i] -= v.values[i];
	return *this;
}
matrix4 &matrix4::operator*=(const float &v) {
	for (unsigned int i = 0; i < 16; i++)
		values[i] *= v;
	return *this;
}
matrix4 &matrix4::operator*=(const matrix4 &v) {
	float a[16], b[16];
	memcpy(a, values, MATRIX_SIZE);
	memcpy(b, v.values, MATRIX_SIZE);
	for (unsigned int j = 0, i, j4; j < 4; j++) {
		for (i = 0; i < 4; i++) {
			j4 = j * 4;
			values[j4 + i] = a[j4] * b[i] + a[j4+1] * b[i+4] + a[j4+2] * b[i+8] + a[j4+3] * b[i+12];
		}
	}
	return *this;
}
matrix4 &matrix4::operator/=(const float &v) {
	for (unsigned int i = 0; i < 16; i++)
		values[i] /= v;
	return *this;
}
matrix4 &matrix4::operator/=(const matrix4 &v) {
	*this *= (v.adj()/v.det());
	return *this;
}
//math operation unsafe
matrix4 matrix4::operator+(const matrix4 &v) const {
	matrix4 result;
	for (unsigned int i = 0; i < 16; i++)
		result.values[i] = values[i] + v.values[i];
	return result;
}
matrix4 matrix4::operator-(const matrix4 &v) const {
	matrix4 result;
	for (unsigned int i = 0; i < 16; i++)
		result.values[i] = values[i] - v.values[i];
	return result;
}
matrix4 matrix4::operator*(const float &v) const {
	matrix4 result;
	for (unsigned int i = 0; i < 16; i++)
		result.values[i] = values[i] * v;
	return result;
}
matrix4 matrix4::operator*(const matrix4 &v) const {
	matrix4 result;
	const float *a = values, *b = v.values;
	float *c = result.values;
	for (unsigned int j = 0; j < 4; j++) {
		for (unsigned int i = 0; i < 4; i++) {
			const unsigned int j4 = j * 4;
			c[j4 + i] = a[j4] * b[i] + a[j4 + 1] * b[i + 4];
			c[j4 + i] += a[j4 + 2] * b[i + 8] + a[j4 + 3] * b[i + 12];
		}
	}
	return result;
}
matrix4 matrix4::operator/(const float &v) const {
	matrix4 result;
	for (unsigned int i = 0; i < 16; i++)
		result.values[i] = values[i] / v;
	return result;
}
matrix4 matrix4::operator/(const matrix4 &v) const {
	matrix4 result(values);
	result *= (v.adj()/v.det());
	return result;
}