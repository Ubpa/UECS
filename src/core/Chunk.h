#pragma once

#include <small_vector.h>

#include <UECS/config.h>

#include <cstdint>
#include <span>

namespace Ubpa::UECS {
	using byte = uint8_t;
	static_assert(sizeof(byte) == 1);
	struct alignas(ChunkAlignment) Chunk {
		static constexpr std::size_t size = ChunkSize;

		struct Layout {
			std::size_t capacity;
			small_vector<std::size_t, 16> offsets;
		};
		static Layout GenLayout(std::span<const std::size_t> alignments, std::span<const std::size_t> sizes) noexcept;

		byte* Data() noexcept { return buffer.data(); }

	private:
		std::array<byte, size> buffer;
	};
	static_assert(sizeof(Chunk) == Chunk::size);
}
