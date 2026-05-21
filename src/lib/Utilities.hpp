#pragma once

#include <Geode/Geode.hpp>
#include <numbers>

using namespace geode::prelude;

namespace Sculptor {

	static constexpr float PRECISION_HIGH = 0.01f;
	static constexpr float PRECISION_LOW = 0.01f;

	static const double PI = std::numbers::pi;

	CCPoint polar(float radius, float theta);

	float lerp(float a, float b, float mix);

	float inverseLerp(float a, float b, float t);

	float clamp(float value, float a, float b);

	float roundTo(float value, int places);

	CCPoint roundTo(CCPoint value, int places);

	bool isClose(float a, float b, float precision = PRECISION_HIGH);

	bool isClose(CCPoint a, CCPoint b, float precision = PRECISION_HIGH);

	bool rangeContains(float start, float end, float value);

	int safeModulo(int a, int b);

	float msBetween(const std::chrono::steady_clock::time_point& start, const std::chrono::steady_clock::time_point& end);



}