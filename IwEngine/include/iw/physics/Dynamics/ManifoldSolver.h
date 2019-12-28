#pragma once

#include "RigidbodySolver.h"

namespace IW {
namespace Physics {
	class ManifoldSolver
		: public DynamicSolver
	{
	public:
		IWPHYSICS_API
		void Solve(
			std::vector<Rigidbody*>& bodies,
			std::vector<Manifold>& manifolds,
			scalar dt) override;
	};
}

	using namespace Physics;
}
