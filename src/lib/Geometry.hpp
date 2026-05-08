#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace Sculptor {

	struct Sequence {

		std::vector<CCPoint> points;

		auto begin() { return points.begin(); }
		auto end() { return points.end(); }
		auto size() { return points.size(); }
		CCPoint& operator[](int i) { return points[i]; }
		void push_back(CCPoint point) { points.push_back(point); }

	};

	struct Polygon : Sequence {



	};

	struct Triangle : Polygon {

	};

	struct rightTriangle : Triangle {

	};


}
