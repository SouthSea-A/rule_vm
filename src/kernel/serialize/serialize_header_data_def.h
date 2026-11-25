#pragma once
#include "serialize_data_def.h"

namespace rule_serialize
{
	struct serialize_header_info
	{
	private:
		std::uint64_t payload_size = 0;
		std::uint32_t crc = 0;
	public:
		template<typename self_this>
		decltype(auto) get_payload_size(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.payload_size);
		}
		template<typename self_this>
		decltype(auto) get_crc(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.crc);
		}
	public:
		void set_payload_size(std::uint64_t payload_size_v) noexcept
		{
			this->payload_size = payload_size_v;
			return;
		}
		void set_crc(std::uint32_t crc_v) noexcept
		{
			this->crc = crc_v;
			return;
		}
	public:
		serialize_header_info() = default;
		serialize_header_info(std::uint64_t payload_size_v, std::uint32_t crc_v) :
			payload_size(payload_size_v), crc(crc_v)
		{

		}
	};
}
