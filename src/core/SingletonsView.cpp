#include <UECS/SingletonsView.hpp>

using namespace Ubpa::UECS;

CmptAccessPtr SingletonsView::AccessSingleton(AccessTypeID t) const noexcept {
	for (const auto& singleton : singletons) {
		if (singleton.AccessType() == t)
			return singleton;
	}
	return CmptAccessPtr::Invalid();
}
