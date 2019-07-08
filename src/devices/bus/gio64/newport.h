// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

	newport.h

    SGI "Newport" graphics board emulation

*********************************************************************/

#ifndef MAME_BUS_GIO_NEWPORT_H
#define MAME_BUS_GIO_NEWPORT_H

#pragma once

#include "gio64.h"
#include "screen.h"

#define ENABLE_NEWVIEW_LOG      (0)


/*************************************
 *
 *  XMAP9 Device
 *
 *************************************/

class xmap9_device : public device_t
{
public:
	xmap9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t revision)
		: xmap9_device(mconfig, tag, owner, clock)
	{
		set_revision(revision);
	}

	xmap9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(uint32_t offset);
	void write(uint32_t offset, uint32_t data);

	// Getters
	uint32_t cursor_cmap() const { return m_cursor_cmap; }
	uint32_t popup_cmap() const { return m_popup_cmap; }
	uint32_t mode_entry(uint32_t entry) const { return m_mode_table[entry]; }
	bool is_8bpp() const { return BIT(m_config, 2); }

private:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// Initializers
	void set_revision(uint32_t revision) { m_revision = revision; }

	uint32_t m_config;
	uint32_t m_revision;
	uint32_t m_fifo_available;
	uint32_t m_entries;
	uint32_t m_cursor_cmap;
	uint32_t m_popup_cmap;
	uint32_t m_mode_table_idx;
	uint32_t m_mode_table[0x20];
};

DECLARE_DEVICE_TYPE(XMAP9, xmap9_device)


/*************************************
 *
 *  CMAP Device
 *
 *************************************/

class cmap_device : public device_t
				  , public device_palette_interface
{
public:
	cmap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t revision)
		: cmap_device(mconfig, tag, owner, clock)
	{
		set_revision(revision);
	}

	cmap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(uint32_t offset);
	void write(uint32_t offset, uint32_t data);

	// Getters
	const uint32_t *palette_base() const { return m_palette; }

private:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const override { return 0x2000; }

	// Initializers
	void set_revision(uint32_t revision) { m_revision = revision; }

	uint32_t m_status;
	uint32_t m_revision;
	uint16_t m_palette_idx;
	uint32_t m_palette[0x10000];
};

DECLARE_DEVICE_TYPE(CMAP, cmap_device)


/*************************************
 *
 *  VC2 Device
 *
 *************************************/

class vc2_device : public device_t
{
public:
	vc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(uint32_t offset);
	void write(uint32_t offset, uint32_t data, uint32_t mem_mask);

	// Getters
	int readout_x0() const { return m_readout_x0; }
	int readout_y0() const { return m_readout_y0; }
	int readout_x1() const { return m_readout_x1; }
	int readout_y1() const { return m_readout_y1; }
	uint16_t begin_did_line(int y);
	uint16_t next_did_line_entry();
	uint8_t get_cursor_pixel(int x, int y);

	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	auto vert_int() { return m_vert_int.bind(); }
	auto screen_timing_changed() { return m_screen_timing_changed.bind(); } // Hack. TODO: Figure out a better way

private:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	enum
	{
		CURSOR_FUNC_ENABLE_BIT = 4,
		CURSOR_ENABLE_BIT = 7,

		CURSOR_MODE_BIT = 8,
		CURSOR_MODE_GLYPH = 0,
		CURSOR_MODE_CROSSHAIR = 1,

		CURSOR_SIZE_BIT = 9,
		CURSOR_SIZE_32 = 0,
		CURSOR_SIZE_64 = 1
	};

	void decode_vt_line(uint32_t line, uint32_t line_seq_ptr);
	void decode_vt_table();
	void update_screen_size();

	bool is_cursor_active(int x, int y);

	uint16_t m_vid_entry;
	uint16_t m_cursor_entry;
	uint16_t m_cursor_x;
	uint16_t m_cursor_y;
	uint16_t m_cur_cursor_x;
	uint16_t m_did_entry;
	uint16_t m_scanline_len;
	uint16_t m_ram_addr;
	uint16_t m_vt_frame_ptr;
	uint16_t m_vt_line_ptr;
	uint16_t m_vt_line_run;
	uint16_t m_vt_line_count;
	uint16_t m_cursor_table_ptr;
	uint16_t m_work_cursor_y;
	uint16_t m_did_frame_ptr;
	uint16_t m_did_line_ptr;
	uint16_t m_display_ctrl;
	uint16_t m_config;
	uint8_t m_reg_idx;
	uint16_t m_reg_data;
	std::unique_ptr<uint16_t[]> m_ram;

	std::unique_ptr<uint32_t[]> m_vt_table;

	int m_readout_x0;
	int m_readout_y0;
	int m_readout_x1;
	int m_readout_y1;

	bool m_enable_cursor;

	devcb_write_line m_vert_int;
	devcb_write_line m_screen_timing_changed;

	static const size_t RAM_SIZE;
};

DECLARE_DEVICE_TYPE(XMAP9, xmap9_device)


/*************************************
 *
 *  Newport Device
 *
 *************************************/

class newport_base_device : public device_t
						  , public device_gio64_card_interface
{
public:
	newport_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t global_mask);

	// device_gio_slot_interface overrides
	virtual void install_device() override;

	DECLARE_READ64_MEMBER(rex3_r);
	DECLARE_WRITE64_MEMBER(rex3_w);

	uint32_t screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vrint_w);
	DECLARE_WRITE_LINE_MEMBER(update_screen_size);

protected:
	// device_t overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void device_add_mconfig(machine_config &config, uint32_t xmap_revision, uint32_t cmap_revision);
	void mem_map(address_map &map) override;

	static constexpr device_timer_id DCB_TIMEOUT = 0;

	enum
	{
		DCB_ADDR_VC2,
		DCB_ADDR_CMAP01,
		DCB_ADDR_CMAP0,
		DCB_ADDR_CMAP1,
		DCB_ADDR_XMAP01,
		DCB_ADDR_XMAP0,
		DCB_ADDR_XMAP1,
		DCB_ADDR_RAMDAC,
		DCB_ADDR_CC1,
		DCB_ADDR_AB1,
		DCB_ADDR_PCD = 12
	};

	enum
	{
		STATUS_GFXBUSY          = (1 << 3),
		STATUS_BACKBUSY         = (1 << 4),
		STATUS_VRINT            = (1 << 5),
		STATUS_VIDEOINT         = (1 << 6),
		STATUS_GFIFOLEVEL_SHIFT = 7,
		STATUS_GFIFOLEVEL_MASK  = (0x3f << STATUS_GFIFOLEVEL_SHIFT),
		STATUS_BFIFOLEVEL_SHIFT = 13,
		STATUS_BFIFOLEVEL_MASK  = (0x1f << STATUS_BFIFOLEVEL_SHIFT),
		STATUS_BFIFO_INT        = 18,
		STATUS_GFIFO_INT        = 19
	};

	struct rex3_t
	{
		uint32_t m_draw_mode0;
		bool m_color_host;
		uint32_t m_draw_mode1;
		uint8_t m_plane_enable;
		uint8_t m_plane_depth;
		bool m_rwpacked;
		bool m_rwdouble;
		uint8_t m_hostdepth;
		uint8_t m_sfactor;
		uint8_t m_dfactor;
		uint8_t m_logicop;

		uint32_t m_store_shift;
		uint32_t m_host_shift;

		uint32_t m_write_width;
		uint32_t m_ls_mode;
		uint32_t m_ls_pattern;
		uint32_t m_ls_pattern_saved;
		uint32_t m_z_pattern;
		uint32_t m_color_back;
		uint32_t m_color_vram;
		uint32_t m_alpha_ref;
		uint32_t m_setup;
		uint32_t m_step_z;
		int32_t m_x_start;
		int32_t m_y_start;
		int32_t m_x_end;
		int32_t m_y_end;
		int16_t m_x_start_frac;
		int16_t m_y_start_frac;
		int16_t m_x_end_frac;
		int16_t m_y_end_frac;
		int16_t m_x_save;
		uint32_t m_xy_move;
		int16_t m_x_move;
		int16_t m_y_move;
		uint32_t m_bres_d;
		uint32_t m_bres_s1;
		uint32_t m_bres_octant_inc1;
		uint32_t m_bres_round_inc2;
		uint32_t m_bres_e1;
		uint32_t m_bres_s2;
		uint32_t m_a_weight0;
		uint32_t m_a_weight1;
		uint32_t m_x_start_f;
		uint32_t m_y_start_f;
		uint32_t m_x_end_f;
		uint32_t m_y_end_f;
		int16_t m_x_start_i;
		uint32_t m_xy_start_i;
		int16_t m_y_start_i;
		uint32_t m_xy_end_i;
		int16_t m_x_end_i;
		int16_t m_y_end_i;
		uint32_t m_x_start_end_i;
		uint32_t m_color_red;
		uint32_t m_color_alpha;
		uint32_t m_color_green;
		uint32_t m_color_blue;
		uint32_t m_curr_color_red;
		uint32_t m_curr_color_alpha;
		uint32_t m_curr_color_green;
		uint32_t m_curr_color_blue;
		int32_t m_slope_red;
		int32_t m_slope_alpha;
		int32_t m_slope_green;
		int32_t m_slope_blue;
		uint32_t m_write_mask;
		uint32_t m_color_i;
		uint32_t m_zero_overflow;
		uint64_t m_host_dataport;
		uint32_t m_dcb_mode;
		uint32_t m_dcb_reg_select;
		uint32_t m_dcb_slave_select;
		uint32_t m_dcb_data_msw;
		uint32_t m_dcb_data_lsw;
		uint32_t m_smask_x[5];
		uint32_t m_smask_y[5];
		uint32_t m_top_scanline;
		uint32_t m_xy_window;
		int16_t m_x_window;
		int16_t m_y_window;
		uint32_t m_clip_mode;
		uint32_t m_config;
		uint32_t m_status;
		uint32_t m_dcb_mask;
	};

	void ramdac_write(uint32_t data);

	void write_x_start(int32_t val);
	void write_y_start(int32_t val);
	void write_x_end(int32_t val);
	void write_y_end(int32_t val);

	bool pixel_clip_pass(int16_t x, int16_t y);
	void write_pixel(uint32_t color);
	void write_pixel(int16_t x, int16_t y, uint32_t color);
	void blend_pixel(uint32_t *dest_buf, uint32_t src);
	void logic_pixel(uint32_t *dest_buf, uint32_t src);
	void store_pixel(uint32_t *dest_buf, uint32_t src);

	void iterate_shade();

	uint32_t get_host_color();
	uint32_t get_rgb_color(int16_t x, int16_t y);

	uint32_t convert_4bpp_bgr_to_8bpp(uint8_t pix_in);
	uint32_t convert_4bpp_bgr_to_12bpp(uint8_t pix_in);
	uint32_t convert_4bpp_bgr_to_24bpp(uint8_t pix_in);
	uint32_t convert_8bpp_bgr_to_4bpp(uint8_t pix_in);
	uint32_t convert_8bpp_bgr_to_12bpp(uint8_t pix_in);
	uint32_t convert_8bpp_bgr_to_24bpp(uint8_t pix_in);
	uint32_t convert_12bpp_bgr_to_4bpp(uint16_t pix_in);
	uint32_t convert_12bpp_bgr_to_8bpp(uint16_t pix_in);
	uint32_t convert_12bpp_bgr_to_24bpp(uint16_t pix_in);
	uint32_t convert_24bpp_bgr_to_4bpp(uint32_t pix_in);
	uint32_t convert_24bpp_bgr_to_8bpp(uint32_t pix_in);
	uint32_t convert_24bpp_bgr_to_12bpp(uint32_t pix_in);

	uint32_t convert_4bpp_bgr_to_24bpp_rgb(uint8_t pix_in);
	uint32_t convert_8bpp_bgr_to_24bpp_rgb(uint8_t pix_in);
	uint32_t convert_12bpp_bgr_to_24bpp_rgb(uint16_t pix_in);

	uint32_t do_endian_swap(uint32_t color);

	struct bresenham_octant_info_t
	{
		int16_t incrx1;
		int16_t incrx2;
		int16_t incry1;
		int16_t incry2;
		uint8_t loop;
	};
	uint8_t get_octant(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy);
	void do_fline(uint32_t color);
	void do_iline(uint32_t color);

	uint32_t do_pixel_read();
	uint64_t do_pixel_word_read();

	void do_rex3_command();

	void ramdac_remap(uint32_t *dest);

	required_device<screen_device> m_screen;
	required_device_array<xmap9_device, 2> m_xmap;
	required_device_array<cmap_device, 2> m_cmap;
	required_device<vc2_device> m_vc2;

	uint32_t m_ramdac_lut_r[256];
	uint32_t m_ramdac_lut_g[256];
	uint32_t m_ramdac_lut_b[256];
	uint8_t m_ramdac_lut_index;

	rex3_t m_rex3;
	std::unique_ptr<uint32_t[]> m_rgbci;
	std::unique_ptr<uint32_t[]> m_olay;
	std::unique_ptr<uint32_t[]> m_pup;
	std::unique_ptr<uint32_t[]> m_cid;
	uint32_t m_global_mask;
	emu_timer *m_dcb_timeout_timer;

#if ENABLE_NEWVIEW_LOG
	void start_logging();
	void stop_logging();

	FILE *m_newview_log;
#endif

	static const uint32_t s_host_shifts[4];
};

class gio64_xl8_device : public newport_base_device
{
public:
	gio64_xl8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class gio64_xl24_device : public newport_base_device
{
public:
	gio64_xl24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

DECLARE_DEVICE_TYPE(GIO64_XL8,  gio64_xl8_device)
DECLARE_DEVICE_TYPE(GIO64_XL24, gio64_xl24_device)

#endif // MAME_BUS_GIO_NEWPORT_H
