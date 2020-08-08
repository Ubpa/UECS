#include <UECS/SingletonLocator.h>

#include <UECS/detail/Util.h>

#include <UContainer/Algorithm.h>

using namespace Ubpa::UECS;
using namespace std;

SingletonLocator::SingletonLocator(const CmptType* types, size_t num) {
	assert(types != nullptr && num > 0);
	for (size_t i = 0; i < num; i++) {
		switch (types[i].GetAccessMode())
		{
		case Ubpa::UECS::AccessMode::LAST_FRAME_SINGLETON:
			lastFrameSingletonTypes.insert(types[i]);
			break;
		case Ubpa::UECS::AccessMode::WRITE_SINGLETON:
			writeSingletonTypes.insert(types[i]);
			break;
		case Ubpa::UECS::AccessMode::LATEST_SINGLETON:
			latestSingletonTypes.insert(types[i]);
			break;
		default:
			assert(false);
			break;
		}
	}

	singletonTypes = SetUnion(lastFrameSingletonTypes, writeSingletonTypes);
	singletonTypes = SetUnion(singletonTypes, latestSingletonTypes);
}

SingletonLocator::SingletonLocator() {}
