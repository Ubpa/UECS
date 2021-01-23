#include <UECS/CmptsView.h>

using namespace Ubpa::UECS;

CmptAccessPtr CmptsView::GetCmpt(AccessTypeID t) const noexcept {
	for(const auto& cmpt : cmpts) {
		if (cmpt.AccessType() == t)
			return cmpt;
	}
	return CmptAccessPtr::Invalid();
}
