#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <limits>
#include <memory>
#include <initializer_list>
#include <optional>
#include <vector>
#include <span>
#include <unordered_map>
#include <iosfwd>
#include <filesystem>
#include "rule_kit_data_def.h"
#include "rule_engine.h"
#include "../serialize/serialize_file.h"
#include "../serialize/serialize_rule.h"

namespace rule_tool
{
	template<typename alloc_rule_kit, typename user_outer>
	class rule_kit
	{
		friend rule_kit_cmd_proxy_call<alloc_rule_kit, user_outer>;
	public:
		using owner_t = rule<alloc_rule_kit, user_outer>;
		using outer_t = rule_kit_outer<rule_kit, user_outer>;
		using allocator_type = alloc_rule_kit;
		using user_outer_type = user_outer;
		using data_t = rdata::rule_var_base_t;
		using op_cont = typename owner_t::parse_op;
		using op_del = typename owner_t::op_del;
		using op_cont_span = typename owner_t::op_parse_span;
		using cmd_invoke_param = typename owner_t::cmd_act_param;
		using cmd_invoke_res = typename owner_t::cmd_act_res;
		using cmd_invoke_act_ip = typename owner_t::run_ip;
		using cmd_invoke_proxy_param = rule_kit_cmd_param<alloc_rule_kit, user_outer>;
		using cmd_invoke_info = rule_kit_cmd_info<alloc_rule_kit, user_outer>;
		using cmd_invoke_def = typename owner_t::cmd_def_act;
		using cmd_invoke_proxy = rule_kit_cmd_proxy_call<alloc_rule_kit, user_outer>;
	private:
		using alloc_traits = typename std::allocator_traits<alloc_rule_kit>;
		using alloc_op_cont_buf = alloc_traits::template rebind_alloc<op_cont>;
		using alloc_op_cont_span_buf = alloc_traits::template rebind_alloc<op_cont_span>;
	private:
		owner_t owner;
		std::vector<op_cont, alloc_op_cont_buf> op_cont_buf;
		std::vector<op_cont_span, alloc_op_cont_span_buf> op_cont_span_buf;
		std::span<op_cont> span_op_cont;
		std::span<op_cont_span> span_op_cont_span;
		std::span<cmd_invoke_act_ip> span_stack;
	public:
		template<typename self_this>
		decltype(auto) get_owner(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.owner);
		}
		const std::span<op_cont>& get_span_op_cont() noexcept
		{
			return this->span_op_cont;
		}
		const std::span<op_cont_span>& get_span_op_cont_span() noexcept
		{
			return this->span_op_cont_span;
		}
		const std::span<cmd_invoke_act_ip>& get_span_stack() noexcept
		{
			return this->span_stack;
		}
	public:
		rule_kit() = delete;
		explicit rule_kit(const rule_kit_cmd_cb_int<alloc_rule_kit, user_outer>& cb_int_v, const rule_kit_cmd_cb_rc_err<alloc_rule_kit, user_outer>& cb_err_rc_v, const rule_kit_cmd_cb_err<alloc_rule_kit, user_outer>& cb_err_v, const rule_kit_cmd_cb_hook<alloc_rule_kit, user_outer>& cb_hook_v, user_outer& user_outer_v, const alloc_rule_kit& alloc_val = alloc_rule_kit()) :
			owner(*this, user_outer_v, cb_int_v, cb_err_rc_v, cb_err_v, cb_hook_v, alloc_val),
			op_cont_buf(alloc_op_cont_buf(alloc_val)),
			op_cont_span_buf(alloc_op_cont_span_buf(alloc_val)),
			span_op_cont({}),
			span_op_cont_span({}),
			span_stack({})
		{

		}
		rule_kit(const rule_kit&) = delete;
		rule_kit& operator=(const rule_kit&) = delete;
		rule_kit(rule_kit&&) = delete;
		rule_kit& operator=(rule_kit&&) = delete;
	public:
		std::reference_wrapper<const std::span<cmd_invoke_act_ip>> get_stack(std::size_t size_v)
		{
			this->span_stack = std::span<cmd_invoke_act_ip>(this->owner.get_stack().begin(), size_v);
			return this->span_stack;
		}
		std::optional<rule_kit_var_read_info> get_var_cont_info(cmd_invoke_proxy_param& proxy_param, var_cont_data::var_mgr_index mgr_index_v, var_cont_data::var_index var_index_v, var_cont_data::var_pos var_pos_begin_v, var_cont_data::var_read_width read_width_v)
		{
			rc_data::rc_res_inst_ref<rc_data::rc_sgl<typename owner_t::var_mgr>> var_mgr_res = this->owner.get_var_set().get(mgr_index_v);

			if (var_mgr_res.has_value() == false)
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_var_read_mgr_no_found,
					rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_mgr_res.error()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_var>,
						mgr_index_v,
						var_index_v,
						std::nullopt,
						var_pos_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			typename owner_t::var_mgr& var_mgr_ref = var_mgr_res.value().get().get_content();
			var_data::var_mgr_res_var_sgl<rdata::rule_var_base_t, typename owner_t::alloc_var> var_sgl_res = var_mgr_ref.get(var_index_v);

			if (var_sgl_res.has_value() == false)
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_var_read_mgr_index_no_found,
					rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_sgl_res.error()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_var>,
						mgr_index_v,
						var_index_v,
						std::nullopt,
						var_pos_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			var_data::var_sgl<rdata::rule_var_base_t, typename owner_t::alloc_var>& var_sgl_ref = var_sgl_res.value();
			vcont::var_content<rdata::rule_var_base_t, typename owner_t::alloc_var>& var_cont_ref = var_sgl_ref.get_content();
			var_cont_data::var_cont_res_span<rdata::rule_var_base_t> var_span_res = var_cont_ref.read(var_pos_begin_v, read_width_v);

			if (var_span_res.has_value() == false)
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_var_read_mgr_begin_width_error,
					rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_span_res.error()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_var>,
						mgr_index_v,
						var_index_v,
						std::nullopt,
						var_pos_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			return rule_kit_var_read_info(mgr_index_v, var_index_v, var_sgl_ref.get_type(), var_pos_begin_v, read_width_v, var_sgl_ref.get_ref(), var_span_res.value());
		}
		std::optional<rule_kit_cmd_read_info> get_cmd_cont_info(cmd_invoke_proxy_param& proxy_param, cmd_data::cmd_mgr_index mgr_index_v, cmd_data::cmd_index cmd_index_v, cmd_data::cmd_op_index cmd_op_begin_v, cmd_data::cmd_op_width read_width_v)
		{
			rc_data::rc_res_inst_ref<rc_data::rc_sgl<typename owner_t::cmd_mgr>> cmd_mgr_res = this->owner.get_cmd_set().get(mgr_index_v);

			if (cmd_mgr_res.has_value() == false)
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_cmd_read_mgr_no_found,
					rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, cmd_mgr_res.error()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_cmd>,
						mgr_index_v,
						cmd_index_v,
						std::nullopt,
						cmd_op_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			typename owner_t::cmd_mgr& cmd_mgr_ref = cmd_mgr_res.value().get().get_content();
			cmd_data::cmd_mgr_res_cmd<rule_kit_cmd_sgl> cmd_sgl_res = cmd_mgr_ref.get(cmd_index_v);

			if (cmd_sgl_res.has_value() == false)
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_cmd_read_mgr_index_no_found,
					rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, cmd_sgl_res.error()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_cmd>,
						mgr_index_v,
						cmd_index_v,
						std::nullopt,
						cmd_op_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			rule_kit_cmd_sgl& cmd_sgl_ref = cmd_sgl_res.value();
			auto cmd_ref_res = cmd_mgr_ref.get_ref(cmd_index_v);

			if (cmd_ref_res.has_value() == false)
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_cmd_read_mgr_index_no_found,
					rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, cmd_ref_res.error()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_cmd>,
						mgr_index_v,
						cmd_index_v,
						std::nullopt,
						cmd_op_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			bool is_beyond_size = static_cast<std::size_t>(cmd_op_begin_v) > cmd_sgl_ref.get_content_span().size();
			bool is_overflow = static_cast<std::size_t>(read_width_v) > std::numeric_limits<std::size_t>::max() - static_cast<std::size_t>(cmd_op_begin_v);

			if (is_beyond_size || is_overflow || static_cast<std::size_t>(cmd_op_begin_v) + static_cast<std::size_t>(read_width_v) > cmd_sgl_ref.get_content_span().size())
			{
				this->owner.set_int_info_run
				(
					proxy_param.get_int_info_save(),
					rdata::rule_int_t::kit_cmd_read_mgr_begin_width_error,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_cmd>,
						mgr_index_v,
						cmd_index_v,
						std::nullopt,
						cmd_op_begin_v,
						read_width_v
					)
				);

				return std::nullopt;
			}

			return rule_kit_cmd_read_info
			(
				mgr_index_v,
				cmd_index_v,
				cmd_sgl_ref.get_type().get(),
				cmd_op_begin_v,
				read_width_v,
				cmd_ref_res.value(),
				cmd_sgl_ref.get_content_span().subspan(static_cast<std::size_t>(cmd_op_begin_v), read_width_v)
			);
		}
	public:
		bool check_cmd_ex(cmd_invoke_proxy_param& proxy_param, const rule_kit_cmd_sgl& cmd_v, cmd_data::cmd_type cmd_t_v, std::size_t cmd_op_quan_v)
		{
			return this->owner.check_cmd_info_run(proxy_param.get_int_info_save(), cmd_v, cmd_t_v, cmd_op_quan_v);
		}
		bool check_cmd(cmd_invoke_proxy_param& proxy_param, cmd_data::cmd_type cmd_t_v, std::size_t cmd_op_quan_v)
		{
			return check_cmd_ex(proxy_param, proxy_param.get_cmd_info().get_cmd_sgl(), cmd_t_v, cmd_op_quan_v);
		}
		bool check_cmd_op_ex(cmd_invoke_proxy_param& proxy_param, const rule_kit_cmd_sgl& cmd_v, cmd_data::cmd_op_index cmd_op_index_v, cmd_data::cmd_op_type cmd_op_t_v, bool is_exec)
		{
			return this->owner.check_cmd_op_info_run(proxy_param.get_int_info_save(), cmd_v, cmd_op_index_v, cmd_op_t_v, is_exec);
		}
		bool check_cmd_op(cmd_invoke_proxy_param& proxy_param, cmd_data::cmd_op_index cmd_op_index_v, cmd_data::cmd_op_type cmd_op_t_v, bool is_exec)
		{
			return check_cmd_op_ex(proxy_param, proxy_param.get_cmd_info().get_cmd_sgl(), cmd_op_index_v, cmd_op_t_v, is_exec);
		}
		template<typename tgt_t>
		bool trans_raw_to_tgt(cmd_invoke_proxy_param& proxy_param, const std::span<data_t>& span_raw_bytes_v, tgt_t& res_save)
		{
			return this->owner.transform_raw_bytes_to_run(proxy_param.get_int_info_save(), span_raw_bytes_v, res_save);
		}
		template<typename tgt_t>
		bool trans_tgt_to_raw(cmd_invoke_proxy_param& proxy_param, const tgt_t& tgt_v, const std::span<rdata::rule_var_base_t>& span_raw_bytes_save)
		{
			return this->owner.transform_tgt_to_raw_bytes_run(proxy_param.get_int_info_save(), tgt_v, span_raw_bytes_save);
		}
		std::optional<std::reference_wrapper<const std::span<op_cont>>> get_op_parse_ex(cmd_invoke_proxy_param& proxy_param, const rule_kit_cmd_sgl& cmd_parse_v, const std::initializer_list<cmd_data::cmd_op_index>& op_parse_v, bool is_alloc_rc, bool is_incr_ref)
		{
			if (op_parse_v.size() > this->op_cont_buf.size())
				this->op_cont_buf.resize(op_parse_v.size());

			const bool is_current_cmd =
				std::addressof(cmd_parse_v) == std::addressof(proxy_param.get_cmd_info().get_cmd_sgl());

			if (is_alloc_rc == true)
			{
				typename std::initializer_list<cmd_data::cmd_op_index>::const_iterator it_alloc = op_parse_v.begin();
				op_cont op_alloc;

				while (it_alloc != op_parse_v.end())
				{
					if (this->owner.get_parse_op_run(proxy_param.get_int_info_save(), proxy_param.get_param_orign(), cmd_parse_v, *it_alloc, op_alloc, true, is_incr_ref, true) == false)
					{
						(void)proxy_param.sync_cmd_info();
						return std::nullopt;
					}

					if (is_current_cmd == true && proxy_param.sync_cmd_info() == false)
						return std::nullopt;

					++it_alloc;
				}
			}

			if (is_current_cmd == true && proxy_param.sync_cmd_info() == false)
				return std::nullopt;

			typename std::initializer_list<cmd_data::cmd_op_index>::const_iterator it = op_parse_v.begin();
			std::size_t index_current = 0;

			while (it != op_parse_v.end())
			{
				bool parse_res = this->owner.get_parse_op_run
				(
					proxy_param.get_int_info_save(),
					proxy_param.get_param_orign(),
					cmd_parse_v,
					*it,
					this->op_cont_buf[index_current],
					false,
					is_incr_ref
				);

				if (parse_res == false)
				{
					(void)proxy_param.sync_cmd_info();
					return std::nullopt;
				}

				index_current++;
				++it;
			}

			this->span_op_cont = std::span(this->op_cont_buf.begin(), op_parse_v.size());

			(void)proxy_param.sync_cmd_info();
			return std::ref(this->span_op_cont);
		}
		std::optional<std::reference_wrapper<const std::span<op_cont>>> get_op_parse(cmd_invoke_proxy_param& proxy_param, const std::initializer_list<cmd_data::cmd_op_index>& op_parse_v, bool is_alloc_rc, bool is_incr_ref)
		{
			return get_op_parse_ex(proxy_param, proxy_param.get_cmd_info().get_cmd_sgl(), op_parse_v, is_alloc_rc, is_incr_ref);
		}
		std::optional<std::reference_wrapper<const std::span<op_cont_span>>> get_op_cont_span(std::span<op_cont> op_parse_span_v)
		{
			if (op_parse_span_v.size() > this->op_cont_span_buf.size())
				this->op_cont_span_buf.resize(op_parse_span_v.size());

			typename std::span<op_cont>::iterator it = op_parse_span_v.begin();
			std::size_t index_current = 0;

			while (it != op_parse_span_v.end())
			{
				this->owner.get_op_parse_span_run(*it, this->op_cont_span_buf[index_current]);
				index_current++;
				it++;
			}

			this->span_op_cont_span = std::span(this->op_cont_span_buf.begin(), op_parse_span_v.size());

			return std::ref(this->span_op_cont_span);
		}
		template<typename alloc_tgt_t>
		bool alloc(cmd_invoke_proxy_param& proxy_param, const alloc_tgt_t& alloc_tgt_v, bool is_incr_ref_ow, bool is_incr_ref_tgt)
		{
			const bool res = this->owner.alloc_rc_run(proxy_param.get_int_info_save(), proxy_param.get_param_orign(), alloc_tgt_v, is_incr_ref_ow, is_incr_ref_tgt);
			(void)proxy_param.sync_cmd_info();
			return res;
		}
	public:
		template<typename dealloc_tgt_t>
		bool dealloc(cmd_invoke_proxy_param& proxy_param, const dealloc_tgt_t& dealloc_tgt_v)
		{
			const bool res = this->owner.dealloc_rc_run(proxy_param.get_int_info_save(), proxy_param.get_param_orign(), dealloc_tgt_v);
			(void)proxy_param.sync_cmd_info();
			return res;
		}
		bool write_op_cont_span(cmd_invoke_proxy_param& proxy_param, op_cont& write_tgt, op_cont& write_src)
		{
			const bool res = this->owner.write_op_span_run(proxy_param.get_int_info_save(), proxy_param.get_param_orign(), write_tgt, write_src);
			(void)proxy_param.sync_cmd_info();
			return res;
		}
	public:
		void set_cb_int(const rule_kit_cmd_cb_int<alloc_rule_kit, user_outer>& cb_int_v)
		{
			this->owner.get_call_bak_set().set_call_back_interrupt(cb_int_v);
			return;
		}
		void set_cb_err_rc(const rule_kit_cmd_cb_rc_err<alloc_rule_kit, user_outer>& cb_err_rc_v)
		{
			this->owner.get_call_bak_set().set_call_back_error_rc(cb_err_rc_v);
			return;
		}
		void set_cb_err(const rule_kit_cmd_cb_err<alloc_rule_kit, user_outer>& cb_err_v)
		{
			this->owner.get_call_bak_set().set_call_back_error(cb_err_v);
		}
		void set_cb_hook(const rule_kit_cmd_cb_hook<alloc_rule_kit, user_outer>& cb_hook_v)
		{
			this->owner.get_call_bak_set().set_call_back_hook(cb_hook_v);
			return;
		}
	public:
		void add_set_cmd_def(cmd_data::cmd_type cmd_t_v, const cmd_invoke_def& cmd_def_v)
		{
			this->owner.add_set_cmd_def(cmd_t_v, cmd_def_v);
			return;
		}
		bool add_set_cmd_set(cmd_data::cmd_mgr_index rc_cmd_index_v, cmd_data::cmd_index cmd_index_v, cmd_data::cmd_type cmd_type_v, cmd_data::cmd_op_index cmd_op_index_v, const std::initializer_list<cmd_data::cmd_op_single>& cmd_op_v)
		{
			return this->owner.add_set_cmd_set(rc_cmd_index_v, cmd_index_v, cmd_type_v, cmd_op_index_v, cmd_op_v);
		}
		bool add_set_cmd_set(cmd_data::cmd_mgr_index rc_cmd_index_v, cmd_data::cmd_index cmd_index_v, cmd_data::cmd_type cmd_type_v, cmd_data::cmd_op_index cmd_op_index_v, std::span<const cmd_data::cmd_op_single> cmd_op_v)
		{
			return this->owner.add_set_cmd_set(rc_cmd_index_v, cmd_index_v, cmd_type_v, cmd_op_index_v, cmd_op_v);
		}
		bool add_set_var_set(var_cont_data::var_mgr_index rc_var_index_v, var_cont_data::var_index var_index_v, var_cont_data::var_type var_type_v, var_cont_data::var_pos pos_begin_v, const std::span<rdata::rule_var_base_t>& span_write_v)
		{
			return this->owner.add_set_var_set(rc_var_index_v, var_index_v, var_type_v, pos_begin_v, span_write_v);
		}
		void run(cmd_data::cmd_mgr_index rc_cmd_index_v, cmd_data::cmd_index cmd_index_v)
		{
			this->owner.run(rc_cmd_index_v, cmd_index_v);
		}
public:
		bool save(const cmd_invoke_act_ip& ip_v, const std::filesystem::path& path_v)
		{
			return rule_serialize_file::save_file(this->owner, ip_v, path_v).has_value();
		}
		bool load(const std::filesystem::path& path_v, cmd_invoke_act_ip& ip_v)
		{
			return rule_serialize_file::load_file(this->owner, ip_v, path_v).has_value();
		}
		void resume(const cmd_invoke_act_ip& ip_v)
		{
			this->owner.resume(ip_v);
			return;
		}
	public:
		void copy_from(const rule_kit& rule_kit_v)
		{
			this->owner.copy_from(rule_kit_v.get_owner());
			return;
		}
		void copy_to(rule_kit& rule_kit_v)
		{
			this->owner.copy_to(rule_kit_v, rule_kit_v.get_owner());
			return;
		}
		void shrink_to_fit()
		{
			this->owner.shrink_to_fit();
			this->op_cont_buf.clear();
			this->op_cont_buf.shrink_to_fit();
			this->op_cont_span_buf.clear();
			this->op_cont_span_buf.shrink_to_fit();
			this->span_op_cont = {};
			this->span_op_cont_span = {};
			return;
		}
	public:
		void clear()
		{
			this->owner.clear();
			this->op_cont_buf.clear();
			this->op_cont_span_buf.clear();
			this->span_op_cont = {};
			this->span_op_cont_span = {};
			this->span_stack = {};
			return;
		}
	};
}

