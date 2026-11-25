#include <cstdio>
#include <memory>
#include <new>
#include "rule_design.h"

namespace desg = rule_design;

inline int failing_budget = 1 << 20;

template<typename t>
struct failing_allocator
{
public:
	using value_type = t;
public:
	failing_allocator() = default;
	template<typename u>
	failing_allocator(const failing_allocator<u>&) noexcept
	{
	}
	t* allocate(std::size_t count)
	{
		if (failing_budget <= 0)
			throw std::bad_alloc();

		failing_budget--;
		return std::allocator<t>().allocate(count);
	}
	void deallocate(t* pointer_v, std::size_t count) noexcept
	{
		std::allocator<t>().deallocate(pointer_v, count);
	}
	template<typename u>
	struct rebind
	{
		using other = failing_allocator<u>;
	};
};

template<typename t, typename u>
bool operator==(const failing_allocator<t>&, const failing_allocator<u>&) noexcept
{
	return true;
}

template<typename t, typename u>
bool operator!=(const failing_allocator<t>&, const failing_allocator<u>&) noexcept
{
	return false;
}

int main()
{
	bool ok = true;

	{
		using alloc_t = failing_allocator<int>;
		failing_budget = 1 << 20;
		desg::rule_desg<alloc_t> desg;
		desg::rule_desg_text_error<alloc_t> error_v;

		failing_budget = 0;
		bool thrown = false;
		desg::rule_design_error code_v = desg::rule_design_error::none;

		try
		{
			const char* text = "defvar 0 {}\ndefcmd 0 {\nret\n}\nstart defcmd 0\n";
			(void)desg.load_text(text, error_v);
		}
		catch (const desg::rule_design_error_throw& error_v)
		{
			thrown = true;
			code_v = error_v.error_code;
		}
		catch (...)
		{
		}

		std::printf("text_alloc_failure: thrown=%d code=%llu\n", (int)thrown, (unsigned long long)code_v);
		ok &= thrown && (code_v == desg::rule_design_error::internal__container || code_v == desg::rule_design_error::internal__parser);
	}

	{
		using alloc_t = failing_allocator<int>;
		failing_budget = 1 << 20;
		desg::rule_desg<alloc_t> desg;
		desg::rule_desg_text_error<alloc_t> error_v;
		const char* text =
			"defvar 0 {\nvar a 0:0:0:0:8\n}\n"
			"defcmd 0 {\nadd a, a, 1\nret\n}\n"
			"start defcmd 0\n";

		if (desg.load_text(text, error_v) == false)
			return 1;

		failing_budget = 0;
		bool thrown = false;
		desg::rule_design_error code_v = desg::rule_design_error::none;

		try
		{
			desg.run_start();
		}
		catch (const desg::rule_design_error_throw& error_v)
		{
			thrown = true;
			code_v = error_v.error_code;
		}
		catch (...)
		{
		}

		std::printf("run_alloc_failure: thrown=%d code=%llu\n", (int)thrown, (unsigned long long)code_v);
		ok &= thrown && (code_v == desg::rule_design_error::internal__container || code_v == desg::rule_design_error::internal__exception);
	}

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
