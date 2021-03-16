#include "Chunk.h"

#include <algorithm>

using namespace Ubpa::UECS;
using namespace std;

Chunk::Layout Chunk::GenLayout(std::span<const std::size_t> alignments, std::span<const std::size_t> sizes) noexcept {
	Layout layout;

	// alignment isn't sorted
	struct Item {
		std::size_t alignment;
		std::size_t idx;
		bool operator<(const Item& y)const noexcept {
			return alignment < y.alignment;
		}
	};
	small_vector<Item, 16> items(sizes.size());
	for (std::size_t i = 0; i < sizes.size(); i++)
		items[i] = Item{ alignments[i], i };
	sort(items.begin(), items.end());

	constexpr std::size_t chunkSize = 16 * 1024;
	std::size_t sumSize = 0;
	for (std::size_t s : sizes)
		sumSize += s;
	layout.capacity = chunkSize / sumSize;

	layout.offsets.resize(sizes.size());
	std::size_t curOffset = 0;
	for (std::size_t i = 0; i < sizes.size(); i++) {
		curOffset = items[i].alignment * ((curOffset + items[i].alignment - 1) / items[i].alignment);
		layout.offsets[items[i].idx] = curOffset;
		curOffset += sizes[items[i].idx] * layout.capacity;
	}

	return layout;
}
