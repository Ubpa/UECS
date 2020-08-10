#pragma once

#include "CmptPtr.h"

namespace Ubpa::UECS {
	class CmptsView {
	public:
		CmptsView(const CmptPtr* cmpts, size_t num) noexcept
			: cmpts{ cmpts }, num{ num } {}

		CmptPtr GetCmpt(CmptType) const noexcept;

		const CmptPtr* Components() const noexcept { return cmpts; }
		size_t NumberOfComponents() const noexcept { return num; }
	private:
		const CmptPtr* cmpts;
		size_t num;
	};
}
