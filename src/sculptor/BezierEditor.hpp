#pragma once

#include <Geode/Geode.hpp>
#include "../lib/Geometry.hpp"
#include "../lib/DragNode.hpp"

using namespace geode::prelude;
using namespace Sculptor;

class Form;

class BezierEditor : public CCDrawNode {
public:

	static BezierEditor* create(Form* form, BezierCurves curves);
	bool init(Form* form, BezierCurves curves);

	Form* form;

	const Poly getApproximation() const;
	
	void setDirty();
	void setCurveDirty(int i);

	void setCurves(BezierCurves curves) {
		this->curves = std::move(curves);		
		setDirty();
	}
	const BezierCurves& getCurves() const { return curves; }
	BezierCurves& getCurves() { return curves; }

	void setCurve(int index, BezierCurve curve) {
		this->curves[index] = std::move(curve);
		setCurveDirty(index);
	}
	const BezierCurve& getCurve(int i) const { return curves.at(i); }

	void setPoint(int curveIndex, int pointIndex, CCPoint point) {
		this->curves[curveIndex][pointIndex] = std::move(point);
		setCurveDirty(curveIndex);
	}
	const CCPoint& getPoint(int curveIndex, int pointIndex) const { return curves.at(curveIndex).points.at(pointIndex); }

	void addCurve(int i, BezierCurve curve);
	void removeCurve(int i);

	void addPoint(int curveIndex, int pointIndex, CCPoint point);
	void removePoint(int curveIndex, int pointIndex);

	int getNodeIndex(DragNode* node);
	int curvePointToNodeIndex(int curveIndex, int pointIndex);
	std::vector<std::pair<int, int>> nodeToCurvePointIndices(int i);
	
	void recalculateIfDirty(bool force = false);

private:	

	BezierCurves curves;

	std::vector<DragNode*> nodes;
	DragNode* UINode;	

	std::vector<bool> dirty;

	mutable Sequences m_approximation;	

	bool postUpdate;
	static constexpr float visibilityRadius = 4.25f;

	void update(float dt);
	void startUpdate();
	void endUpdate();

	void redraw();	
	void redrawIfDirty();		
	void rebuildNodes();

	DragNode* addNode(CCPoint position);
	void removeNode(DragNode* node);
	void removeAllNodes();

	DragNode* addUINode();
	void removeUINode();
	void updateUINode();

	

	
};