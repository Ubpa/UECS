#pragma once

#include "CmptPtr.h"
#include <UContainer/Span.h>

namespace Ubpa::UECS {
	class CmptsView {
	public:
		CmptsView() noexcept = default;
		CmptsView(Span<const CmptAccessPtr> cmpts) noexcept : cmpts{ cmpts } {}

		CmptAccessPtr GetCmpt(CmptAccessType) const noexcept;
		Span<const CmptAccessPtr> Components() const noexcept { return cmpts; }
	private:
		Span<const CmptAccessPtr> cmpts;
	};
}
