#pragma once

#include <Geode/Geode.hpp>
#include <numbers>

using namespace geode::prelude;

static const double PI = std::numbers::pi;

CCPoint polar(float radius, float theta);

float lerp(float a, float b, float mix);

float inverseLerp(float a, float b, float t);

float clamp(float value, float a, float b);

float roundTo(float value, int places);


template <class T> std::vector<std::pair<T, T>> pairwise(std::vector<T> 
) {
	std::vector<std::pair<T, T>> pairs;
	for (int i = 0; i < sequence.size(); i++) {		
		pairs.push_back(std::make_pair(sequence.at(i), sequence.at((i + 1) % sequence.size())));
	}
	return pairs;
}
