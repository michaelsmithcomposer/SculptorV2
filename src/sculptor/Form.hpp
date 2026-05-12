#pragma once

#include <Geode/Geode.hpp>
#include "../sculptor/Layer.hpp"
#include "../lib/Geometry.hpp"
#include "../sculptor/BezierEditor.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;
using namespace Sculptor;

class Form : public CCNode {
public:	

	std::vector<Layer*> layers;
	BezierEditor* bezierEditor;

	static Form* create(BezierCurves curves);
	bool init(BezierCurves curves);

	const Poly& getApproximation() const {		
		recalculateIfDirty();		
		return m_approximation;
	}
	const Clipper2Lib::PathsD& getDecomposition() const {
		recalculateIfDirty();
		return m_decomposition;
	}
	const RightTriangles& getTriangulation() const {
		recalculateIfDirty();
		return m_triangulation;
	}


	Layer* createLayer(const LayerStyle& style);
	void removeLayer(Layer* layer);

	void setDirty();
	void recalculateIfDirty(bool force = false) const;

private:

	
	BezierCurves curves;

	mutable bool dirty;	

	mutable Poly m_approximation;
	mutable Clipper2Lib::PathsD m_decomposition;
	mutable RightTriangles m_triangulation;

	void update(float dt);

	void startUpdate();

	void endUpdate();

	bool postUpdate;

	

	Poly calculateApproximation() const;
	Clipper2Lib::PathsD calculateDecomposition() const;
	RightTriangles calculateTriangulation() const;
	


};