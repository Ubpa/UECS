#pragma once

#include "CmptPtr.hpp"

#include <span>

namespace Ubpa::UECS {
	class CmptsView {
	public:
		CmptsView() noexcept = default;
		CmptsView(std::span<const CmptAccessPtr> cmpts) noexcept : cmpts{ cmpts } {}

		CmptAccessPtr GetCmpt(AccessTypeID) const noexcept;
		std::span<const CmptAccessPtr> Components() const noexcept { return cmpts; }
	private:
		std::span<const CmptAccessPtr> cmpts;
	};
}
