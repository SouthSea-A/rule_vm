#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <utility>
#include <type_traits>
#include <bit>
#include <limits>
#include <array>
#include <span>
#include <algorithm>
#include <variant>
#include <expected>
#include "../variable/var.h"

namespace rule_cmd_data_def
{
	using cmd_data_type_base_8 = std::uint8_t;
	using cmd_data_type_base_16 = std::uint16_t;
	using cmd_data_type_base_32 = std::uint32_t;
	using cmd_data_type_base_64 = std::uint64_t;

	using cmd_index = var_cont_data::var_index;
	using cmd_type = cmd_data_type_base_64;
	using cmd_op_index = var_cont_data::var_pos;
	using cmd_op_width = cmd_data_type_base_64;
	using cmd_mgr_index = cmd_data_type_base_64;

	using cmd_var_base_type = std::byte;

	constexpr cmd_index INVALID_CMD_INDEX = std::numeric_limits<cmd_index>::max();
	constexpr cmd_op_index INVALID_CMD_OP_INDEX = std::numeric_limits<cmd_op_index>::max();
	constexpr cmd_mgr_index INVALID_CMD_MGR_INDEX = std::numeric_limits<cmd_mgr_index>::max();

	enum class op_var_type : cmd_data_type_base_16
	{
		var_type,
		var_self,
		var_tgt,
		var_val,
		var_ptr,
		var_del,
		invalid,
	};

	struct var_target
	{
	public:
		op_var_type handle_type;
		var_cont_data::var_mgr_index ownership;
		var_cont_data::var_index index;
		var_cont_data::var_type type;
		var_cont_data::var_pos begin;
		var_cont_data::var_read_width width;
	public:
		template<typename self_this>
		decltype(auto) get_handle_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.handle_type);
		}
		template<typename self_this>
		decltype(auto) get_ownership(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ownership);
		}
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
		decltype(auto) get_begin(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.begin);
		}
		template<typename self_this>
		decltype(auto) get_width(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.width);
		}
	public:
		void set_handle_type(op_var_type handle_type_val)
		{
			this->handle_type = handle_type_val;
		}
		void set_ownership(var_cont_data::var_mgr_index ownership_val)
		{
			this->ownership = ownership_val;
		}
		void set_index(var_cont_data::var_index var_index_val)
		{
			this->index = var_index_val;
		}
		void set_type(var_cont_data::var_type var_type_val)
		{
			this->type = var_type_val;
		}
		void set_begin(var_cont_data::var_pos begin_val)
		{
			this->begin = begin_val;
		}
		void set_width(var_cont_data::var_read_width width_val)
		{
			this->width = width_val;
		}
	public:
		var_target() :
			handle_type(op_var_type::invalid), ownership(var_cont_data::INVALID_VAR_MGR_INDEX), index(var_cont_data::INVALID_VAR_INDEX), type(0), begin(var_cont_data::INVALID_VAR_POS), width(0)
		{

		}
		var_target(op_var_type handle_type_val, var_cont_data::var_mgr_index ownership_val, var_cont_data::var_index index_val, var_cont_data::var_type type_v, var_cont_data::var_pos begin_val, var_cont_data::var_read_width width_val) :
			handle_type(handle_type_val), ownership(ownership_val), index(index_val), type(type_v), begin(begin_val), width(width_val)
		{

		}
	};

	using var_cmd_mgr_index = std::common_type_t<var_cont_data::var_mgr_index, cmd_mgr_index>;
	using var_cmd_index = std::common_type_t<var_cont_data::var_index, cmd_index>;
	using var_cmd_type = std::common_type_t<var_cont_data::var_type, cmd_type>;
	using var_cmd_begin_pos = std::common_type_t<var_cont_data::var_pos, cmd_op_index>;

	constexpr var_cmd_mgr_index INVALID_VAR_CMD_MGR_INDEX = std::numeric_limits<var_cmd_mgr_index>::max();
	constexpr var_cmd_index INVALID_VAR_CMD_INDEX = std::numeric_limits<var_cmd_index>::max();
	constexpr var_cmd_begin_pos INVALID_VAR_CMD_BEGIN_POS = std::numeric_limits<var_cmd_begin_pos>::max();

	enum class op_cmd_type : cmd_data_type_base_16
	{
		cmd_type,
		cmd_self,
		cmd_tgt,
		cmd_val,
		cmd_ptr,
		cmd_del,
		invalid,
	};

	struct cmd_target
	{
	public:
		op_cmd_type handle_type;
		var_cmd_mgr_index ownership;
		var_cmd_index index;
		var_cmd_type type;
		var_cmd_begin_pos begin;
		cmd_op_width width;
	public:
		template<typename self_this>
		decltype(auto) get_handle_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.handle_type);
		}
		template<typename self_this>
		decltype(auto) get_ownership(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ownership);
		}
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
		decltype(auto) get_begin(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.begin);
		}
		template<typename self_this>
		decltype(auto) get_width(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.width);
		}
	public:
		void set_handle_type(op_cmd_type handle_type_val)
		{
			this->handle_type = handle_type_val;
		}
		void set_ownership(var_cmd_mgr_index ownership_val)
		{
			this->ownership = ownership_val;
		}
		void set_index(var_cmd_index index_val)
		{
			this->index = index_val;
		}
		void set_type(var_cmd_type type_val)
		{
			this->type = type_val;
		}
		void set_begin(var_cmd_begin_pos begin_val)
		{
			this->begin = begin_val;
		}
		void set_width(cmd_op_width width_val)
		{
			this->width = width_val;
		}
	public:
		cmd_target() :
			handle_type(op_cmd_type::invalid), ownership(INVALID_VAR_CMD_MGR_INDEX), index(INVALID_VAR_CMD_INDEX), type(0), begin(INVALID_VAR_CMD_BEGIN_POS), width(0)
		{

		}
		cmd_target(op_cmd_type handle_type_val, var_cmd_mgr_index ownership_val, var_cmd_index index_val, var_cmd_type type_v, var_cmd_begin_pos begin_val, cmd_op_width width_val) :
			handle_type(handle_type_val), ownership(ownership_val), index(index_val), type(type_v), begin(begin_val), width(width_val)
		{

		}
	};

	enum class imm_type : cmd_data_type_base_8
	{
		self,
		byte,
		word,
		dword,
		qword,
		invalid,
	};

	constexpr std::size_t imm_width_byte_max = std::max({ sizeof(std::uint64_t) });

	struct imm_target
	{
	public:
		imm_type type;
		std::array<cmd_var_base_type, imm_width_byte_max> data;
	public:
		template<typename self_this>
		decltype(auto) get_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type);
		}
		template<typename self_this>
		decltype(auto) get_data(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.data);
		}
	public:
		void set_type(imm_type type_v)
		{
			this->type = type_v;
		}
		void set_data(const std::array<cmd_var_base_type, imm_width_byte_max>& data_v)
		{
			this->data = data_v;
		}
	public:
		imm_target() :
			type(imm_type::invalid), data{}
		{

		}
		imm_target(imm_type type_val, const std::array<cmd_var_base_type, imm_width_byte_max>& data_val) :
			type(type_val), data(data_val)
		{

		}
	};

	using cmd_op_single = std::variant<imm_target, var_target, cmd_target>;

	enum class cmd_op_type : cmd_data_type_base_8
	{
		imm,
		var,
		cmd,
		both,
	};

	using raw_bytes_base_field_type = std::uint64_t;

	constexpr std::size_t RAW_BYTES_SIZE_INT_8 = 1;
	constexpr std::size_t RAW_BYTES_SIZE_INT_16 = 2;
	constexpr std::size_t RAW_BYTES_SIZE_INT_32 = 4;
	constexpr std::size_t RAW_BYTES_SIZE_INT_64 = 8;

	constexpr std::size_t RAW_BYTES_SIZE_UINT_8 = 1;
	constexpr std::size_t RAW_BYTES_SIZE_UINT_16 = 2;
	constexpr std::size_t RAW_BYTES_SIZE_UINT_32 = 4;
	constexpr std::size_t RAW_BYTES_SIZE_UINT_64 = 8;

	constexpr std::size_t RAW_BYTES_SIZE_FLOAT_32 = 4;
	constexpr std::size_t RAW_BYTES_SIZE_FLOAT_64 = 8;

	constexpr std::size_t RAW_BYTES_SIZE_FIELD_BASE = sizeof(raw_bytes_base_field_type);
	constexpr std::size_t RAW_BYTES_SIZE_VAR_CMD_TYPE = RAW_BYTES_SIZE_FIELD_BASE * 1;
	constexpr std::size_t RAW_BYTES_SIZE_VAR_TARGET = RAW_BYTES_SIZE_FIELD_BASE * 6;
	constexpr std::size_t RAW_BYTES_SIZE_CMD_TARGET = RAW_BYTES_SIZE_FIELD_BASE * 6;
	constexpr std::size_t RAW_BYTES_SIZE_IMM_TARGET = RAW_BYTES_SIZE_FIELD_BASE * 1 + imm_width_byte_max;
	constexpr std::size_t RAW_BYTES_SIZE_CMD_OP = RAW_BYTES_SIZE_FIELD_BASE * 1 + std::max({ RAW_BYTES_SIZE_VAR_TARGET , RAW_BYTES_SIZE_CMD_TARGET , RAW_BYTES_SIZE_IMM_TARGET });

	constexpr std::endian RAW_BYTES_ORDER = std::endian::big;

	template<bool is_const>
	struct cmd_single
	{
	public:
		cmd_index index;
		std::conditional_t<is_const, std::reference_wrapper<const cmd_type>, std::reference_wrapper<cmd_type>> type;
		std::conditional_t<is_const, std::span<const cmd_op_single>, std::span<cmd_op_single>> content;
	public:
		template<typename self_this>
		decltype(auto) get_index(this self_this&& self)
		{
			return std::forward_like<self_this>(self.index);
		}
		template<typename self_this>
		decltype(auto) get_type(this self_this&& self)
		{
			return std::forward_like<self_this>(self.type);
		}
		template<typename self_this>
		decltype(auto) get_content_span(this self_this&& self)
		{
			return std::forward_like<self_this>(self.content);
		}
	};

	using cmd_mgr_res = std::expected<void, kernel_error>;
	template<typename T>
	using cmd_mgr_res_size = std::expected<T, kernel_error>;
	template<typename T>
	using cmd_mgr_res_cmd = std::expected<T, kernel_error>;

	template<typename T>
	using cmd_mgr_res_cmd_single_it = std::expected<T, kernel_error>;
	template<typename T>
	using cmd_mgr_res_cmd_single_it_const = std::expected<T, kernel_error>;
}