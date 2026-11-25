#pragma once
#include <cstdint>
#include <cstring>
#include <istream>
#include <ostream>
#include <span>
#include "serialize_data_def.h"

namespace rule_serialize
{
	inline std::uint32_t crc32_update(std::uint32_t crc, std::uint8_t byte_v)
	{
		crc ^= static_cast<std::uint32_t>(byte_v);
		crc = (crc >> 8) ^ (0xEDB88320u & (0u - (crc & 1u)));
		return crc;
	}

	inline std::uint32_t crc32_finish(std::uint32_t crc)
	{
		return ~crc;
	}

	inline void write_u32_raw(std::ostream& out_v, std::uint32_t value_v)
	{
		char bytes_v[4] =
		{
			static_cast<char>((value_v >> 24) & 0xff),
			static_cast<char>((value_v >> 16) & 0xff),
			static_cast<char>((value_v >> 8) & 0xff),
			static_cast<char>(value_v & 0xff),
		};

		out_v.write(bytes_v, 4);
	}

	inline void write_u64_raw(std::ostream& out_v, std::uint64_t value_v)
	{
		char bytes_v[8];

		for (std::size_t i = 0; i < 8; i++)
		{
			bytes_v[i] = static_cast<char>((value_v >> ((7 - i) * 8)) & 0xff);
		}

		out_v.write(bytes_v, 8);
	}

	inline bool read_u32_raw(std::istream& in_v, std::uint32_t& value_save)
	{
		unsigned char bytes_v[4];

		if (!in_v.read(reinterpret_cast<char*>(bytes_v), 4))
			return false;

		value_save = (static_cast<std::uint32_t>(bytes_v[0]) << 24) |
			(static_cast<std::uint32_t>(bytes_v[1]) << 16) |
			(static_cast<std::uint32_t>(bytes_v[2]) << 8) |
			static_cast<std::uint32_t>(bytes_v[3]);

		return true;
	}

	inline bool read_u64_raw(std::istream& in_v, std::uint64_t& value_save)
	{
		unsigned char bytes_v[8];

		if (!in_v.read(reinterpret_cast<char*>(bytes_v), 8))
			return false;

		value_save = 0;

		for (std::size_t i = 0; i < 8; i++)
		{
			value_save = (value_save << 8) | static_cast<std::uint64_t>(bytes_v[i]);
		}

		return true;
	}

	class stream_writer
	{
	private:
		std::ostream& out;
		std::uint32_t crc = 0xFFFFFFFFu;
		std::uint64_t count = 0;
	public:
		explicit stream_writer(std::ostream& out_v) noexcept :
			out(out_v)
		{
		}
	public:
		std::uint64_t get_count() const noexcept
		{
			return this->count;
		}
		std::uint32_t get_crc() const noexcept
		{
			return crc32_finish(this->crc);
		}
		bool good() const noexcept
		{
			return this->out.good();
		}
	public:
		void put_u8(std::uint8_t value_v)
		{
			const char byte_v = static_cast<char>(value_v);
			this->crc = crc32_update(this->crc, value_v);
			this->count++;
			this->out.write(&byte_v, 1);
		}
		void put_u32(std::uint32_t value_v)
		{
			for (std::size_t i = 0; i < 4; i++)
				this->put_u8(static_cast<std::uint8_t>(value_v >> ((3 - i) * 8)));
		}
		void put_u64(std::uint64_t value_v)
		{
			for (std::size_t i = 0; i < 8; i++)
				this->put_u8(static_cast<std::uint8_t>(value_v >> ((7 - i) * 8)));
		}
		void put_bytes(std::span<const byte_t> bytes_v)
		{
			for (const byte_t byte_v : bytes_v)
				this->put_u8(static_cast<std::uint8_t>(byte_v));

			return;
		}
	};

	class stream_reader
	{
	private:
		std::istream& in;
		std::uint32_t crc = 0xFFFFFFFFu;
		std::uint64_t count = 0;
	public:
		explicit stream_reader(std::istream& in_v) noexcept :
			in(in_v)
		{
		}
	public:
		std::uint64_t get_count() const noexcept
		{
			return this->count;
		}
		std::uint32_t get_crc() const noexcept
		{
			return crc32_finish(this->crc);
		}
		bool good() const noexcept
		{
			return this->in.good();
		}
	public:
		bool get_u8(std::uint8_t& value_save)
		{
			char byte_v = 0;

			if (!this->in.read(&byte_v, 1))
				return false;

			value_save = static_cast<std::uint8_t>(byte_v);
			this->crc = crc32_update(this->crc, value_save);
			this->count++;
			return true;
		}
		bool get_u32(std::uint32_t& value_save)
		{
			value_save = 0;

			for (std::size_t i = 0; i < 4; i++)
			{
				std::uint8_t byte_v = 0;

				if (this->get_u8(byte_v) == false)
					return false;

				value_save = (value_save << 8) | static_cast<std::uint32_t>(byte_v);
			}

			return true;
		}
		bool get_u64(std::uint64_t& value_save)
		{
			value_save = 0;

			for (std::size_t i = 0; i < 8; i++)
			{
				std::uint8_t byte_v = 0;

				if (this->get_u8(byte_v) == false)
					return false;

				value_save = (value_save << 8) | static_cast<std::uint64_t>(byte_v);
			}

			return true;
		}
		bool get_bytes(std::span<byte_t> bytes_save)
		{
			for (byte_t& byte_v : bytes_save)
			{
				std::uint8_t value_v = 0;

				if (this->get_u8(value_v) == false)
					return false;

				byte_v = static_cast<byte_t>(value_v);
			}

			return true;
		}
	};
}
