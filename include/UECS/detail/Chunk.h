#pragma once

#include <cstdint>
#include <array>
#include <tuple>
#include <vector>

namespace Ubpa::UECS {
	using byte = uint8_t;
	static_assert(sizeof(byte) == 1);
	struct alignas(128) Chunk {
		static constexpr size_t size = 16 * 1024;

		struct Layout {
			size_t capacity;
			std::vector<size_t> offsets;
		};
		static Layout GenLayout(const std::vector<size_t>& alignments, const std::vector<size_t>& sizes) noexcept;

		byte* Data() noexcept { return buffer.data(); }

	private:
		std::array<byte, size> buffer;
	};
	static_assert(sizeof(Chunk) == Chunk::size);
}
