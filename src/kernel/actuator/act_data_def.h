#pragma once
#include <cstdint>
#include <utility>
#include <type_traits>
#include <concepts>
#include <optional>
#include <functional>
#include <expected>

namespace rule_act_data_def
{
    using act_data_type_base_8 = std::uint8_t;
    using act_data_type_base_16 = std::uint16_t;
    using act_data_type_base_32 = std::uint32_t;
    using act_data_type_base_64 = std::uint64_t;

    template <typename rc_cmd>
    struct act_cmd_ip
    {
    public:
        using command_set_index = typename rc_cmd::index_type;
        using command_index = typename rc_cmd::resource_type::index_type;
    public:
        command_set_index cmd_set_index;
        command_index index;
    public:
        template<typename self_this>
        decltype(auto) get_cmd_set_index(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.cmd_set_index);
        }
        template<typename self_this>
        decltype(auto) get_index(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.index);
        }
    public:
        void set_cmd_set_index(command_set_index cmd_set_index_v)
        {
            this->cmd_set_index = cmd_set_index_v;
        }
        void set_index(command_index index_v)
        {
            this->index = index_v;
        }
    public:
        act_cmd_ip() noexcept :
            cmd_set_index(), index()
        {

        }
        act_cmd_ip(command_set_index cmd_set_index_v, command_index index_v) noexcept :
            cmd_set_index(cmd_set_index_v), index(index_v)
        {

        }
        act_cmd_ip(const act_cmd_ip& other_v) noexcept :
            cmd_set_index(other_v.cmd_set_index), index(other_v.index)
        {

        }
        act_cmd_ip& operator=(const act_cmd_ip& other_v) noexcept
        {
            if (this == &other_v)
                return *this;

            this->cmd_set_index = other_v.cmd_set_index;
            this->index = other_v.index;

            return *this;
        }
        act_cmd_ip(act_cmd_ip&& other_v) noexcept :
            cmd_set_index(std::move(other_v.cmd_set_index)), index(std::move(other_v.index))
        {

        }
        act_cmd_ip& operator=(act_cmd_ip&& other_v) noexcept
        {
            if (this == &other_v)
                return *this;

            this->cmd_set_index = std::move(other_v.cmd_set_index);
            this->index = std::move(other_v.index);

            return *this;
        }
    };

    template <typename rc_cmd, typename outer>
    struct act_run_param
    {
    public:
        using cmd_sgl_it_t = typename rc_cmd::resource_type::iterator;
        using outer_ptr = outer*;
    public:
        act_cmd_ip<rc_cmd> ip;
        std::reference_wrapper<cmd_sgl_it_t> cmd_it;
        outer_ptr data_outer;
    public:
        template<typename self_this>
        decltype(auto) get_ip(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.ip);
        }
        template<typename self_this>
        decltype(auto) get_cmd_it(this self_this&& self) noexcept
        {
            return self.cmd_it.get();
        }
        template<typename self_this>
        decltype(auto) get_data_outer(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.data_outer);
        }
    public:
        void set_op_val(const act_cmd_ip<rc_cmd>& ip_v)
        {
            this->ip = ip_v;
        }
        void set_cmd_it_val(cmd_sgl_it_t& cmd_it_v)
        {
            this->cmd_it = std::ref(cmd_it_v);
        }
        void set_data_outer(outer_ptr data_outer_v)
        {
            this->data_outer = data_outer_v;
        }
    public:
        act_run_param() = delete;
        act_run_param(const act_cmd_ip<rc_cmd>& ip_v, cmd_sgl_it_t& cmd_it_v, outer_ptr data_outer_v) :
            ip(ip_v), cmd_it(cmd_it_v), data_outer(data_outer_v)
        {

        }
    };

    enum class act_run_int_t : act_data_type_base_8
    {
        invalid,
        next,
        invoke_end,
        invoke,
        error,
        exit,
    };

    enum class act_err_t : act_data_type_base_8
    {
        invalid_ip,
        invalid_ip_end,
        invalid_ip_begin,
        invalid_ip_invoke,
        invalid_ip_stack,
        invalid_ip_update,
        invalid_cmd_type,
    };

    enum class act_hook_t : act_data_type_base_8
    {
        pass,
        ip_change,
        exit,
    };

    template <typename rc_cmd, typename outer>
    struct act_err_param_base
    {
    public:
        using outer_ptr = outer*;
    public:
        act_err_t error;
        std::reference_wrapper<std::reference_wrapper<act_cmd_ip<rc_cmd>>> ip;
        outer_ptr data_outer;
    public:
        template<typename self_this>
        decltype(auto) get_error(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.error);
        }
        template<typename self_this>
        decltype(auto) get_ip(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.ip.get().get());
        }
        template<typename self_this>
        decltype(auto) get_data_outer(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.data_outer);
        }
    public:
        void set_error(act_err_t error_v)
        {
            this->error = error_v;
        }
        void set_ip_val(act_cmd_ip<rc_cmd> ip_v)
        {
            this->ip.get().get() = ip_v;
        }
        void set_data_outer(outer_ptr data_outer_v)
        {
            this->data_outer = data_outer_v;
        }
    public:
        act_err_param_base() = delete;
        act_err_param_base(act_err_t error_v, std::reference_wrapper<act_cmd_ip<rc_cmd>>& ip_v, outer_ptr data_outer_v) :
            error(error_v), ip(ip_v), data_outer(data_outer_v)
        {

        }
    };

    template <typename rc_cmd, typename err_code_outer_t, typename outer>
    struct act_err_param
    {
    public:
        using outer_ptr = outer*;
    public:
        act_err_param_base<rc_cmd, outer> base;
        err_code_outer_t error_outer;
    public:
        template<typename self_this>
        decltype(auto) get_base(this self_this&& self)
        {
            return std::forward_like<self_this>(self.base);
        }
        template<typename self_this>
        decltype(auto) get_error_outer(this self_this&& self)
        {
            return std::forward_like<self_this>(self.error_outer);
        }
    public:
        act_err_param() = delete;
        act_err_param(act_err_t error_v, err_code_outer_t error_outer_v, outer_ptr base_v, std::reference_wrapper<act_cmd_ip<rc_cmd>>& ip_v) :
            base(error_v, ip_v, base_v), error_outer(error_outer_v)
        {

        }
    };

    template <typename rc_cmd, typename outer>
    struct act_hook_param
    {
    public:
        using outer_ptr = outer*;
        using command_single_it_type = typename rc_cmd::resource_type::iterator;
        using cmd_sgl_it_t = command_single_it_type;
    public:
        std::reference_wrapper<std::reference_wrapper<act_cmd_ip<rc_cmd>>> ip;
        std::reference_wrapper<std::reference_wrapper<cmd_sgl_it_t>> cmd_it;
        outer_ptr data_outer;
    public:
        template<typename self_this>
        decltype(auto) get_ip(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.ip.get().get());
        }
        template<typename self_this>
        decltype(auto) get_cmd_it(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.cmd_it.get().get());
        }
        template<typename self_this>
        decltype(auto) get_data_outer(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.data_outer);
        }
    public:
        void set_ip_val(act_cmd_ip<rc_cmd> ip_v)
        {
            this->ip.get().get() = ip_v;
        }
        void set_cmd_it_val(cmd_sgl_it_t cmd_it_v)
        {
            this->cmd_it.get().get() = cmd_it_v;
        }
        void set_data_outer(outer_ptr data_outer_v)
        {
            this->data_outer = data_outer_v;
        }
    public:
        act_hook_param() = delete;
        act_hook_param(std::reference_wrapper<act_cmd_ip<rc_cmd>>& ip_v, std::reference_wrapper<cmd_sgl_it_t>& cmd_it_v, outer_ptr data_outer_v) :
            ip(ip_v), cmd_it(cmd_it_v), data_outer(data_outer_v)
        {

        }
    };

    template<typename rc_cmd, typename T>
    struct act_run_int
    {
    public:
        act_run_int_t type;
        act_cmd_ip<rc_cmd> ip_current;
        std::optional<act_cmd_ip<rc_cmd>> ip_target;
        T info;
    public:
        template<typename self_this>
        decltype(auto) get_type(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.type);
        }
        template<typename self_this>
        decltype(auto) get_ip_current(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.ip_current);
        }
        template<typename self_this>
        decltype(auto) get_ip_target(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.ip_target);
        }
        template<typename self_this>
        decltype(auto) get_info(this self_this&& self) noexcept
        {
            return std::forward_like<self_this>(self.info);
        }
    public:
        void set_type(act_run_int_t type_v)
        {
            this->type = type_v;
        }
        void set_ip_current(act_cmd_ip<rc_cmd> ip_current_v)
        {
            this->ip_current = ip_current_v;
        }
        void set_ip_target(std::optional<act_cmd_ip<rc_cmd>> ip_target_v)
        {
            this->ip_target = ip_target_v;
        }
        void set_info(const T& info_v)
        {
            this->info = info_v;
        }
    public:
        act_run_int() :
            type(act_run_int_t::invalid), ip_current(), ip_target(std::nullopt), info()
        {

        }
        act_run_int(act_run_int_t type_v, act_cmd_ip<rc_cmd> ip_current_v, std::optional<act_cmd_ip<rc_cmd>> ip_target_v, const T& info_v) :
            type(type_v), ip_current(ip_current_v), ip_target(ip_target_v), info(info_v)
        {

        }
        act_run_int(const act_run_int& other_v) :
            type(other_v.type), ip_current(other_v.ip_current), ip_target(other_v.ip_target), info(other_v.info)
        {

        }
        act_run_int& operator=(const act_run_int& other_v)
        {
            if (this == &other_v)
                return *this;

            this->type = other_v.type;
            this->ip_current = other_v.ip_current;
            this->ip_target = other_v.ip_target;
            this->info = other_v.info;

            return *this;
        }
        act_run_int(act_run_int&& other_v) noexcept :
            type(std::move(other_v.type)), ip_current(std::move(other_v.ip_current)), ip_target(std::move(other_v.ip_target)), info(std::move(other_v.info))
        {
            other_v.type = act_run_int_t::invalid;
        }
        act_run_int& operator=(act_run_int&& other_v)
        {
            if (this == &other_v)
                return *this;

            this->type = std::exchange(other_v.type, act_run_int_t::invalid);
            this->ip_current = std::move(other_v.ip_current);
            this->ip_target = std::move(other_v.ip_target);
            this->info = std::move(other_v.info);

            return *this;
        }
    };

    enum class stop_flag : act_data_type_base_8
    {
        stop_ready = 0,
        run_ready = 1,
        run = 2,
        stop_send = 3,
        stop_write = 4,
    };
}