#pragma once

#include "CmptPtr.hpp"

#include <span>

namespace Ubpa::UECS {
	class CmptsView {
	public:
		CmptsView() noexcept;
		CmptsView(std::span<const CmptAccessPtr> cmpts) noexcept;

		CmptAccessPtr GetCmpt(AccessTypeID) const noexcept;
		std::span<const CmptAccessPtr> AccessComponents() const noexcept;
	private:
		std::span<const CmptAccessPtr> cmpts;
	};
}
