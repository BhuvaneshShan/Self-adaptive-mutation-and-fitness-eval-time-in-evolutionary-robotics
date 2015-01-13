/*
 *  Distribution.h
 *  roborobo
 *
 *  Created by Cristian Dinu on 04/01/13.
 *
 *  This is a class for modeling a pool of items, each with an associated
 *  relative probability. The main method used is draw(), which draws an item
 *  from the pool according to the defined probabilities.
 */

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include "Utils/Rand.h"

#include <vector>
#include <algorithm>

using namespace std;

template <class T>
class Distribution {
public:
    Distribution();
    virtual ~Distribution();

    virtual size_t size();
    virtual T draw();
    virtual T drawFromInterval(size_t firstIndex, size_t lastIndex);

    virtual void clear();
    virtual void addItem(const T& item, double pseudo_probability);
protected:
    vector<double> cdf;
    vector<T> items;

    virtual double _totalCdf();
    virtual T _drawFromInterval(double minCdf, double maxCdf);
};

template <class T>
Distribution<T>::Distribution() {
	// nothing to do
}

template <class T>
Distribution<T>::~Distribution() {
	// nothing to do
}

template <class T>
size_t Distribution<T>::size() {
	return cdf.size();
}

template <class T>
T Distribution<T>::draw() {
	return _drawFromInterval(0.0, _totalCdf());
}

template <class T>
T Distribution<T>::drawFromInterval(size_t firstIndex, size_t lastIndex) {
	double minCdf = (firstIndex > 0) ? cdf[firstIndex-1] : 0.0;
	double maxCdf = (lastIndex < cdf.size()) ? cdf[lastIndex] : _totalCdf();

	return _drawFromInterval(minCdf, maxCdf);
}

template <class T>
void Distribution<T>::clear() {
	cdf.clear();
	items.clear();
}

template <class T>
void Distribution<T>::addItem(const T& item, double pseudo_probability) {
	cdf.push_back(_totalCdf() + pseudo_probability);
	items.push_back(item);
}

template <class T>
double Distribution<T>::_totalCdf() {
	return (cdf.size() > 0) ? cdf[cdf.size()-1] : 0.0;
}

template <class T>
T Distribution<T>::_drawFromInterval(double minCdf, double maxCdf) {
	double cursor = minCdf + (maxCdf-minCdf) * Rand::randouble();

	vector<double>::iterator it  = upper_bound(cdf.begin(), cdf.end(), cursor);
	size_t index = it - cdf.begin();
	if (index >= items.size()) {
		index = items.size() - 1;
	}

	return items[index];
}

#endif /* ENERGYDISTRIBUTION_H_ */
