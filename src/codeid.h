/*
 *  Copyright (C) 2017-2018, Thomas Maier-Komor
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

#ifndef CODEID_H
#define CODEID_H

typedef enum {
	ct_invalid = 0,
	ct_json_cstr,
	ct_json_indent,
	ct_json_string,
	ct_mangle_double,
	ct_mangle_float,
	ct_place_varint,
	ct_read_double,
	ct_read_float,
	ct_read_u16,
	ct_read_u32,
	ct_read_u64,
	ct_read_varint,
	ct_send_bytes,
	ct_send_msg,
	ct_send_u16,
	ct_send_u32,
	ct_send_u64,
	ct_send_varint,
	ct_send_xvarint,	// sign extended varint for varintbits < 64
	ct_sint_varint,
	ct_skip_content,
	ct_to_dblstr,
	ct_to_decstr,
	ct_varint_sint,		// convert varint to signed varint
	ct_wiresize,
	ct_wiresize_s,
	ct_wiresize_x,
	ct_write_u16,
	ct_write_u32,
	ct_write_u64,
	ct_write_varint,
	ct_write_xvarint,	// sign extended varint for varintbits < 64
	ct_id_max,		// beginning of unassigned id range
} codeid_t;

#endif
