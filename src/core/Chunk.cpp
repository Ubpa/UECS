#pragma once

#include <UECS/detail/Chunk.h>

using namespace Ubpa;
using namespace std;

Chunk::Layout Chunk::GenLayout(const vector<size_t>& alignments, const vector<size_t>& sizes) noexcept {
	Layout layout;

	// alignment isn't sorted
	struct Item {
		size_t alignment;
		size_t idx;
		bool operator<(const Item& y)const noexcept {
			return alignment < y.alignment;
		}
	};
	vector<Item> items;
	for (size_t i = 0; i < sizes.size(); i++)
		items.push_back(Item{ alignments[i], i });
	sort(items.begin(), items.end());

	constexpr size_t chunkSize = 16 * 1024;
	size_t sumSize = 0;
	for (size_t s : sizes)
		sumSize += s;
	layout.capacity = chunkSize / sumSize;

	layout.offsets.resize(sizes.size());
	size_t curOffset = 0;
	for (size_t i = 0; i < sizes.size(); i++) {
		curOffset = items[i].alignment * ((curOffset + items[i].alignment - 1) / items[i].alignment);
		layout.offsets[items[i].idx] = curOffset;
		curOffset += sizes[items[i].idx] * layout.capacity;
	}

	return layout;
}
