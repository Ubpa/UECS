#pragma once

#include "CmptPtr.h"

namespace Ubpa::UECS {
	class SingletonsView {
	public:
		SingletonsView(const CmptAccessPtr* singletons, size_t num)
			: singletons{ singletons }, num{ num } {}

		CmptAccessPtr GetSingleton(CmptAccessType) const;

		const CmptAccessPtr* Singletons() const noexcept { return singletons; }
		size_t NumberOfSingletons() const noexcept { return num; }
	private:
		const CmptAccessPtr* singletons;
		size_t num;
	};
}
