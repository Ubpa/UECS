#include <UECS/SingletonsView.h>

using namespace Ubpa::UECS;

CmptAccessPtr SingletonsView::GetSingleton(CmptAccessType t) const {
	for (size_t i = 0; i < num; i++) {
		if (singletons[i].AccessType() == t)
			return singletons[i];
	}
	return CmptAccessPtr::Invalid();
}
