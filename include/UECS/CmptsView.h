#pragma once

#include "CmptPtr.h"

namespace Ubpa::UECS {
	class CmptsView {
	public:
		CmptsView(const CmptAccessPtr* cmpts, size_t num) noexcept
			: cmpts{ cmpts }, num{ num } {}

		CmptAccessPtr GetCmpt(CmptAccessType) const noexcept;

		const CmptAccessPtr* Components() const noexcept { return cmpts; }
		size_t NumberOfComponents() const noexcept { return num; }
	private:
		const CmptAccessPtr* cmpts;
		size_t num;
	};
}
