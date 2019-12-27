#pragma once

#include "RigidbodySolver.h"
#include <vector>

namespace IW {
namespace Physics {
	class ManifoldSolver
		: public RigidbodySolver
	{
	public:
		IWPHYSICS_API
		void Solve(
			std::vector<Rigidbody*>& bodies,
			scalar dt) override;
	};
}

	using namespace Physics;
}