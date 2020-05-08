#pragma once

#include <cstdint>
#include <array>
#include <tuple>
#include <vector>

namespace Ubpa {
	using byte = uint8_t;
	static_assert(sizeof(byte) == 1);
	struct alignas(128) Chunk {
		static constexpr size_t size = 16 * 1024;

		template<size_t N>
		struct Info {
			constexpr Info(size_t capacity, std::array<size_t, N> sizes, std::array<size_t, N> offsets)
				: capacity(capacity), sizes(sizes), offsets(offsets) {}

			size_t capacity;
			std::array<size_t, N> sizes;
			std::array<size_t, N> offsets;
		};

		// return Info<max(1,sizeof...(Cmpts))>
		template<typename... Cmpts>
		static constexpr auto StaticInfo() noexcept;

		// capacity, offsets
		static const std::tuple<size_t, std::vector<size_t>> CO(const std::vector<size_t>& sizes) noexcept;

		byte* Data() noexcept { return buffer.data(); }

	private:
		std::array<byte, size> buffer;
	};
	static_assert(sizeof(Chunk) == Chunk::size);
}

#include "Chunk.inl"
