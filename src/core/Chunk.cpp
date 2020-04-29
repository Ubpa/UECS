#pragma once

#include <UECS/detail/Chunk.h>

using namespace Ubpa;
using namespace std;

const tuple<size_t, vector<size_t>> Chunk::CO(const vector<size_t>& sizes) noexcept {
	size_t N = sizes.size();
	if (N == 0)
		return { Chunk::size,vector<size_t>(1,1) };

	size_t sumSize = 0;
	for (auto s : sizes)
		sumSize += s;

	size_t capacity = size / sumSize;
	vector<size_t> offsets;
	offsets.resize(N);
	offsets[0] = 0;
	for (size_t i = 1; i < N; i++)
		offsets[i] = offsets[i - 1] + capacity * sizes[i - 1];

	return { capacity,offsets };
}
