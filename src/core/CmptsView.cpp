#include <UECS/CmptsView.h>

using namespace Ubpa::UECS;

CmptAccessPtr CmptsView::GetCmpt(CmptAccessType t) const noexcept {
	for (size_t i = 0; i < num; i++) {
		if (cmpts[i].AccessType() == t) {
			return cmpts[i];
		}
	}
	return CmptAccessPtr::Invalid();
}
