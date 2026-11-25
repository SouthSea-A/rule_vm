#pragma once
#include <cstdint>
#include <string>
#include <exception>

enum class [[nodiscard]] kernel_error
{
	manager__cache_mapping__add_set__size_max,
	manager__cache_mapping__add_set__order_out_of_range,
	manager__cache_mapping__add_set__resize_unexpected,
	manager__cache_mapping__add_set__read_unexpected,

	manager__cache_mapping__insert_front__size_max,
	manager__cache_mapping__insert_front__order_out_of_range,
	manager__cache_mapping__insert_front__resize_unexpected,
	manager__cache_mapping__insert_front__read_unexpected,
	manager__cache_mapping__insert_front__insert_unexpected,

	manager__cache_mapping__get__order_out_of_range,
	manager__cache_mapping__get__read_unexpected,
	manager__cache_mapping__get__read_pos_invalid,

	manager__cache_mapping__get_last__order_out_of_range,

	manager__cache_mapping__del__order_out_of_range,
	manager__cache_mapping__del__erase_unexpected,
	//--------------------------------------------------------------------

	manager__page__get_free__empty,

	manager__page__get_element_ref_by_order__read_unexpected,

	manager__page__get_element_ref_by_pos__index_pos_invalid,
	manager__page__get_element_ref_by_pos__read_unexpected,

	manager__page__get_element_const_ref_by_pos__index_pos_invalid,
	manager__page__get_element_const_ref_by_pos__read_unexpected,

	manager__page__tidy_element__resize_unexpected,
	manager__page__tidy_element__shrink_to_fit_unexpected,

	manager__page__add_free_by_order__emplace_back_unexpected,

	manager__page__add_free_by_pos__emplace_back_unexpected,

	manager__page__add_free_roll_back__emplace_back_unexpected,

	manager__page__add__placeholder,
	manager__page__add__element_missing,
	manager__page__add__size_max,
	manager__page__add__emplace_back_unexpected,
	manager__page__add__emplace_unexpected,
	manager__page__add__element_deconstruct_unexpected,


	manager__page__insert_front__placeholder,
	manager__page__insert_front__size_max,
	manager__page__insert_front__emplace_back_unexpected,
	manager__page__insert_front__emplace_unexpected,

	manager__page__insert_back__placeholder,
	manager__page__insert_back__size_max,
	manager__page__insert_back__emplace_back_unexpected,
	manager__page__insert_back__emplace_unexpected,

	manager__page__write__copy_unexpected,

	manager__page__cut__index_invalid,
	manager__page__cut__size_max,
	manager__page__cut__element_missing,
	//--------------------------------------------------------------------

	manager__cache_prefix_mapping__init_rng__allocate_unexpected,
	manager__cache_prefix_mapping__init_rng__construct_unexpected,

	manager__cache_prefix_mapping__insert__value_invalid,
	manager__cache_prefix_mapping__insert__pos_out_of_range,
	manager__cache_prefix_mapping__insert__node_allocate_unexpected,
	manager__cache_prefix_mapping__insert__node_construct_unexpected,

	manager__cache_prefix_mapping__update__value_invalid,
	manager__cache_prefix_mapping__update__pos_out_of_range,
	manager__cache_prefix_mapping__update__node_missing,

	manager__cache_prefix_mapping__prefix_before_pos__pos_out_of_range,

	manager__cache_prefix_mapping__locate_by_index__index_negative,
	manager__cache_prefix_mapping__locate_by_index__index_out_of_range,
	manager__cache_prefix_mapping__locate_by_index__node_missing,

	manager__cache_prefix_mapping__copy_construct__clone_unexpected,
	manager__cache_prefix_mapping__copy_assign__clone_unexpected,
	manager__cache_prefix_mapping__move_assign__clone_unexpected,
	//--------------------------------------------------------------------

	manager__iterator__test_validity_iterator__manager_invalid,

	manager__iterator__operator_star__iterator_invalid,
	manager__iterator__operator_arrow__iterator_invalid,
	manager__iterator__operator_compare__manager_mismatch,
	manager__iterator__operator_minus__manager_mismatch,
	//--------------------------------------------------------------------

	manager__manager__get_block_ref__index_invalid,
	manager__manager__get_block_ref_const__index_invalid,

	manager__manager__add_block__emplace_back_unexpected,
	manager__manager__add_block__emplace_unexpected,

	manager__manager__insert_front_block__emplace_back_unexpected,
	manager__manager__insert_front_block__emplace_unexpected,

	manager__manager__insert_back_block__emplace_back_unexpected,
	manager__manager__insert_back_block__emplace_unexpected,

	manager__manager__free_roll_back__emplace_back_unexpected,

	manager__manager__add_free__emplace_back_unexpected,
	manager__manager__get_free__free_empty,

	manager__manager__block_add__emplace_right_unexpected,

	manager__manager__block_insert_front__block_invalid_left,
	manager__manager__block_insert_front__order_invalid,
	manager__manager__block_insert_front__emplace_right_unexpected,

	manager__manager__block_insert_back__block_invalid_left,
	manager__manager__block_insert_back__order_invalid,
	manager__manager__block_insert_back__emplace_right_unexpected,

	manager__manager__block_get_const__block_invalid_left,
	manager__manager__block_get_const__order_invalid,

	manager__manager__block_get__block_invalid_left,
	manager__manager__block_get__order_invalid,

	manager__manager__block_write__block_invalid_left,
	manager__manager__block_write__order_invalid,

	manager__manager__block_del__block_invalid_left,
	manager__manager__block_del__order_invalid,

	manager__manager__get_index_block_by_order__order_invalid,

	manager__manager__pop_back__empty,
	manager__manager__back__empty,
	manager__manager__front__empty,

	manager__manager__copy_construct__copy_unexpected,
	manager__manager__copy_assign__copy_unexpected,
	//--------------------------------------------------------------------

	variable__var_content__var_content__resize_unexpected,
	variable__var_content__copy_construct__copy_unexpected,
	variable__var_content__copy_assign__copy_unexpected,

	variable__var_content__read__var_pos_invalid,
	variable__var_content__read__var_width_invalid,

	variable__var_content__widen__resize_unexpected,

	variable__var_content__del__var_pos_invalid,
	variable__var_content__del__var_width_invalid,
	variable__var_content__del__erase_unexpected,

	variable__var_content__read__range_not_allocated,

	variable__var_content__ensure__range_overflow,
	variable__var_content__ensure__allocate_unexpected,

	variable__var_content__get__content_not_contiguous,
	//--------------------------------------------------------------------

	variable__var_sgl__set_content__copy_unexpected,
	//--------------------------------------------------------------------

	variable__variable_manager__test_validity_var_index_it__index_invalid,
	variable__variable_manager__test_validity_var_index_it_const__index_invalid,

	variable__variable_manager__insert_var_sgl__index_invalid,
	variable__variable_manager__erase_var_sgl__index_invalid,

	variable__variable_manager__add_set__range_out_of_range,
	variable__variable_manager__add_set__copy_unexpected,

	variable__variable_manager__del_it__iterator_invalid,

	variable__variable_manager__copy_construct__copy_unexpected,
	variable__variable_manager__copy_assign__copy_unexpected,
	//--------------------------------------------------------------------

	command__command_manager__del_it__iterator_invalid,
	//--------------------------------------------------------------------

	resource__rc_sgl__rc_sgl__copy_unexpected,
	resource__rc_sgl__set_content__copy_unexpected,
	//--------------------------------------------------------------------

	resource__resource__get_instance_by_rc_index__index_invalid,
	resource__resource__get_instance_const_by_rc_index__index_invalid,

	resource__resource__add__index_invalid,
	resource__resource__destroy_it__iterator_invalid,
	//--------------------------------------------------------------------

	serialize__magic_mismatch,
	serialize__version_mismatch,
	serialize__layout_mismatch,
	serialize__truncated,
	serialize__crc_mismatch,
	serialize__io_error,
	serialize__limit_exceeded,
	serialize__invalid_section,
	serialize__cmd_type_missing,
	serialize__var_restore_failed,
	serialize__cmd_restore_failed,
	serialize__manager_restore_failed,
	serialize__stack_restore_failed,
	//--------------------------------------------------------------------
};

struct kernel_error_throw
{
public:
	kernel_error error_code;
	std::string error_msg;
public:
	template<typename self_this>
	decltype(auto) get_error_code(this self_this&& self)
	{
		return std::forward_like<self_this>(self.error_code);
	}
	template<typename self_this>
	decltype(auto) get_error_msg(this self_this&& self)
	{
		return std::forward_like<self_this>(self.error_msg);
	}
public:
	kernel_error_throw() = delete;
	kernel_error_throw(kernel_error error_code_v, const std::string& error_msg_v) :
		error_code(error_code_v), error_msg(error_msg_v)
	{

	}
};

