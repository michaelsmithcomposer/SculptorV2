#pragma once

#include <Geode/Geode.hpp>
#include "../sculptor/Layer.hpp"
#include "../lib/Geometry.hpp"
#include "../lib/Modulator.hpp"
#include "../sculptor/BezierEditor.hpp"
#include "../external/clipper2/clipper.h"

using namespace geode::prelude;

namespace Sculptor {

	class Form : public CCNode {
	public:

		CCPoint position;

		std::vector<Layer*> layers;
		std::vector<Modulator*> modulators;
		BezierEditor* bezierEditor;


		static Form* create(CCPoint position, BezierCurves curves);
		bool init(CCPoint position, BezierCurves curves);
		void onExit();

		const Poly& getApproximation() const {
			recalculateIfDirty();
			return m_approximation;
		}
		const Polys& getDecomposition() const {
			recalculateIfDirty();
			return m_decomposition;
		}
		const RightTriangles& getTriangulation() const {
			recalculateIfDirty();
			return m_triangulation;
		}

		Layer* addLayer(Layer* layer);
		void removeLayer(Layer* layer);

		Modulator* addModulator(Modulator* modulator);
		void removeModulator(Modulator* modulator);

		int getModulatorID(Modulator* modulator) {
			return std::ranges::find(modulators, modulator) - modulators.begin();
		}

		int getLayerID(Layer* layer) {
			return std::ranges::find(layers, layer) - layers.begin();
		}

		void setDirty();
		void recalculateIfDirty(bool force = false) const;

	private:




		mutable bool dirty;

		mutable Poly m_approximation;
		mutable Polys m_decomposition;
		mutable RightTriangles m_triangulation;

		void update(float dt);

		void startUpdate();

		void endUpdate();

		bool postUpdate;



		Poly calculateApproximation() const;
		Polys calculateDecomposition() const;
		RightTriangles calculateTriangulation() const;



	};
}