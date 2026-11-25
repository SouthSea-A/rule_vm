#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include "../error/error_code.h"

namespace rule_serialize
{
	using byte_t = std::byte;

	constexpr std::uint32_t SERIALIZE_VERSION = 1;
	constexpr std::size_t SERIALIZE_HEADER_SIZE = 40;
	constexpr std::size_t SERIALIZE_MAX_MANAGERS = 1u << 20;
	constexpr std::size_t SERIALIZE_MAX_ELEMENTS = 1u << 24;
	constexpr std::size_t SERIALIZE_MAX_BLOCKS = 1u << 24;
	constexpr std::size_t SERIALIZE_MAX_OPS = 1u << 24;
	constexpr std::size_t SERIALIZE_MAX_FILE_SIZE = 1ull << 34;

	using result = std::expected<void, kernel_error>;
}
