#include <Geode/Geode.hpp>
#include "../lib/utilities.hpp"

using namespace geode::prelude;

CCPoint polar(float radius, float theta) {
    return ccp(radius * cos(theta), radius * sin(theta));
}

float lerp(float a, float b, float mix) {
	return a + (b - a) * mix;
}

float inverseLerp(float a, float b, float t) {
	return (t - a) / (b - a);
}

float clamp(float value, float a, float b) {
	float min = std::min(a, b);
	float max = std::max(a, b);
	return std::clamp(value, min, max);
}

float roundTo(float value, int places) {
	double factor = std::pow(10.0, places);
	return std::round(value * factor) / factor;
}

CCPoint roundTo(CCPoint value, int places) {
	double factor = std::pow(10.0, places);
	return ccp(roundTo(value.x, places), roundTo(value.y, places));
}

bool isClose(float a, float b, float precision) {
	return std::fabs(a - b) < precision;
}

bool isClose(CCPoint a, CCPoint b, float precision) {
	return isClose(a.x, b.x, precision) && isClose(a.y, b.y, precision);	
}

bool rangeContains(float start, float end, float value) {
	float min = std::min(start, end);
	float max = std::max(start, end);
	return ((value > min) || isClose(value, min)) && ((value < max) || (isClose(value, max)));
}
