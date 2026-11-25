#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <atomic>
#include <memory>
#include <utility>
#include <type_traits>
#include <concepts>
#include <optional>
#include <functional>
#include <expected>
#include "act_data_def.h"
#include "act_concept.h"

namespace rule_act
{
    template<typename rc_cmd, typename stk_t, typename cmd_def_set_t, typename cb_t, typename outer>
    requires act_conc::concept_rc_cmd<rc_cmd>&& act_conc::concept_act_stk<stk_t, rc_cmd>&& act_conc::concept_cmd_def_set<cmd_def_set_t, rc_cmd, typename cb_t::call_back_interrupt_info_type, outer>&& act_conc::concept_call_back<cb_t, rc_cmd, outer>
    class actuator
    {
        static_assert(std::is_same_v<typename rc_cmd::unexpected_error_type, typename rc_cmd::resource_type::unexpected_error_type>, "资源层与指令层的错误类型必须一致，才能共用同一个错误回调");
    private:
        using outer_ptr = outer*;
        using err_cb_rc_param_t = act_data::act_err_param<rc_cmd, typename rc_cmd::unexpected_error_type, outer>;
        using err_cb_param_t = act_data::act_err_param_base<rc_cmd, outer>;
        using hook_cb_param_t = act_data::act_hook_param<rc_cmd, outer>;
        using act_run_param_t = act_data::act_run_param<rc_cmd, outer>;
        using cmd_rc_sgl_t = typename rc_cmd::value_type;
        using cmd_sgl_t = typename rc_cmd::resource_type;
        using cmd_sgl_it_t = typename cmd_sgl_t::iterator;
        using cmd_def_set_it_t = typename cmd_def_set_t::iterator;
        using cmd_rc_err_cb_code_t = typename rc_cmd::unexpected_error_type;
        using cmd_sgl_err_cb_code_t = typename cmd_sgl_t::unexpected_error_type;
    private:
        std::reference_wrapper<rc_cmd> cmd_actuate_set;
        std::reference_wrapper<stk_t> stack;
        std::reference_wrapper<cmd_def_set_t> cmd_define_set;
        std::reference_wrapper<cb_t> call_back_set;
        outer_ptr data_outer;
    private:
        void run_error_handle(act_data::act_err_t error_type, cmd_rc_err_cb_code_t error_outer, std::reference_wrapper<act_data::act_cmd_ip<rc_cmd>>& ip_current, std::reference_wrapper<cmd_sgl_it_t>& it_update, bool& is_end)
        {
            std::invoke
            (
                this->call_back_set.get().get_call_back_error_rc(),
                err_cb_rc_param_t(error_type, error_outer, this->data_outer, ip_current),
                is_end
            );

            if (is_end == true)
                return;

            std::expected<std::reference_wrapper<cmd_rc_sgl_t>, cmd_rc_err_cb_code_t> cmd_set_current = this->cmd_actuate_set.get().get(ip_current.get().get_cmd_set_index());

            if (cmd_set_current.has_value() == false)
            {
                std::invoke
                (
                    this->call_back_set.get().get_call_back_error_rc(),
                    err_cb_rc_param_t(act_data::act_err_t::invalid_ip_update, cmd_set_current.error(), this->data_outer, ip_current),
                    is_end
                );
                is_end = true;
                return;
            }

            cmd_sgl_t& command_set_single_current = cmd_set_current.value().get().get_content();
            std::expected<cmd_sgl_it_t, cmd_sgl_err_cb_code_t> it_update_res = command_set_single_current.get_it(ip_current.get().get_index());

            if (it_update_res.has_value() == false)
            {
                std::invoke
                (
                    this->call_back_set.get().get_call_back_error_rc(),
                    err_cb_rc_param_t(act_data::act_err_t::invalid_ip_update, it_update_res.error(), this->data_outer, ip_current),
                    is_end
                );
                is_end = true;
                return;
            }

            it_update.get() = it_update_res.value();
            return;
        }
        bool run_hook_handle(std::reference_wrapper<cmd_sgl_t>& cmd_set_single_current, std::reference_wrapper<act_data::act_cmd_ip<rc_cmd>>& ip_current, std::reference_wrapper<cmd_sgl_it_t>& it_update, bool& is_end)
        {
            auto& hook_cb_ref = this->call_back_set.get().get_call_back_hook();
            act_data::act_hook_t hook_res = hook_cb_ref.get_cb_outer() == nullptr ? act_data::act_hook_t::pass : hook_cb_ref(hook_cb_param_t(ip_current, it_update, this->data_outer));
            bool res_val = false;

            switch (hook_res)
            {
            case act_data::act_hook_t::exit:
            {
                is_end = true;
                res_val = false;
                break;
            }
            case act_data::act_hook_t::ip_change:
            {
                std::expected<std::reference_wrapper<cmd_rc_sgl_t>, cmd_rc_err_cb_code_t> cmd_set_current = this->cmd_actuate_set.get().get(ip_current.get().get_cmd_set_index());

                if (cmd_set_current.has_value() == false)
                {
                    run_error_handle(act_data::act_err_t::invalid_ip_update, cmd_set_current.error(), ip_current, it_update, is_end);
                    is_end = true;
                    res_val = false;
                }
                else
                {
                    cmd_set_single_current = cmd_set_current.value().get().get_content();
                    is_end = false;
                    res_val = false;
                }

                break;
            }
            case act_data::act_hook_t::pass:
            {
                is_end = false;
                res_val = true;
                break;
            }
            default:
            {
                break;
            }
            }

            return res_val;
        }
        void run_interrupt_handle(std::reference_wrapper<cmd_sgl_t>& cmd_set_single_current, typename cmd_def_set_t::iterator& cmd_define_it, std::reference_wrapper<act_data::act_cmd_ip<rc_cmd>>& ip_current_run, std::reference_wrapper<cmd_sgl_it_t>& it_current_run, bool& is_end)
        {
            if (cmd_define_it->second == nullptr)
            {
                is_end = true;
                return;
            }

            act_run_param_t run_param(ip_current_run, it_current_run, this->data_outer);
            act_data::act_run_int<rc_cmd, typename cb_t::call_back_interrupt_info_type> run_res = std::invoke(cmd_define_it->second, run_param);

            std::invoke(this->call_back_set.get().get_call_back_interrupt(), run_param, run_res, is_end);

            if (is_end == true)
                return;

            auto command_set_refresh_handle = [&]()->bool
            {
                std::expected<std::reference_wrapper<cmd_rc_sgl_t>, cmd_rc_err_cb_code_t> cmd_set_current = this->cmd_actuate_set.get().get(ip_current_run.get().get_cmd_set_index());

                if (cmd_set_current.has_value() == false)
                    return false;

                cmd_set_single_current = cmd_set_current.value().get().get_content();
                return true;
            };

            auto command_ip_target_exist_handle = [&](bool& is_need_retry)->void
            {
                ip_current_run.get() = run_res.get_ip_target().value();

                if (command_set_refresh_handle() == false)
                {
                    is_end = true;
                    is_need_retry = false;
                    return;
                }

                std::expected<cmd_sgl_it_t, cmd_sgl_err_cb_code_t> it_current_run_res = cmd_set_single_current.get().get_it(ip_current_run.get().get_index());

                if (it_current_run_res.has_value() == false)
                {
                    is_end = true;
                    is_need_retry = false;
                    return;
                }

                it_current_run.get() = it_current_run_res.value();
                is_need_retry = false;
                return;
            };

            auto command_invoke_end_handle = [&]()->void
            {
                while (this->stack.get().empty() == false)
                {
                    act_data::act_cmd_ip<rc_cmd> ip_invoke_last = this->stack.get().back();

                    this->stack.get().pop_back();
                    ip_current_run.get() = ip_invoke_last;

                    if (command_set_refresh_handle() == false)
                        continue;

                    std::expected<cmd_sgl_it_t, cmd_sgl_err_cb_code_t> it_current_run_res = cmd_set_single_current.get().get_it(ip_invoke_last.get_index());

                    if (it_current_run_res.has_value() == false)
                        continue;

                    it_current_run.get() = it_current_run_res.value();
                    ++it_current_run.get();

                    if (it_current_run.get() == cmd_set_single_current.get().end())
                        continue;

                    ip_current_run.get().set_index(it_current_run.get()->get_index());
                    is_end = false;
                    return;
                }

                is_end = true;
                return;
            };

            auto command_next_handle = [&]()->void
            {
                if (command_set_refresh_handle() == false)
                {
                    is_end = true;
                    return;
                }

                if (cmd_set_single_current.get().is_valid(it_current_run.get()) == false)
                {
                    command_invoke_end_handle();
                    return;
                }

                if (it_current_run.get()->get_index() == ip_current_run.get().get_index())
                    ++it_current_run.get();

                if (it_current_run.get() == cmd_set_single_current.get().end())
                {
                    command_invoke_end_handle();
                }
                else
                {
                    ip_current_run.get().set_index(it_current_run.get()->get_index());
                    is_end = false;
                }

                return;
            };

            switch (run_res.get_type())
            {
            case act_data::act_run_int_t::invalid:
            {
                is_end = true;
                break;
            }
            case act_data::act_run_int_t::next:
            {
                bool is_retry = false;

                if (run_res.get_ip_target().has_value() == false)
                {
                    command_next_handle();
                }
                else
                {
                    do
                    {
                        command_ip_target_exist_handle(is_retry);

                        if (is_end == true)
                            break;

                    } while (is_retry == true);
                }

                break;
            }
            case act_data::act_run_int_t::invoke:
            {
                if (run_res.get_ip_target().has_value() == false)
                {
                    command_next_handle();
                }
                else
                {
                    act_data::act_cmd_ip<rc_cmd> ip_invoke_current = ip_current_run.get();
                    bool is_retry = false;

                    do
                    {
                        command_ip_target_exist_handle(is_retry);

                        if (is_end == true)
                            break;

                        if (is_retry == false) this->stack.get().emplace_back(ip_invoke_current);

                    } while (is_retry == true);
                }

                break;
            }
            case act_data::act_run_int_t::error:
            {
                is_end = true;
                break;
            }
            case act_data::act_run_int_t::invoke_end:
            {
                command_invoke_end_handle();
                break;
            }
            case act_data::act_run_int_t::exit:
            {
                is_end = true;
                break;
            }
            default:
            {
                is_end = true;
                break;
            }
            }

            return;
        }
    public:
        actuator() = delete;
        explicit actuator(rc_cmd& cmd_actuate_set_v, stk_t& stack_v, cmd_def_set_t& cmd_define_set_v, cb_t& call_back_set_v, outer_ptr data_outer_v) :
            cmd_actuate_set(cmd_actuate_set_v),
            stack(stack_v),
            cmd_define_set(cmd_define_set_v),
            call_back_set(call_back_set_v),
            data_outer(data_outer_v)
        {

        }
        actuator(const actuator& other_v) :
            cmd_actuate_set(other_v.cmd_actuate_set),
            stack(other_v.stack),
            cmd_define_set(other_v.cmd_define_set),
            call_back_set(other_v.call_back_set),
            data_outer(other_v.data_outer)
        {

        }
        actuator& operator=(const actuator& other_v)
        {
            if (this == &other_v)
                return *this;

            this->cmd_actuate_set = other_v.cmd_actuate_set;
            this->stack = other_v.stack;
            this->cmd_define_set = other_v.cmd_define_set;
            this->call_back_set = other_v.call_back_set;
            this->data_outer = other_v.data_outer;

            return *this;
        }
        actuator(actuator&& other_v) :
            cmd_actuate_set(std::move(other_v.cmd_actuate_set)),
            stack(std::move(other_v.stack)),
            cmd_define_set(std::move(other_v.cmd_define_set)),
            call_back_set(std::move(other_v.call_back_set)),
            data_outer(std::move(other_v.data_outer))
        {

        }
        actuator& operator=(actuator&& other_v)
        {
            if (this == &other_v)
                return *this;

            this->cmd_actuate_set = std::move(other_v.cmd_actuate_set);
            this->stack = std::move(other_v.stack);
            this->cmd_define_set = std::move(other_v.cmd_define_set);
            this->call_back_set = std::move(other_v.call_back_set);
            this->data_outer = std::move(other_v.data_outer);

            return *this;
        }
    public:
        void set(rc_cmd& rc_cmd_v, stk_t& stack_v, cmd_def_set_t& cmd_define_set_v, cb_t& call_back_set_v, outer_ptr data_outer_v)
        {
            this->cmd_actuate_set = rc_cmd_v;
            this->stack = stack_v;
            this->cmd_define_set = cmd_define_set_v;
            this->call_back_set = call_back_set_v;
            this->data_outer = data_outer_v;
        }
        void set_rc_cmd(rc_cmd& rc_cmd_v)
        {
            this->cmd_actuate_set = rc_cmd_v;
        }
        void set_cmd_define_set(cmd_def_set_t& cmd_define_set_v)
        {
            this->cmd_define_set = cmd_define_set_v;
        }
        void set_call_back_set(cb_t& call_back_set_v)
        {
            this->call_back_set = call_back_set_v;
        }
        void set_stack(stk_t& stack_v)
        {
            this->stack = stack_v;
        }
        void set_data_outer(outer_ptr data_outer_v)
        {
            this->data_outer = data_outer_v;
        }
        template <typename self_type>
        decltype(auto) get_rc_command(this self_type&& self) noexcept
        {
            return std::forward_like<self_type>(self.cmd_actuate_set.get());
        }
        template <typename self_type>
        decltype(auto) get_command_define_set(this self_type&& self) noexcept
        {
            return std::forward_like<self_type>(self.cmd_define_set.get());
        }
        template <typename self_type>
        decltype(auto) get_call_back_set(this self_type&& self) noexcept
        {
            return std::forward_like<self_type>(self.call_back_set.get());
        }
        template <typename self_type>
        decltype(auto) get_run_stack_current(this self_type&& self) noexcept
        {
            return std::forward_like<self_type>(self.stack);
        }
        template <typename self_type>
        decltype(auto) get_data_outer(this self_type&& self) noexcept
        {
            return std::forward_like<self_type>(self.data_outer);
        }
        void run(const act_data::act_cmd_ip<rc_cmd>& ip_begin, bool is_clear_stack = true)
        {
            if (is_clear_stack == true)
                this->stack.get().clear();

            bool is_end = false;
            act_data::act_cmd_ip<rc_cmd> ip_current = ip_begin;
            std::reference_wrapper<act_data::act_cmd_ip<rc_cmd>> ip_current_ref = ip_current;

            std::expected<std::reference_wrapper<cmd_rc_sgl_t>, cmd_rc_err_cb_code_t> cmd_set_current = this->cmd_actuate_set.get().get(ip_current_ref.get().get_cmd_set_index());

            if (cmd_set_current.has_value() == false)
            {
                std::invoke
                (
                    this->call_back_set.get().get_call_back_error_rc(),
                    err_cb_rc_param_t(act_data::act_err_t::invalid_ip_begin, cmd_set_current.error(), this->data_outer, ip_current_ref),
                    is_end
                );
                return;
            }

            std::reference_wrapper<cmd_sgl_t> cmd_set_single_current = cmd_set_current.value().get().get_content();
            std::expected<cmd_sgl_it_t, cmd_sgl_err_cb_code_t> ip_it_current_res = cmd_set_single_current.get().get_it(ip_current_ref.get().get_index());

            if (ip_it_current_res.has_value() == false)
            {
                std::invoke
                (
                    this->call_back_set.get().get_call_back_error_rc(),
                    err_cb_rc_param_t(act_data::act_err_t::invalid_ip_begin, ip_it_current_res.error(), this->data_outer, ip_current_ref),
                    is_end
                );
                return;
            }

            cmd_sgl_it_t ip_it_current = ip_it_current_res.value();
            std::reference_wrapper<cmd_sgl_it_t> ip_it_current_ref = ip_it_current;

            while (is_end == false)
            {
                if (ip_it_current_ref.get() == cmd_set_single_current.get().end())
                {
                    std::invoke
                    (
                        this->call_back_set.get().get_call_back_error(),
                        err_cb_param_t(act_data::act_err_t::invalid_ip_end, ip_current_ref, this->data_outer),
                        is_end
                    );
                    continue;
                }

                cmd_def_set_it_t cmd_define_it = this->cmd_define_set.get().find(static_cast<typename cmd_def_set_t::key_type>(ip_it_current_ref.get()->get_type().get()));

                if (cmd_define_it == this->cmd_define_set.get().end())
                {
                    std::invoke
                    (
                        this->call_back_set.get().get_call_back_error(),
                        err_cb_param_t(act_data::act_err_t::invalid_cmd_type, ip_current_ref, this->data_outer),
                        is_end
                    );
                    continue;
                }

                if (run_hook_handle(cmd_set_single_current, ip_current_ref, ip_it_current_ref, is_end) == false)
                    continue;

                run_interrupt_handle(cmd_set_single_current, cmd_define_it, ip_current_ref, ip_it_current_ref, is_end);

            }

            return;
        }
    };

}