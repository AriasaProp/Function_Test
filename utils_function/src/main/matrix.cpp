#include "matrix.hpp"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>

template <unsigned H, unsigned V>
matrix<H,V>::matrix() {}
template <unsigned H, unsigned V>
matrix<H,V>::matrix(std::initializer_list<std::initializer_list<const float>> values) {
    if (values.size() != V) throw std::invalid_argument("Jumlah vertical tidak sesuai.");
    /*
    for(unsigned v = 0, h = 0; v < V; v++) {
        for (h = 0; h < H; h++) {
            this->data[v][h] = in[v][h];
        }
    }
    */
    for (unsigned v = 0; v < V; ++v) {
        if (values[v].size() != H) throw std::invalid_argument("Jumlah horizontal tidak sesuai.");
        for (unsigned h = 0; h < H; ++h) {
            data[v][h] = values[v][h];
        }
    }
}

template <unsigned H, unsigned V>
matrix<H,V>::~matrix() {}

template <unsigned H, unsigned V>
float &matrix<H,V>::operator[](unsigned l) { return data[l/V][l%V]; }
template <unsigned H, unsigned V>
float &matrix<H,V>::operator()(unsigned v, unsigned h) { return data[v][h]; }

template <unsigned H, unsigned V>
matrix<H,V> matrix<H,V>::operator+(const matrix<H,V>& other) {
    matrix<H,V> result;
    for (unsigned i = 0, j = 0; i < V; ++i) {
        for (j = 0; j < H; ++j) {
            result.data[i] = this->data[i][j] + other.data[i][j];
        }
    }
    return result;
}
template <unsigned H, unsigned V>
matrix<H,V> matrix<H,V>::operator*(const matrix<H,V>& other) {
    matrix<H,V> result;
    for (unsigned i = 0, j = 0, k = 0; i < H; ++i) {
        for (j = 0; j < V; ++j) {
            for (k = 0; k < V; ++k) {
                result(i, j) += this->data[i * V + k] * other.data[k * V + j];
            }
        }
    }
    return result;
}

template <unsigned H, unsigned V>
void matrix<H,V>::printInfo() {
    std::cout << std::endl;
    for (unsigned i = 0, j = 0; i < V; i++) {
        std::cout << "\n[";
        for (j = 0; j < H; j++) {
            std::cout << "| " << data[i][j] << " |";
        }
        std::cout << ']';
    }
    std::cout << std::endl;
}

template <unsigned H, unsigned V>
unsigned matrix<H,V>::number_of_digits (float &n) {
	std::ostringstream strs;
	strs << n;
	return strs.str().size();
}

template <unsigned H, unsigned V>
void matrix<H,V>::print_matrix () {
	unsigned max_len_each_vert[H] {};
	//find length each vertical
	for (unsigned j = 0; j < H; ++j) { //horizontal
		unsigned max_len {};

		for (unsigned i = 0; i < V; ++i) { //vertical
			if (const auto num_length {number_of_digits(this->data[i][j])}; num_length > max_len)
				max_len = num_length;
		}
		max_len_each_vert[j] = max_len;
	}

	for (unsigned i = 0, j = 0; i < V; ++i) {
		for (j = 0; j < H; ++j) {
			std::cout << (j == 0 ? "\n| " : "") << std::setw(max_len_each_vert[j]) << this->data[i][j] << (j == H - 1 ? " |" : " ");
    }
	}
	std::cout << '\n';
}

