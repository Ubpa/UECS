#pragma once

#include "CmptPtr.h"

namespace Ubpa::UECS {
	class SingletonsView {
	public:
		SingletonsView(const CmptPtr* singletons, size_t num)
			: singletons{ singletons }, num{ num } {}

		CmptPtr GetSingleton(CmptType) const;

		const CmptPtr* Singletons() const noexcept { return singletons; }
		size_t NumberOfSingletons() const noexcept { return num; }
	private:
		const CmptPtr* singletons;
		size_t num;
	};
}
