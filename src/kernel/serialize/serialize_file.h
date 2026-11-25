#pragma once
#include "../kernel_namespace.h"
#include <filesystem>
#include <fstream>
#include "serialize_data_def.h"
#include "serialize_rule.h"

namespace rule_serialize_file
{
	template<typename rule_t>
	rule_serialize::result save_file(
		rule_t& rule_v,
		const typename rule_t::run_ip& ip_v,
		const std::filesystem::path& path_v)
	{
		std::error_code error_v;

		if (path_v.empty())
			return std::unexpected(kernel_error::serialize__io_error);

		if (std::filesystem::exists(path_v, error_v) == true && std::filesystem::is_regular_file(path_v, error_v) == false)
			return std::unexpected(kernel_error::serialize__io_error);

		if (error_v)
			return std::unexpected(kernel_error::serialize__io_error);

		std::ofstream out_v(path_v, std::ios::binary | std::ios::trunc);

		if (out_v.good() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		return rule_serialize::serialize_rule(rule_v, ip_v, out_v);
	}

	template<typename rule_t>
	rule_serialize::result load_file(
		rule_t& rule_v,
		typename rule_t::run_ip& ip_out,
		const std::filesystem::path& path_v)
	{
		std::error_code error_v;

		if (path_v.empty() || std::filesystem::is_regular_file(path_v, error_v) == false)
			return std::unexpected(kernel_error::serialize__io_error);

		if (error_v)
			return std::unexpected(kernel_error::serialize__io_error);

		const std::uintmax_t file_size_v = std::filesystem::file_size(path_v, error_v);

		if (error_v || file_size_v > static_cast<std::uintmax_t>(rule_serialize::SERIALIZE_MAX_FILE_SIZE))
			return std::unexpected(kernel_error::serialize__limit_exceeded);

		std::ifstream in_v(path_v, std::ios::binary);

		if (in_v.good() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		return rule_serialize::deserialize_rule(rule_v, in_v, ip_out);
	}
}
