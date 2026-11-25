#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <utility>
#include <type_traits>
#include <concepts>
#include <optional>
#include <functional>
#include <expected>
#include "act_data_def.h"

namespace rule_act_concept
{
    template<typename T>
    concept concept_rc_cmd = requires
    (
        T rc_cmd_test,
        typename T::index_type rc_index_test,
        typename T::value_type rc_sgl_test,
        typename T::resource_type cmd_test,
        typename T::resource_type::index_type cmd_index_test,
        typename T::resource_type::iterator cmd_it_test,
        typename T::resource_type::command_type cmd_sgl_test
    )
    {
        { rc_cmd_test.get(rc_index_test) } -> std::same_as<std::expected<std::reference_wrapper<typename T::value_type>, typename T::unexpected_error_type>>;
        { rc_sgl_test.get_content() } -> std::same_as<typename T::resource_type&>;
        { cmd_test.get(cmd_index_test) } -> std::same_as<std::expected<typename T::resource_type::command_type, typename T::resource_type::unexpected_error_type>>;
        { cmd_test.get_it(cmd_index_test) } -> std::same_as<std::expected<typename T::resource_type::iterator, typename T::resource_type::unexpected_error_type>>;
        { cmd_test.begin() } -> std::same_as<typename T::resource_type::iterator>;
        { cmd_test.end() } -> std::same_as<typename T::resource_type::iterator>;
        { ++cmd_it_test } -> std::same_as<typename T::resource_type::iterator&>;
        { cmd_it_test++ } -> std::same_as<typename T::resource_type::iterator>;
        { cmd_it_test->get_index() } -> std::same_as<typename T::resource_type::index_type&>;
        { cmd_it_test->get_type() } -> std::same_as<std::reference_wrapper<typename T::resource_type::command_define_id_type>&>;
        { cmd_it_test.is_valid() } -> std::same_as<bool>;
        { cmd_sgl_test.get_index() } -> std::same_as<typename T::resource_type::index_type&>;
        { cmd_sgl_test.get_type() } -> std::same_as<std::reference_wrapper<typename T::resource_type::command_define_id_type>&>;
    };

    template<typename T, typename rc_cmd>
    concept concept_act_stk = requires(T stack_test, act_data::act_cmd_ip<rc_cmd> stack_ip_test)
    {
        requires concept_rc_cmd<rc_cmd>;

        stack_test.emplace_back(stack_ip_test);
        { stack_test.back() } -> std::same_as<act_data::act_cmd_ip<rc_cmd>&>;
        { stack_test.empty() } -> std::same_as<bool>;
        stack_test.pop_back();
        stack_test.clear();
    };

    template<typename T, typename rc_cmd, typename interrupt_info, typename outer>
    concept concept_cmd_def = requires(T cmd_def_test, act_data::act_run_param<rc_cmd, outer>&param_test)
    {
        requires concept_rc_cmd<rc_cmd>;

        { std::invoke(cmd_def_test, param_test) } -> std::same_as<act_data::act_run_int<rc_cmd, interrupt_info>>;
    };

    template<typename T, typename rc_cmd, typename interrupt_info, typename outer>
    concept concept_cmd_def_set = requires
    (
        T cmd_def_set_test,
        typename T::key_type cmd_def_set_key_test,
        typename T::iterator cmd_def_set_it_test,
        act_data::act_run_param<rc_cmd, outer>&cmd_def_set_call_param_test,
        typename T::value_type cmd_def_set_v_test
    )
    {
        requires concept_cmd_def<typename T::mapped_type, rc_cmd, interrupt_info, outer>;

        { cmd_def_set_test.find(cmd_def_set_key_test) } -> std::same_as<typename T::iterator>;
        { cmd_def_set_it_test.operator->() } -> std::same_as<typename T::value_type*>;
        { std::invoke(cmd_def_set_v_test.second, cmd_def_set_call_param_test) } -> std::same_as<act_data::act_run_int<rc_cmd, interrupt_info>>;
    };

    template<typename T, typename rc_cmd, typename interrupt_info, typename outer>
    concept concept_int_call_back = requires(T cb_test, act_data::act_run_param<rc_cmd, outer>&cb_param_run_test, act_data::act_run_int<rc_cmd, interrupt_info>&cb_param_int_test, bool& cb_param_end_test)
    {
        requires concept_rc_cmd<rc_cmd>;

        { std::invoke(cb_test, cb_param_run_test, cb_param_int_test, cb_param_end_test) } -> std::same_as<void>;
    };

    template<typename T, typename rc_cmd, typename outer>
    concept concept_err_call_back_rc = requires(T cb_test, act_data::act_err_param<rc_cmd, typename rc_cmd::unexpected_error_type, outer> cb_param_err_test, bool& cb_param_end_test)
    {
        requires concept_rc_cmd<rc_cmd>;

        { std::invoke(cb_test, cb_param_err_test, cb_param_end_test) } -> std::same_as<void>;
    };

    template<typename T, typename rc_cmd, typename outer>
    concept concept_err_call_back = requires(T cb_test, act_data::act_err_param_base<rc_cmd, outer> cb_param_err_test, bool& cb_param_end_test)
    {
        requires concept_rc_cmd<rc_cmd>;
        { std::invoke(cb_test, cb_param_err_test, cb_param_end_test) } -> std::same_as<void>;
    };

    template<typename T, typename rc_cmd, typename outer>
    concept concept_hook_call_back = requires(T cb_test, act_data::act_hook_param<rc_cmd, outer> cb_param_hook_test)
    {
        requires concept_rc_cmd<rc_cmd>;
        { std::invoke(cb_test, cb_param_hook_test) } -> std::same_as<act_data::act_hook_t>;
    };

    template<typename T, typename rc_cmd, typename outer>
    concept concept_call_back = requires(T cb_test)
    {
        requires concept_int_call_back<typename T::call_back_interrupt_type, rc_cmd, typename T::call_back_interrupt_info_type, outer>;
        requires concept_err_call_back_rc<typename T::call_back_error_resource_type, rc_cmd, outer>;
        requires concept_err_call_back<typename T::call_back_error_type, rc_cmd, outer>;
        requires concept_hook_call_back<typename T::call_back_hook_type, rc_cmd, outer>;

        { cb_test.get_call_back_interrupt() } -> std::same_as<typename T::call_back_interrupt_type&>;
        { cb_test.get_call_back_error_rc() } -> std::same_as<typename T::call_back_error_resource_type&>;
        { cb_test.get_call_back_error() } -> std::same_as<typename T::call_back_error_type&>;
        { cb_test.get_call_back_hook() } -> std::same_as<typename T::call_back_hook_type&>;
    };

}