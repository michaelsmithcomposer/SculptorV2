#pragma once

#include <Geode/Geode.hpp>
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;

namespace Sculptor {
	
	CCPoint moveTowards(CCPoint point, CCPoint target, float distance);

	using Points = std::vector<CCPoint>;

	struct Line {
	public:
		CCPoint a, b;

		Line(CCPoint a, CCPoint b) : a(a), b(b) {}

		float angle() const;
		float length() const;
		std::optional<float> gradient() const;

		bool contains(const CCPoint& point) const;
		std::optional<CCPoint> evaluate(float x) const;
		CCPoint projectionOf(const CCPoint& point) const;
		CCPoint normal() const;
		CCPoint lerp(float t) const {
			return a.lerp(b, t);
		}
		CCPoint midpoint() const {
			return lerp(0.5);
		}
		std::optional<float> inverseLerp(CCPoint) const;

		bool coincident(const Line& other) const;
		void shrink(float distance);
		void shrinkProportional(float amount);

	private:			
		mutable std::optional<float> m_angle;
		mutable std::optional<float> m_length;
		mutable std::optional<float> m_gradient;
	};

	using Lines = std::vector<Line>;

	struct Sequence {

		Points points;

		Sequence() = default;
		Sequence(Points points) : points(std::move(points)) {}
		Sequence(std::initializer_list<CCPoint> points) : points(points) {}

		auto begin() const { return points.begin(); }
		auto end() const { return points.end(); }
		auto size() const { return points.size(); }
		auto front() const { return points.front(); }
		auto back() const { return points.back(); }
		CCPoint& operator[](int i) { return points[i]; }		
		void push_back(CCPoint point) { points.push_back(point); }

		float length() const;
		Clipper2Lib::PathD asPath() const;
		static Sequence fromPath(Clipper2Lib::PathD path);		
		Lines edges() const;	

		bool containsPoint(const CCPoint& point) const { return !edgeIndicesContaining(point).empty(); }
		std::vector<int> edgeIndicesContaining(const CCPoint& point) const;

		std::optional<float> inverseLerp(const CCPoint& point) const;

		CCRect boundingBox() const;

		static Sequence createDashedLine(const Line& line, float spacing, float width);

	};

	using Sequences = std::vector<Sequence>;

	struct BezierCurve : Sequence {

		static constexpr int approximationDepth = 4;
		static constexpr int arcLengthSamples = 100;
		static constexpr float approximationSpacing = 15;

		CCPoint evaluate(float t) const;

		float curvature() const {
			return length() / points.front().getDistance(points.back());
		}

		std::pair<BezierCurve, BezierCurve> split(float t) const;

		Sequence approximateRecursive() const;

		Sequence approximateUniform() const;

		Sequence approximateUniformT() const;

	};

	using BezierCurves = std::vector<BezierCurve>;

	struct Poly : Sequence {

		bool containsEdge(const Line& edge) const { return edgeIndex(edge).has_value(); }
		std::optional<int> edgeIndex(const Line& edge) const;

		bool containsEdgePoint(const CCPoint& point) const { return !edgeIndicesContaining(point).empty(); }
		std::vector<int> edgeIndicesContaining(const CCPoint& point) const;		

		bool contains(const CCPoint& point) const;

		Lines edges() const;
		static Poly fromBezierCurves(BezierCurves curves);

		CCPoint normalAt(const CCPoint& point) const;
		CCPoint projectionOf(const CCPoint& point) const;
	};

	struct RightTriangle;
	struct RightTrianglePair;

	struct Triangle : Poly {
	public:

		static std::optional<Triangle> create(CCPoint a, CCPoint b, CCPoint c) {
			Triangle triangle = Triangle(a, b, c);
			if (triangle.isDegenerate()) {
				return std::nullopt;
			}
			return triangle;
		}

		void standardize();

		bool isRight();

		bool isDegenerate();

		RightTrianglePair orthogonalize();

		CCPoint& a() { return points.at(0); };
		const CCPoint& a() const { return points.at(0); };
		CCPoint& b() { return points.at(1); };
		const CCPoint& b() const { return points.at(1); };
		CCPoint& c() { return points.at(2); };
		const CCPoint& c() const { return points.at(2); };

		float ab() const { return a().getDistance(b()); };
		float bc() const { return b().getDistance(c()); };
		float ca() const { return c().getDistance(a()); };


	protected:

		Triangle() = default;
		Triangle(CCPoint a, CCPoint b, CCPoint c) {
			push_back(a);
			push_back(b);
			push_back(c);
			standardize();
		}

	};

	struct RightTriangle : Triangle {

		RightTriangle(CCPoint a, CCPoint b, CCPoint c)
			: Triangle(a, b, c) {
			//assert(isRight());
			makeClockwise();
		}

		Line hypotenuse() const { return Line(a(), c()); }

		void makeClockwise();

		float antialiasingWidth() const;

	};

	using RightTriangles = std::vector<RightTriangle>;

	struct RightTrianglePair {
		RightTriangle left;
		RightTriangle right;
	};	



	

}

template <>
struct fmt::formatter<Sculptor::Line> {
	constexpr auto parse(fmt::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Sculptor::Line& line, fmt::format_context& ctx) const {			
		fmt::format_to(ctx.out(), "[{:.4f}, {:.4f} -> {:.4f}, {:.4f}]", line.a.x, line.a.y, line.b.x, line.b.y);		
		return ctx.out();
	}
};

template <>
struct fmt::formatter<Sculptor::Sequence> {
	constexpr auto parse(fmt::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Sculptor::Sequence& seq, fmt::format_context& ctx) const {
		fmt::format_to(ctx.out(), "Sequence[{}]: ", seq.points.size());
		for (const auto& p : seq.points) {
			fmt::format_to(ctx.out(), "({:.4f}, {:.4f}) ", p.x, p.y);
		}
		return ctx.out();
	}
};