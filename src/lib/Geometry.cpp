#include <Geode/Geode.hpp>
#include <ranges>
#include "../lib/geometry.hpp"
#include "../lib/Interface.hpp"
#include "../lib/utilities.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;

namespace Sculptor {

	CCPoint moveTowards(CCPoint point, CCPoint target, float distance) {
		return point.lerp(target, distance / point.getDistance(target));
	}
	  
	float Line::angle() const {
		if (!m_angle) {
			CCPoint norm = (b - a).normalize();
			m_angle = atan2(norm.y, norm.x);
		}
		return *m_angle;
	}

	float Line::length() const {
		if (!m_length) {
			m_length = a.getDistance(b);
		}
		return *m_length;
	}

	std::optional<float> Line::gradient() const {
		if (!m_gradient && (a.x != b.x)) {
			m_gradient = (b.y - a.y) / (b.x - a.x);					
		}
		return m_gradient;	
	}

	bool Line::contains(const CCPoint& point) const {
		if (a.x == b.x) {
			return isClose(point.x, a.x) && rangeContains(a.y, b.y, point.y);
		}
		else {
			std::optional<CCPoint> result = evaluate(point.x);
			return (result.has_value()) && (isClose(result.value().y, point.y));
		}
	}

	std::optional<CCPoint> Line::evaluate(float x) const {
		if (a.x != b.x) {
			float y = *gradient() * (x - a.x) + a.y;
			if (rangeContains(a.x, b.x, x)) {
				return ccp(x, y);
			}
		}
		return std::nullopt;
	}

	CCPoint Line::projectionOf(const CCPoint& point) const {
		if (a.x == b.x) {
			return ccp(a.x, clamp(point.y, a.y, b.y));
		}
		else {
			CCPoint projection = (point - b).project(a - b) + b;
			float x = std::clamp(projection.x, std::min(a.x, b.x), std::max(a.x, b.x));
			return *evaluate(x);
		}
	}

	bool Line::coincident(const Line& other) const {
		return (isClose(a, other.a) && isClose(b, other.b)) || (isClose(a, other.b) && isClose(b, other.a));
	}

	CCPoint Line::normal() const {
		CCPoint dir = (b - a).normalize();
		return ccp(dir.y, -dir.x);
	}

	void Line::shrink(float distance) {
		auto _a = a;
		auto _b = b;
		a = moveTowards(_a, _b, distance);
		b = moveTowards(_b, _a, distance);
	}

	void Line::shrinkProportional(float amount) {
		auto _a = a;
		auto _b = b;
		a = _a.lerp(_b, amount);
		b = _b.lerp(_a, amount);
	}

	//

	float Sequence::length() const {
		float l = 0;
		for (const auto& edge : edges()) {
			l += edge.length();
		}
		return l;
	}

	Clipper2Lib::PathD Sequence::asPath() const {
		Clipper2Lib::PathD path;
		for (const auto& point : points) {
			path.push_back({ point.x, point.y });
		}
		return path;
	}

	Sequence Sequence::fromPath(Clipper2Lib::PathD path) {
		Sequence sequence;
		for (const auto& point : path) {
			sequence.push_back(ccp( point.x, point.y ));
		}
		return sequence;
	}	

	CCRect Sequence::boundingBox() const {
		auto rect = Clipper2Lib::GetBounds(asPath());
		return { static_cast<float>(rect.left), static_cast<float>(rect.bottom), static_cast<float>(rect.right - rect.left), static_cast<float>(rect.top - rect.bottom) };
	}

	Lines Sequence::edges() const {
		Lines lines;
		for (const auto& [i, point] : std::views::enumerate(points | std::views::take(points.size() - 1))) {
			lines.push_back(Line(point, points[i + 1]));
		}
		return lines;
	}		

	Sequence Sequence::createDashedLine(const Line& line, float spacing, float width) {
		Sequence sequence;
		float count = std::floor(line.length() / spacing);
		for (int i = 0; i < count; i++) {
			sequence.push_back(line.lerp(i / count));
			sequence.push_back(line.lerp((i + width) / count));
		}
		return sequence;
	}

	//

	Lines Poly::edges() const {
		Lines lines;
		for (const auto& [i, point] : std::views::enumerate(points)) {
			lines.push_back(Line(point, points[(i + 1) % points.size()]));
		}		
		return lines;
	}

	Poly Poly::fromBezierCurves(BezierCurves curves) {
		Poly poly;
		for (const auto& curve : curves) {
			for (const auto& point : curve | std::views::take(curve.size() - 1)) {
				poly.push_back(point);
			}
		}
		return poly;
	}

	std::optional<int> Poly::edgeIndex(const Line& line) const {		
		for (const auto& [i, edge] : std::views::enumerate(edges())) {			
			if (edge.coincident(line)) {
				return i;
			}
		}
		return std::nullopt;
	}

	std::vector<int> Poly::edgeIndicesContaining(const CCPoint& point) const {
		std::vector<int> result;
		for (const auto& [i, line] : std::views::enumerate(edges())) {
			if (line.contains(point)) {
				result.push_back(i);
			}
		}
		return result;
	}

	CCPoint Poly::normalAt(const CCPoint& point) const {
		CCPoint normal;
		for (const auto& edge : edges()) {			
			normal += edge.normal() * (1 / (1e-6f + point.getDistance(edge.projectionOf(point))));
		}
		return normal.normalize();
	}

	CCPoint Poly::projectionOf(const CCPoint& point) {	
		auto e = edges();
		auto closestEdge = std::ranges::min_element(e, {}, [point](const Line& edge) { return point.getDistance(edge.projectionOf(point)); });
		return closestEdge->projectionOf(point);
	}

	//	

	void Triangle::standardize() {
		if (ab() > bc() && ab() > ca()) {
			std::swap(b(), c());
		}
		else if (bc() > ca() && bc() > ab()) {
			std::swap(a(), b());
		}
	}

	bool Triangle::isRight() {
		CCPoint ab = (a() - b()).normalize();		
		CCPoint bc = (b() - c()).normalize();
		CCPoint ca = (c() - a()).normalize();				
		return fabsf(ab.dot(bc)) < 0.01f || fabsf(bc.dot(ca)) < 0.01f || fabsf(ca.dot(ab)) < 0.01f;
	}

	bool Triangle::isDegenerate() {
		CCPoint ab = (b() - a()).normalize();
		CCPoint ac = (c() - a()).normalize();		
		return isClose(fabsf(ab.cross(ac)), 0);
	}

	RightTrianglePair Triangle::orthogonalize() {
		CCPoint projection = Line(a(), c()).projectionOf(b());
		return RightTrianglePair{ RightTriangle(a(), projection, b()), RightTriangle(c(), projection, b())};
	}

	void RightTriangle::makeClockwise() {
		CCPoint ab = b() - a();
		CCPoint ac = c() - a();
		double cross = (double)ab.x * ac.y - (double)ab.y * ac.x;
		if (cross > 0) {
			std::swap(a(), c());
		}
	}

	float RightTriangle::antialiasingWidth() const {	
		float w = (ab() * bc()) / ca();
		float scaledW = (w / gdUnit) * 0.5;
		return clamp(scaledW, 0.01, 3);
	}

	CCPoint BezierCurve::evaluate(float t) const {
		Points curve = points;

		int n = curve.size();
		for (int order = 1; order < n; order++) {
			for (int i = 0; i < n - order; i++) {
				curve[i] = curve[i].lerp(curve[i + 1], t);
			}
			if (order == n - 1) {				
				return curve[0];
			}
		}
	}

	std::pair<BezierCurve, BezierCurve> BezierCurve::split(float t) const {
		Points curve = points;
		Points left, right;

		left.push_back(curve.front());
		right.push_back(curve.back());

		int n = curve.size();
		for (int order = 1; order < n; order++) {
			for (int i = 0; i < n - order; i++) {
				curve[i] = curve[i].lerp(curve[i + 1], t);
			}
			left.push_back(curve.front());
			right.push_back(curve[n - order - 1]);
		}

		std::ranges::reverse(right);
		return std::make_pair(BezierCurve(left), BezierCurve(right));
	}

	Sequence BezierCurve::approximateRecursive() const {

		if (size() == 2) {
			return { points };
		}		

		BezierCurves curves { *this };
		BezierCurves nextCurves;

		int depth = approximationDepth;
		while (depth--) {
			nextCurves.clear();
			for (const auto& curve : curves) {
				auto [left, right] = curve.split(0.5);
				nextCurves.push_back(left);
				nextCurves.push_back(right);
			}
			curves = nextCurves;
		}

		Sequence result;
		for (auto& curve : curves) {
			result.push_back(curve[0]);
		}
		result.push_back(curves.back().back());

		return result;

	}

	Sequence BezierCurve::approximateUniform() const {
		if (size() == 2) {
			return { points };
		}

		Points arcPoints;
		std::vector<float> arcLengths;			
		for (int i = 0; i < arcLengthSamples; i++) {			
			float t = static_cast<float>(i) / arcLengthSamples;
			auto point = evaluate(t);				
			arcPoints.push_back(point);			
			auto lastPoint = (i == 0) ? ccp(0, 0) : arcPoints[i - 1];		
			arcLengths.push_back( (i == 0) ? 0 : arcLengths[i - 1] + lastPoint.getDistance(point) );			
			
		}

		Points result;
		result.push_back(points.front());
		float length = arcLengths.back();
		int count = floor(length / approximationSpacing);
		for (int s = 1; s < count; s++) {
			float segmentLength = length * (static_cast<float>(s) / count);			
			for (const auto& [i, l] : std::views::enumerate(arcLengths)) {				
				if (l >= segmentLength) {					
					result.push_back(arcPoints[i]);
					break;
				}
			}
		}
		result.push_back(points.back());

		return { result };
	}

	Sequence BezierCurve::approximateUniformT() const {
		if (size() == 2) {
			return { points };
		}
		Sequence result;
		result.push_back(points.front());		
		int count = clamp(floor(curvature() * approximationSpacing), 5, 15);		
		for (int i = 1; i < count; i++) {
			float t = static_cast<float>(i) / count;
			result.push_back(evaluate(t));
		}
		result.push_back(points.back());
		return result;
	}

}