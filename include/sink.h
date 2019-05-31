/*
 *  Copyright (C) 2017-2019, Thomas Maier-Komor
 *
 *  This source file belongs to Wire-Format-Compiler.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SINK_H
#define _SINK_H

#include <string.h>
#include <stdint.h>
#include <sys/types.h>

class Sink
{
	public:
	virtual void sink_vi(uint64_t v) = 0;
	virtual void sink8(uint8_t u8) = 0;
	virtual void sink16(uint16_t u8) = 0;
	virtual void sink32(uint32_t u8) = 0;
	virtual void sink64(uint64_t u8) = 0;
	virtual void sink_bytes(const uint8_t *, size_t) = 0;
};


class DetermineSize : public Sink
{
	size_t size;

	public:
	DetermineSize()
	: size(0)
	{ }

	void sink_vi(uint64_t v)
	{
		do {
			v >>= 7;
			++size;
		} while (v);
	}

	void sink8(uint8_t u8)
	{
		++size;
	}

	void sink16(uint16_t v)
	{
		size += 2;
	}

	void sink32(uint32_t v)
	{
		size += 4;
	}

	void sink64(uint64_t v)
	{
		size += 8;
	}

	void sink_bytes(const uint8_t *b, size_t n)
	{
		size += n;
	}

	ssize_t getSize() const
	{
		return size;
	}
};


class ToBufferUnchecked : public Sink
{
	uint8_t *buffer, *at;

	public:
	ToBufferUnchecked(uint8_t *b)
	: buffer(b)
	, at(b)
	{ }

	void sink_vi(uint64_t v)
	{
		do {
			uint8_t u8 = v & 0x7f;
			v >>= 7;
			if (v)
				u8 |= 0x80;
			*at++ = u8;
		} while (v);
	}

	void sink8(uint8_t u8)
	{
		*at++ = u8;
	}

	void sink16(uint16_t v)
	{
		*(uint16_t *)at = v;
		at += 2;
	}

	void sink32(uint32_t v)
	{
		*(uint32_t *)at = v;
		at += 4;
	}

	void sink64(uint64_t v)
	{
		*(uint64_t *)at = v;
		at += 8;
	}

	void sink_bytes(const uint8_t *b, size_t n)
	{
		memcpy(at,b,n);
		at += n;
	}

	ssize_t getSize() const
	{
		return at-buffer;
	}

	uint8_t *getBuffer() const
	{
		return buffer;
	}
};


#ifdef WITH_SINK_STRING
class SinkString : public Sink
{
	std::string &str;

	public:
	SinkString(std::string &s)
	: str(s)
	{

	}

	void sink_vi(uint64_t v)
	{
		do {
			uint8_t u8 = v & 0x7f;
			v >>= 7;
			if (v)
				u8 |= 0x80;
			str.push_back(u8);
		} while (v);
	}

	void sink8(uint8_t u8)
	{
		str.push_back(u8);
	}

	void sink16(uint16_t v)
	{
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v);
	}

	void sink32(uint32_t v)
	{
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v);
	}

	void sink64(uint64_t v)
	{
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v & 0xff);
		v >>= 8;
		str.push_back(v);
	}

	void sink_bytes(const uint8_t *b, size_t n)
	{
		str.append((const char *)b,n);
	}

};
#endif // WITH_SINK_STRING


#endif
