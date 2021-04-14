// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

**************************************************************************/
#ifndef MAME_INCLUDES_GAELCO3D_H
#define MAME_INCLUDES_GAELCO3D_H

#pragma once

#include "cpu/adsp2100/adsp2100.h"
#include "cpu/tms32031/tms32031.h"
#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/gaelco3d.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/dmadac.h"
#include "video/poly.h"
#include "screen.h"

#define SOUND_CHANNELS  4


class gaelco3d_state : public driver_device
{
	static constexpr int MAX_POLYGONS = 4096;
	static constexpr int MAX_POLYDATA = MAX_POLYGONS * 21;
	static constexpr int MAX_VERTICES = 32;

public:
	gaelco3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_adsp_ram_base(*this, "adsp_ram_base")
		, m_m68k_ram_base16(*this, "m68k_ram_base16")
		, m_m68k_ram_base32(*this, "m68k_ram_base32")
		, m_adsp_control_regs(*this, "adsp_regs")
		, m_adsp_fastram_base(*this, "adsp_fastram")
		, m_maincpu(*this, "maincpu")
		, m_adsp(*this, "adsp")
		, m_eeprom(*this, "eeprom")
		, m_tms(*this, "tms")
		, m_dmadac(*this, "dac%u", 0U)
		, m_serial(*this, "serial")
		, m_screen(*this, "screen")
		, m_soundlatch(*this, "soundlatch")
		, m_mainlatch(*this, "mainlatch")
		, m_outlatch(*this, "outlatch")
		, m_adsp_autobuffer_timer(*this, "adsp_timer")
		, m_paletteram16(*this, "paletteram16")
		, m_paletteram32(*this, "paletteram32")
		, m_analog(*this, "ANALOG%u", 0U)
		, m_adsp_bank(*this, "adspbank")
	{ }

	void footbpow(machine_config &config);
	void gaelco3d2(machine_config &config);
	void gaelco3d(machine_config &config);

	template <int N> DECLARE_READ_LINE_MEMBER(analog_bit_r);
	template <int N> DECLARE_READ_LINE_MEMBER(fp_analog_bit_r);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void device_register_save(save_registrar &save) override
	{
		// machine items
		save.reg(NAME(m_sound_status))
			.reg(NAME(m_analog_ports))
			.reg(NAME(m_framenum))
			.reg(NAME(m_adsp_ireg))
			.reg(NAME(m_adsp_ireg_base))
			.reg(NAME(m_adsp_incs))
			.reg(NAME(m_adsp_size))
			.reg(NAME(m_fp_clock))
			.reg(NAME(m_fp_state))
			.reg(NAME(m_fp_analog_ports))
			.reg(NAME(m_fp_length))

			// video items
			.reg(NAME(m_palette), 32768)
			.reg(NAME(m_polydata_buffer), MAX_POLYDATA)
			.reg(NAME(m_polydata_count))
			.reg(NAME(m_lastscan))
			.reg(NAME(m_poly));
	}

	struct gaelco3d_object_data
	{
		uint32_t tex, color;
		float ooz_dx, ooz_dy, ooz_base;
		float uoz_dx, uoz_dy, uoz_base;
		float voz_dx, voz_dy, voz_base;
		float z0;
	};

	class gaelco3d_renderer : public poly_manager<float, gaelco3d_object_data, 1, 2000>
	{
	public:
		gaelco3d_renderer(gaelco3d_state &state);
		~gaelco3d_renderer() {}

		bitmap_ind16 &screenbits() { return m_screenbits; }
		uint32_t polygons() { uint32_t result = m_polygons; m_polygons = 0; return result; }

		void render_poly(screen_device &screen, uint32_t *polydata);

		void register_save(save_registrar &save) { save.reg(NAME(m_screenbits)).reg(NAME(m_zbuffer)); }

	protected:
		gaelco3d_state &m_state;

	private:
		bitmap_ind16 m_screenbits;
		bitmap_ind16 m_zbuffer;
		uint32_t m_polygons;
		offs_t m_texture_size;
		offs_t m_texmask_size;
		std::unique_ptr<uint8_t[]> m_texture;
		std::unique_ptr<uint8_t[]> m_texmask;

		void render_noz_noperspective(int32_t scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
		void render_normal(int32_t scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
		void render_alphablend(int32_t scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	};

	required_shared_ptr<uint32_t> m_adsp_ram_base;
	optional_shared_ptr<uint16_t> m_m68k_ram_base16;
	optional_shared_ptr<uint32_t> m_m68k_ram_base32;
	required_shared_ptr<uint16_t> m_adsp_control_regs;
	required_shared_ptr<uint16_t> m_adsp_fastram_base;
	required_device<cpu_device> m_maincpu;
	required_device<adsp21xx_device> m_adsp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<tms32031_device> m_tms;
	required_device_array<dmadac_sound_device, SOUND_CHANNELS> m_dmadac;
	required_device<gaelco_serial_device> m_serial;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<ls259_device> m_mainlatch;
	required_device<ls259_device> m_outlatch;
	required_device<timer_device> m_adsp_autobuffer_timer;

	optional_shared_ptr<uint16_t> m_paletteram16;
	optional_shared_ptr<uint32_t> m_paletteram32;
	optional_ioport_array<4> m_analog;
	required_memory_bank m_adsp_bank;

	uint8_t m_sound_status;
	uint8_t m_analog_ports[4];
	uint32_t m_fp_analog_ports[2];
	uint32_t m_fp_length[2];
	uint8_t m_fp_clock;
	uint8_t m_fp_state;
	uint8_t m_framenum;
	uint8_t m_adsp_ireg;
	offs_t m_adsp_ireg_base;
	offs_t m_adsp_incs;
	offs_t m_adsp_size;
	std::unique_ptr<rgb_t[]> m_palette;
	std::unique_ptr<uint32_t[]> m_polydata_buffer;
	uint32_t m_polydata_count;
	int m_lastscan;
	int m_video_changed;
	std::unique_ptr<gaelco3d_renderer> m_poly;

	void irq_ack_w(uint16_t data);
	uint16_t sound_status_r(offs_t offset, uint16_t mem_mask = ~0);
	void sound_status_w(uint16_t data);
	DECLARE_WRITE_LINE_MEMBER(analog_port_clock_w);
	DECLARE_WRITE_LINE_MEMBER(analog_port_latch_w);
	uint32_t tms_m68k_ram_r(offs_t offset);
	void tms_m68k_ram_w(offs_t offset, uint32_t data);
	void tms_iack_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(tms_reset_w);
	DECLARE_WRITE_LINE_MEMBER(tms_irq_w);
	DECLARE_WRITE_LINE_MEMBER(tms_control3_w);
	void adsp_control_w(offs_t offset, uint16_t data);
	void adsp_rombank_w(offs_t offset, uint16_t data);
	DECLARE_WRITE_LINE_MEMBER(radikalb_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(unknown_137_w);
	DECLARE_WRITE_LINE_MEMBER(unknown_13a_w);
	void gaelco3d_render_w(uint32_t data);
	void gaelco3d_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gaelco3d_paletteram_020_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(ser_irq);
	uint16_t eeprom_data_r(offs_t offset, uint16_t mem_mask = ~0);

	DECLARE_MACHINE_RESET(gaelco3d2);
	DECLARE_MACHINE_RESET(common);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_gen);
	TIMER_DEVICE_CALLBACK_MEMBER(adsp_autobuffer_irq);
	void gaelco3d_render(screen_device &screen);
	void adsp_tx_callback(offs_t offset, uint32_t data);
	DECLARE_WRITE_LINE_MEMBER(fp_analog_clock_w);

	void adsp_data_map(address_map &map);
	void adsp_program_map(address_map &map);
	void main020_map(address_map &map);
	void main_map(address_map &map);
	void tms_map(address_map &map);
};

#endif // MAME_INCLUDES_GAELCO3D_H
