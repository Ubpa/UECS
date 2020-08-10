#include <UECS/CmptsView.h>

using namespace Ubpa::UECS;

CmptPtr CmptsView::GetCmpt(CmptType t) const noexcept {
	for (size_t i = 0; i < num; i++) {
		if (cmpts[i].Type() == t) {
			assert(cmpts[i].Type().GetAccessMode() == t.GetAccessMode());
			return cmpts[i];
		}
	}
	return { CmptType::Invalid(), nullptr };
}
