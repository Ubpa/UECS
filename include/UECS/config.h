#pragma once

namespace Ubpa::UECS {
	// compile time config

	// Cache-Lines size is (typically) 64 bytes
	// it has to be an exponent of 2 and >= 16
	constexpr std::size_t ChunkAlignment = 64;

	// 16384 bytes : 16 KB
	// it has to be a multiple of ChunkAlignment and an exponent of 2
	constexpr std::size_t ChunkSize = 16384;
}

