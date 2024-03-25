#pragma once
#include "utils.h"

class interval
{
public:
	double min;
	double max;

	interval() : min(+infinity), max(-infinity) {} // default = empty interval	interval() : min(+infinity), max(-infinity) // default = empty interval
	interval(double min, double max) : min(min), max(max) {} 
	interval(const interval& a, const interval& b) : min(fmin(a.min, b.min)), max(fmax(a.max, b.max)) {}
	
	bool contains(double t) const
	{
		return min <= t && t <= max;
	}

	bool surrounds(double t) const 
	{
		return min < t && t < max;
	}

	double clamp(double x) const
	{
		if (x < min) return min;
		if (x > max) return max;
		return x;
	}

	double size() const
	{
		return max - min;
	}

	interval expand(double delta) const
	{
		double padding = delta * 0.5;
		return interval(min - padding, max + padding);
	}

	

	static const interval empty, universe;
};


const static interval empty;
const static interval universe(-infinity, +infinity);
