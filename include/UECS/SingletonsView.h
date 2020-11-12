#pragma once

#include "CmptPtr.h"

#include <UContainer/Span.h>

namespace Ubpa::UECS {
	class SingletonsView {
	public:
		SingletonsView(Span<const CmptAccessPtr> singletons) noexcept
			: singletons{ singletons } {}

		CmptAccessPtr GetSingleton(CmptAccessType) const noexcept;
		Span<const CmptAccessPtr> Singletons() const noexcept { return singletons; }
	private:
		Span<const CmptAccessPtr> singletons;
	};
}
