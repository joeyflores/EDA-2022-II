// Copyright
#ifndef SRC_POINT_HPP_
#define SRC_POINT_HPP_

#include <cmath>
#include <iostream>

template <size_t N>
class Point {
public:
    typedef double* iterator;
    typedef const double* const_iterator;

    size_t size() const;

    double& operator[](size_t index);
    double operator[](size_t index) const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    friend std::ostream& operator<<(std::ostream& out, const Point& lhs) {
        out << "[ ";
        for (size_t i = 0; i < N - 1; ++i) {
            out << lhs[i] << " - ";
        }
        out << lhs[N - 1] << " ]";
        return out;
    }

private:
    double coords[N];
};

template <size_t N>
double distance(const Point<N>& one, const Point<N>& two);

template <size_t N>
bool operator==(const Point<N>& one, const Point<N>& two);

template <size_t N>
bool operator!=(const Point<N>& one, const Point<N>& two);

/** Point class implementation details */

#include <algorithm>

template <size_t N>
size_t Point<N>::size() const {
    return N;
}

template <size_t N>
double& Point<N>::operator[](size_t index) {
    return coords[index];
}

template <size_t N>
double Point<N>::operator[](size_t index) const {
    return coords[index];
}

template <size_t N>
typename Point<N>::iterator Point<N>::begin() {
    return coords;
}

template <size_t N>
typename Point<N>::const_iterator Point<N>::begin() const {
    return coords;
}

template <size_t N>
typename Point<N>::iterator Point<N>::end() {
    return begin() + size();
}

template <size_t N>
typename Point<N>::const_iterator Point<N>::end() const {
    return begin() + size();
}

template <size_t N>
double distance(const Point<N>& one, const Point<N>& two) {
    double result = 0.0;
    for (size_t i = 0; i < N; ++i) {
        result += (one[i] - two[i]) * (one[i] - two[i]);
    }
    return sqrt(result);
}

template <size_t N>
bool operator==(const Point<N>& one, const Point<N>& two) {
    return std::equal(one.begin(), one.end(), two.begin());
}

template <size_t N>
bool operator!=(const Point<N>& one, const Point<N>& two) {
    return !(one == two);
}

#endif  // SRC_POINT_HPP_