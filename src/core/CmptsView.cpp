#include <UECS/CmptsView.h>

#include <UECS/EntityLocator.h>

using namespace Ubpa::UECS;
using namespace std;

CmptPtr CmptsView::GetCmpt(CmptType t) const {
	size_t i = 0;
	for (auto iter = locator->CmptTypes().begin(); iter != locator->CmptTypes().end(); ++iter, ++i) {
		if (*iter == t) {
			assert(iter->GetAccessMode() == t.GetAccessMode());
			return CmptPtr(*iter, *(cmpts + i));
		}
	}
	assert(false);
	return CmptPtr(t, nullptr);
}
