#pragma once
#include "serialize_data_def.h"

namespace rule_serialize_var_data_def
{
	struct var_block_record
	{
	private:
		std::uint64_t pos = 0;
		std::uint64_t length = 0;
	public:
		template<typename self_this>
		decltype(auto) get_pos(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.pos);
		}
		template<typename self_this>
		decltype(auto) get_length(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.length);
		}
	public:
		var_block_record() = default;
		var_block_record(std::uint64_t pos_v, std::uint64_t length_v) :
			pos(pos_v), length(length_v)
		{

		}
	};

	struct var_record
	{
	private:
		std::uint64_t index = 0;
		std::uint64_t type = 0;
		std::uint64_t ref = 0;
		std::uint64_t block_count = 0;
	public:
		template<typename self_this>
		decltype(auto) get_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.index);
		}
		template<typename self_this>
		decltype(auto) get_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type);
		}
		template<typename self_this>
		decltype(auto) get_ref(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ref);
		}
		template<typename self_this>
		decltype(auto) get_block_count(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.block_count);
		}
	public:
		void set_index(std::uint64_t index_v) noexcept
		{
			this->index = index_v;
			return;
		}
		void set_type(std::uint64_t type_v) noexcept
		{
			this->type = type_v;
			return;
		}
		void set_ref(std::uint64_t ref_v) noexcept
		{
			this->ref = ref_v;
			return;
		}
		void set_block_count(std::uint64_t block_count_v) noexcept
		{
			this->block_count = block_count_v;
			return;
		}
	public:
		var_record() = default;
	};
}
