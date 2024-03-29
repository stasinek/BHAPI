/*
	Copyright (c) 2003, Thomas Kurschel


	Part of Radeon driver
		
	TV-Out registers
*/

#ifndef _TV_OUT_REGS_H
#define _TV_OUT_REGS_H

#define	RADEON_TV_MASTER_CNTL								0x0800
#		define RADEON_TV_MASTER_CNTL_TV_ASYNC_RST			(1 << 0)
#		define RADEON_TV_MASTER_CNTL_CRT_ASYNC_RST			(1 << 1)
#		define RADEON_TV_MASTER_CNTL_RESTART_PHASE_FIX		(1 << 3)
#		define RADEON_TV_MASTER_CNTL_TV_FIFO_ASYNC_RST		(1 << 4)
#		define RADEON_TV_MASTER_CNTL_VIN_ASYNC_RST			(1 << 5)
#		define RADEON_TV_MASTER_CNTL_AUD_ASYNC_RST			(1 << 6)
#		define RADEON_TV_MASTER_CNTL_DVS_ASYNC_RST			(1 << 7)
#		define RADEON_TV_MASTER_CNTL_CRT_FIFO_CE_EN			(1 << 9)
#		define RADEON_TV_MASTER_CNTL_TV_FIFO_CE_EN			(1 << 10)
#		define RADEON_TV_MASTER_CNTL_RE_SYNC_NOW_SEL_MASK	(3 << 14)
#		define RADEON_TV_MASTER_CNTL_TVCLK_ALWAYS_ONb		(1 << 30)
#		define RADEON_TV_MASTER_CNTL_TV_ON					(1 << 31)

#define	RADEON_TV_RGB_CNTL									0x0804
#		define RADEON_TV_RGB_CNTL_RGB_SRC_SEL_SHIFT			8
#		define RADEON_TV_RGB_CNTL_RGB_DITHER_EN				(1 << 5)
#		define RADEON_TV_RGB_CNTL_UVRAM_READ_MARGIN_SHIFT	16
#		define RADEON_TV_RGB_CNTL_FIFORAM_FIFOMACRO_READ_MARGIN_SHIFT 20

#define RADEON_TV_HTOTAL									0x080c
#define RADEON_TV_HDISP										0x0810
#define RADEON_TV_HSTART									0x0818
#define RADEON_TV_VTOTAL									0x0820
#define RADEON_TV_VDISP										0x0824
#define RADEON_TV_FTOTAL									0x082c
#define RADEON_TV_FRESTART									0x0834
#define RADEON_TV_HRESTART									0x0838
#define RADEON_TV_VRESTART									0x083c

#define RADEON_TV_HOST_READ_DATA							0x0840
#define RADEON_TV_HOST_WRITE_DATA							0x0844
#define RADEON_TV_HOST_RD_WT_CNTL							0x0848
#		define RADEON_TV_HOST_RD_WT_CNTL_RD					(1 << 12)
#		define RADEON_TV_HOST_RD_WT_CNTL_RD_ACK				(1 << 13)
#		define RADEON_TV_HOST_RD_WT_CNTL_WT					(1 << 14)
#		define RADEON_TV_HOST_RD_WT_CNTL_WT_ACK				(1 << 15)


#define	RADEON_TV_VSCALER_CNTL1								0x084c
#		define RADEON_TV_VSCALER_CNTL1_UV_INC_SHIFT			0
#		define RADEON_TV_VSCALER_CNTL1_UV_INC_MASK			0x0000ffff
#		define RADEON_TV_VSCALER_CNTL1_UV_THINNER_SHIFT		16
#		define RADEON_TV_VSCALER_CNTL1_UV_THINNER_MASK		0x003f0000
#		define RADEON_TV_VSCALER_CNTL1_Y_W_EN				(1 << 24)
#		define RADEON_TV_VSCALER_CNTL1_Y_DEL_W_SIG_SHIFT	26
#		define RADEON_TV_VSCALER_CNTL1_RESTART_FIELD		(1 << 29)


#define	RADEON_TV_TIMING_CNTL								0x0850
#		define RADEON_TV_TIMING_CNTL_UV_OUTPUT_POST_SCALE_SHIFT	24

#define	RADEON_TV_VSCALER_CNTL2								0x0854
#		define RADEON_TV_VSCALER_CNTL2_DITHER_MODE			(1 << 0)
#		define RADEON_TV_VSCALER_CNTL2_Y_OUTPUT_DITHER_EN	(1 << 1)
#		define RADEON_TV_VSCALER_CNTL2_UV_OUTPUT_DITHER_EN	(1 << 2)
#		define RADEON_TV_VSCALER_CNTL2_UV_TO_BUF_DITHER_EN	(1 << 3)
#		define RADEON_TV_VSCALER_CNTL2_UV_ACCUM_INIT_SHIFT	24

#define	RADEON_TV_Y_FALL_CNTL								0x0858
#		define RADEON_TV_Y_FALL_CNTL_Y_FALL_PING_PONG		(1 << 16)
#		define RADEON_TV_Y_FALL_CNTL_Y_COEFF_EN				(1 << 17)
#		define RADEON_TV_Y_FALL_CNTL_Y_COEFF_VALUE_SHIFT	24
		
#define	RADEON_TV_Y_RISE_CNTL								0x085c
#		define RADEON_TV_Y_RISE_CNTL_Y_RISE_PING_PONG		(1 << 16)

#define	RADEON_TV_Y_SAW_TOOTH_CNTL							0x0860
#		define RADEON_TV_Y_SAW_TOOTH_CNTL_SLOPE_SHIFT		16

#define	RADEON_TV_UPSAMP_AND_GAIN_CNTL						0x0864
#define	RADEON_TV_GAIN_LIMIT_SETTINGS						0x0868
#define	RADEON_TV_LINEAR_GAIN_SETTINGS						0x086c

#define	RADEON_TV_MODULATOR_CNTL1							0x0870
#		define RADEON_TV_MODULATOR_CNTL1_YFLT_EN			(1 << 2)
#		define RADEON_TV_MODULATOR_CNTL1_UVFLT_EN			(1 << 3)
#		define RADEON_TV_MODULATOR_CNTL1_ALT_PHASE_EN		(1 << 6)
#		define RADEON_TV_MODULATOR_CNTL1_SYNC_TIP_LEVEL		(1 << 7)
#		define RADEON_TV_MODULATOR_CNTL1_SET_UP_LEVEL_SHIFT	8
#		define RADEON_TV_MODULATOR_CNTL1_SET_UP_LEVEL_MASK	0x00007f00
#		define RADEON_TV_MODULATOR_CNTL1_BLANK_LEVEL_SHIFT	16
#		define RADEON_TV_MODULATOR_CNTL1_BLANK_LEVEL_MASK	0x007f0000
#		define RADEON_TV_MODULATOR_CNTL1_SLEW_RATE_LIMIT	(1 << 23)
#		define RADEON_TV_MODULATOR_CNTL1_CY_FILT_BLEND_SHIFT 28

#define	RADEON_TV_MODULATOR_CNTL2							0x0874
#		define TV_MODULATOR_CNTL2_U_BURST_LEVEL_MASK		0x1ff
#		define TV_MODULATOR_CNTL2_V_BURST_LEVEL_MASK		0x1ff
#		define TV_MODULATOR_CNTL2_V_BURST_LEVEL_SHIFT		16

#define	RADEON_TV_PRE_DAC_MUX_CNTL							0x0888
#		define RADEON_TV_PRE_DAC_MUX_CNTL_Y_RED_EN			(1 << 0)
#		define RADEON_TV_PRE_DAC_MUX_CNTL_C_GRN_EN			(1 << 1)
#		define RADEON_TV_PRE_DAC_MUX_CNTL_CMP_BLU_EN		(1 << 2)
#		define RADEON_TV_PRE_DAC_MUX_CNTL_DAC_DITHER_EN		(1 << 3)
#		define RADEON_TV_PRE_DAC_MUX_CNTL_RED_MX_SHIFT		4
#		define RADEON_TV_PRE_DAC_MUX_CNTL_GRN_MX_SHIFT		8
#		define RADEON_TV_PRE_DAC_MUX_CNTL_BLU_MX_SHIFT		12
#		define RADEON_TV_MUX_FORCE_DAC_DATA					6
#		define RADEON_TV_PRE_DAC_MUX_CNTL_FORCE_DAC_DATA_SHIFT	16

#define	RADEON_TV_DAC_CNTL									0x088c
#		define RADEON_TV_DAC_CNTL_NBLANK					(1 << 0)
#		define RADEON_TV_DAC_CNTL_NHOLD						(1 << 1)
#		define RADEON_TV_DAC_CNTL_PEDESTAL					(1 << 2)
#		define RADEON_TV_DAC_CNTL_DASLEEP					(1 << 3) // Theatre only
#		define RADEON_TV_DAC_CNTL_DETECT					(1 << 4)
#		define RADEON_TV_DAC_CNTL_CMPOUT					(1 << 5)
#		define RADEON_TV_DAC_CNTL_BGSLEEP					(1 << 6)
#		define RADEON_TV_DAC_CNTL_STD_PAL					(0 << 8)
#		define RADEON_TV_DAC_CNTL_STD_NTSC					(1 << 8)
#		define RADEON_TV_DAC_CNTL_STD_PS2					(2 << 8)
#		define RADEON_TV_DAC_CNTL_STD_RS343					(3 << 8)
#		define RADEON_TV_DAC_CNTL_BGADJ_SHIFT				16
#		define RADEON_TV_DAC_CNTL_DACADJ_SHIFT				20
#		define RADEON_TV_DAC_CNTL_RDACPD					(1 << 24)
#		define RADEON_TV_DAC_CNTL_GDACPD					(1 << 25)
#		define RADEON_TV_DAC_CNTL_BDACPD					(1 << 26)
#		define RADEON_TV_DAC_CNTL_RDACDET					(1 << 29)
#		define RADEON_TV_DAC_CNTL_GDACDET					(1 << 30)
#		define RADEON_TV_DAC_CNTL_BDACDET					(1 << 31)

#define	RADEON_TV_CRC_CNTL									0x0890

#define	RADEON_TV_UV_ADR									0x08ac
#		define RADEON_TV_UV_ADR_MAX_UV_ADR_MASK				0x000000ff
#		define RADEON_TV_UV_ADR_MAX_UV_ADR_SHIFT			0
#		define RADEON_TV_UV_ADR_TABLE1_BOT_ADR_MASK			0x0000ff00
#		define RADEON_TV_UV_ADR_TABLE1_BOT_ADR_SHIFT		8
#		define RADEON_TV_UV_ADR_TABLE3_TOP_ADR_MASK			0x00ff0000
#		define RADEON_TV_UV_ADR_TABLE3_TOP_ADR_SHIFT		16
#		define RADEON_TV_UV_ADR_HCODE_TABLE_SEL_MASK		0x06000000
#		define RADEON_TV_UV_ADR_HCODE_TABLE_SEL_SHIFT		25
#		define RADEON_TV_UV_ADR_VCODE_TABLE_SEL_MASK		0x18000000
#		define RADEON_TV_UV_ADR_VCODE_TABLE_SEL_SHIFT		27
#define RADEON_TV_MAX_FIFO_ADDR								0x1a7
#define RADEON_TV_MAX_FIFO_ADDR_INTERN						0x1ff


#define	RADEON_TV_PLL_FINE_CNTL								0x20

#define	RADEON_TV_PLL_CNTL									0x21
#		define RADEON_TV_PLL_CNTL_TV_M0_LO_MASK				0xff
#		define RADEON_TV_PLL_CNTL_TV_N0_LO_MASK				0x1ff
#		define RADEON_TV_PLL_CNTL_TV_N0_LO_SHIFT			8
#		define RADEON_TV_PLL_CNTL_TV_M0_LO_BITS				8
#		define RADEON_TV_PLL_CNTL_TV_M0_HI_SHIFT			18
#		define RADEON_TV_PLL_CNTL_TV_N0_LO_BITS				9
#		define RADEON_TV_PLL_CNTL_TV_N0_HI_SHIFT			21
#		define RADEON_TV_PLL_CNTL_TV_SLIP_EN				(1 << 23)
#		define RADEON_TV_PLL_CNTL_TV_P_SHIFT				24
#		define RADEON_TV_PLL_CNTL_TV_DTO_EN					(1 << 28)

#		define RADEON_TV_CRT_PLL_CNTL_M0_LO_MASK			0xff
#		define RADEON_TV_CRT_PLL_CNTL_N0_LO_MASK			0x1ff
#		define RADEON_TV_CRT_PLL_CNTL_N0_LO_SHIFT			8
#		define RADEON_TV_CRT_PLL_CNTL_M0_LO_BITS			8
#		define RADEON_TV_CRT_PLL_CNTL_M0_HI_SHIFT			18
#		define RADEON_TV_CRT_PLL_CNTL_N0_LO_BITS			9
#		define RADEON_TV_CRT_PLL_CNTL_N0_HI_SHIFT			21
#		define RADEON_TV_CRT_PLL_CNTL_CLKBY2				(1 << 25)


#define	RADEON_TV_PLL_CNTL1									0x22
#		define RADEON_TV_PLL_CNTL1_TVPCP_SHIFT				8
#		define RADEON_TV_PLL_CNTL1_TVPVG_SHIFT				11
#		define RADEON_TV_PLL_CNTL1_TVPDC_SHIFT				14
#		define RADEON_TV_PLL_CNTL1_TVCLK_SRC_SEL_CPUCLK		(0 << 30)
#		define RADEON_TV_PLL_CNTL1_TVCLK_SRC_SEL_TVPLLCLK	(1 << 30)
#		define RADEON_TV_PLL_CNTL1_TVPLL_TEST				(1 << 31)


#		define RADEON_TV_CLOCK_SEL_CNTL_BYTCLK_SHIFT		2
#		define RADEON_TV_CLOCK_SEL_CNTL_BYTCLKD_SHIFT		8

#endif
