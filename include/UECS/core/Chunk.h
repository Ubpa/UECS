#pragma once

#include <cstdint>
#include <array>
#include <tuple>
#include <vector>
#include <utility>
#include <cassert>

#include <UTemplate/Typelist.h>

namespace Ubpa {
	using byte = uint8_t;
	static_assert(sizeof(byte) == 1);
	struct Chunk {
		static constexpr size_t size = 16 * 1024;

		template<size_t N>
		struct Info {
			constexpr Info(size_t capacity, std::array<size_t, N> sizes, std::array<size_t, N> offsets)
				: capacity(capacity), sizes(sizes), offsets(offsets) {}

			size_t capacity;
			std::array<size_t, N> sizes;
			std::array<size_t, N> offsets;
		};

		template<typename... Cmpts>
		static constexpr Info<sizeof...(Cmpts)> StaticInfo() noexcept {
			static_assert(sizeof...(Cmpts) > 0);
			constexpr size_t capacity = size / (sizeof(Cmpts) + ...);
			constexpr size_t N = sizeof...(Cmpts);
			constexpr std::array<size_t, N> sizes{ sizeof(Cmpts)... };
			std::array<size_t, N> offsets{ 0 };
			for (size_t i = 1; i < N; i++)
				offsets[i] = offsets[i - 1] + capacity * sizes[i-1];
			return { capacity,sizes,offsets };
		}

		// capacity, offsets
		static const std::tuple<size_t, std::vector<size_t>> CO(const std::vector<size_t>& sizes) noexcept {
			size_t N = sizes.size();
			assert(N > 0);

			size_t sumSize = 0;
			for (auto s : sizes) {
				assert(s > 0);
				sumSize += s;
			}

			size_t capacity = size / sumSize;
			std::vector<size_t> offsets;
			offsets.resize(N);
			offsets[0] = 0;
			for (size_t i = 1; i < N; i++)
				offsets[i] = offsets[i - 1] + capacity * sizes[i - 1];
			return { capacity,offsets };
		}

		constexpr byte* Data() noexcept { return buffer.data(); }

	private:
		std::array<byte, size> buffer;
	};
	static_assert(sizeof(Chunk) == Chunk::size);
}
