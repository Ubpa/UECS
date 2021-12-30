#include <UECS/CmptsView.hpp>

using namespace Ubpa::UECS;

CmptsView::CmptsView() noexcept = default;

CmptsView::CmptsView(std::span<const CmptAccessPtr> cmpts) noexcept : cmpts{ cmpts } {}

CmptAccessPtr CmptsView::GetCmpt(AccessTypeID t) const noexcept {
	for(const auto& cmpt : cmpts) {
		if (cmpt.AccessType() == t)
			return cmpt;
	}
	return CmptAccessPtr::Invalid();
}

std::span<const CmptAccessPtr> CmptsView::AccessComponents() const noexcept { return cmpts; }
