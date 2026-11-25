#include <cstdio>
#include <vector>
#include <optional>
#include "manager.h"

namespace mgr_ns = rule_manager;

using mgr_t = mgr_ns::manager<int, 4>;
using it_t = mgr_t::iterator;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", ok ? "PASS" : "FAIL", name);
	return ok;
}

static bool fill(mgr_t& m, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (m.push_back(i).has_value() == false)
			return false;
	}

	return true;
}

static bool sequence_ok(mgr_t& m, const std::vector<int>& expected)
{
	if (m.size() != expected.size())
		return false;

	std::size_t i = 0;

	for (auto it = m.begin(); it != m.end(); ++it, i++)
	{
		if (*it != expected[i])
			return false;
	}

	for (std::size_t k = 0; k < expected.size(); k++)
	{
		auto v = m.at(k);

		if (v.has_value() == false || v.value().get() != expected[k])
			return false;
	}

	return true;
}

int main()
{
	bool ok = true;

	// 1. 无分裂，同块插入在游标前
	{
		mgr_t m;
		fill(m, 3);
		it_t cursor = m.begin() + 2;
		auto r = m.insert_ex(cursor, m.begin(), 99);
		ok &= expect("insert_no_split_same_block", r.has_value() && *cursor == 2 && sequence_ok(m, { 99, 0, 1, 2 }));
	}

	// 2. 分裂，游标留在旧块
	{
		mgr_t m;
		fill(m, 4);
		it_t cursor = m.begin();
		auto r = m.insert_ex(cursor, m.begin(), 99);
		ok &= expect("insert_split_cursor_old", r.has_value() && *cursor == 0 && sequence_ok(m, { 99, 0, 1, 2, 3 }));
	}

	// 3. 分裂，游标进入新块
	{
		mgr_t m;
		fill(m, 4);
		it_t cursor = m.begin() + 2;
		auto r = m.insert_ex(cursor, m.begin(), 99);
		ok &= expect("insert_split_cursor_new", r.has_value() && *cursor == 2 && sequence_ok(m, { 99, 0, 1, 2, 3 }));
	}

	// 4. 分裂，插入落在新块，游标在新块
	{
		mgr_t m;
		fill(m, 4);
		it_t cursor = m.begin() + 3;
		auto r = m.insert_ex(cursor, m.begin() + 2, 99);
		ok &= expect("insert_split_into_new", r.has_value() && *cursor == 3 && sequence_ok(m, { 0, 1, 99, 2, 3 }));
	}

	// 5. 分裂发生在游标之前的块
	{
		mgr_t m;
		fill(m, 6);
		it_t cursor = m.begin() + 4;
		auto r = m.insert_ex(cursor, m.begin(), 99);
		ok &= expect("insert_split_earlier_block", r.has_value() && *cursor == 4 && sequence_ok(m, { 99, 0, 1, 2, 3, 4, 5 }));
	}

	// 6. 无分裂，插入在游标之前的另一块
	{
		mgr_t m;
		fill(m, 6);
		m.erase(m.begin());
		it_t cursor = m.begin() + 3;
		auto r = m.insert_ex(cursor, m.begin() + 1, 99);
		ok &= expect("insert_earlier_block_no_split", r.has_value() && *cursor == 4 && sequence_ok(m, { 1, 99, 2, 3, 4, 5 }));
	}

	// 7. 删除同块中游标之前的元素
	{
		mgr_t m;
		fill(m, 3);
		it_t cursor = m.begin() + 2;
		auto r = m.erase_ex(cursor, m.begin());
		ok &= expect("erase_before_same_block", r.has_value() && *cursor == 2 && sequence_ok(m, { 1, 2 }));
	}

	// 8. 删除游标之前整块（块被移除）
	{
		mgr_t m;
		fill(m, 8);
		m.erase(m.begin());
		m.erase(m.begin());
		m.erase(m.begin());
		it_t cursor = m.begin() + 1;
		auto r = m.erase_ex(cursor, m.begin());
		ok &= expect("erase_removed_block_before", r.has_value() && *cursor == 4 && sequence_ok(m, { 4, 5, 6, 7 }));
	}

	// 9. 删除游标自身，同块后继顶上
	{
		mgr_t m;
		fill(m, 3);
		it_t cursor = m.begin() + 1;
		auto r = m.erase_ex(cursor, m.begin() + 1);
		ok &= expect("erase_cursor_same_block", r.has_value() && *cursor == 2 && sequence_ok(m, { 0, 2 }));
	}

	// 10. 删除游标自身，块尾，后继块顶上
	{
		mgr_t m;
		fill(m, 6);
		it_t cursor = m.begin() + 3;
		auto r = m.erase_ex(cursor, m.begin() + 3);
		ok &= expect("erase_cursor_block_tail", r.has_value() && *cursor == 4 && sequence_ok(m, { 0, 1, 2, 4, 5 }));
	}

	// 11. 删除游标自身（最后一个元素）-> end
	{
		mgr_t m;
		fill(m, 3);
		it_t cursor = m.begin() + 2;
		auto r = m.erase_ex(cursor, m.begin() + 2);
		ok &= expect("erase_cursor_end", r.has_value() && cursor == m.end() && sequence_ok(m, { 0, 1 }));
	}

	// 12. 删除游标自身，所在块只有 1 个元素（块移除，后继块顶上）
	{
		mgr_t m;
		fill(m, 8);
		m.erase(m.begin());
		m.erase(m.begin());
		m.erase(m.begin());
		it_t cursor = m.begin();
		auto r = m.erase_ex(cursor, m.begin());
		ok &= expect("erase_cursor_removed_block", r.has_value() && *cursor == 4 && sequence_ok(m, { 4, 5, 6, 7 }));
	}

	// 13. emplace_ex 无分裂，同块插入在游标前
	{
		mgr_t m;
		fill(m, 3);
		it_t cursor = m.begin() + 2;
		auto r = m.emplace_ex(cursor, m.begin(), 99);
		ok &= expect("emplace_no_split_same_block", r.has_value() && *cursor == 2 && sequence_ok(m, { 99, 0, 1, 2 }));
	}

	// 14. emplace_ex 块分裂，游标进入新块
	{
		mgr_t m;
		fill(m, 4);
		it_t cursor = m.begin() + 2;
		auto r = m.emplace_ex(cursor, m.begin(), 99);
		ok &= expect("emplace_split_cursor_new", r.has_value() && *cursor == 2 && sequence_ok(m, { 99, 0, 1, 2, 3 }));
	}

	// 15. emplace_back_ex：追加不移动游标（O(1)）
	{
		mgr_t m;
		fill(m, 3);
		it_t cursor = m.begin() + 1;
		auto r = m.emplace_back_ex(cursor, 99);
		ok &= expect("emplace_back_keeps_cursor", r.has_value() && *cursor == 1 && sequence_ok(m, { 0, 1, 2, 99 }));
	}

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
