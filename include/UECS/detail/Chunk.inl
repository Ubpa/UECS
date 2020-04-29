#pragma once

namespace Ubpa::detail::Chunk_ {
	template<typename... Cmpts>
	struct StaticInfo;
}

namespace Ubpa {
	template<typename... Cmpts>
	static constexpr auto Chunk::StaticInfo() noexcept {
		if constexpr (sizeof...(Cmpts) > 1) {
			static_assert(std::min({ std::alignment_of_v<Cmpts>... }) == std::max({ std::alignment_of_v<Cmpts>... }),
				"different alignment");
		}
		return detail::Chunk_::StaticInfo<Cmpts...>::run();
	}
}

namespace Ubpa::detail::Chunk_ {
	template<typename... Cmpts>
	struct StaticInfo {
		static constexpr Chunk::Info<sizeof...(Cmpts)> run() noexcept {
			static_assert(sizeof...(Cmpts) > 0);
			constexpr size_t capacity = Chunk::size / (sizeof(Cmpts) + ...);
			constexpr size_t N = sizeof...(Cmpts);
			constexpr std::array<size_t, N> sizes{ sizeof(Cmpts)... };
			std::array<size_t, N> offsets{ 0 };
			for (size_t i = 1; i < N; i++)
				offsets[i] = offsets[i - 1] + capacity * sizes[i - 1];
			return { capacity,sizes,offsets };
		}
	};

	template<>
	struct StaticInfo<> {
		static constexpr Chunk::Info<1> run() noexcept {
			constexpr size_t capacity = Chunk::size;
			constexpr size_t N = 1;
			constexpr std::array<size_t, N> sizes{ 1 };
			constexpr std::array<size_t, N> offsets{ 0 };
			return { capacity,sizes,offsets };
		}
	};
}