/* 
	definitions for used nVidia acceleration engine commands.

	Written by Rudolf Cornelissen 12/2004-12/2005
*/

#ifndef NV_ACC_H
#define NV_ACC_H

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t SetRop5;				/* b0-7 is ROP5 */
} cmd_nv_rop5_solid;

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t TopLeft;				/* b0-15 is left, b16-31 is top */
	uint32_t HeightWidth;			/* b0-15 is width, b16-31 is height */
} cmd_nv_image_black_rectangle;

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t SetColorFormat;		/* colorspace */
	uint32_t reserved03[0x0001];
	uint32_t SetShape;			/* b0-1: %00 = 8X_8Y; %01 = 64X_1Y; %10 = 1X_64Y */
	uint32_t reserved04[0x0001];
	uint32_t SetColor0;			/* b0-31 is color */
	uint32_t SetColor1;			/* b0-31 is color */
	uint32_t SetPattern[0x0002];	/* b0-31 is bitmap */
} cmd_nv_image_pattern;

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t SourceOrg;			/* b0-15 is X, b16-31 is Y */
	uint32_t DestOrg;				/* b0-15 is X, b16-31 is Y */
	uint32_t HeightWidth;			/* b0-15 is width, b16-31 is height */
} cmd_nv_image_blit;

//fixme: using nv4_gdi_rectangle_text for DMA acc. Differs slightly from this one!
//WARNING: nv4_gdi_rectangle_text can only do 32 unclipped rects at once!
typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t SetColorFormat;		/* colorspace */
	uint32_t reserved03[0x003e];
	uint32_t Color1A;				/* b0-31 is color */
	struct {
		uint32_t LeftTop;			/* b0-15 is top, b16-31 is left */
		uint32_t WidthHeight;		/* b0-15 is height, b16-31 is width */
	} UnclippedRectangle[0x40];	/* command can handle upto 64 unclipped rects */
/* fixme: XFree also defines the registers below:
 * (used for the 2D 'ScanlineCPUToScreenColorExpandFill' and 'ColorExpandScanline'
 * functions.)
 * We don't use this currently. */
/*
    U032 reserved04[(0x080)-3];
    struct
    {
        U032 TopLeft;
        U032 BottomRight;
    } ClipB;
    U032 Color1B;
    struct
    {
        U032 TopLeft;
        U032 BottomRight;
    } ClippedRectangle[64];
    U032 reserved05[(0x080)-5];
    struct
    {
        U032 TopLeft;
        U032 BottomRight;
    } ClipC;
    U032 Color1C;
    U032 WidthHeightC;
    U032 PointC;
    U032 MonochromeData1C;
    U032 reserved06[(0x080)+121];
    struct
    {
        U032 TopLeft;
        U032 BottomRight;
    } ClipD;
    U032 Color1D;
    U032 WidthHeightInD;
    U032 WidthHeightOutD;
    U032 PointD;
    U032 MonochromeData1D;
    U032 reserved07[(0x080)+120];
    struct
    {
        U032 TopLeft;
        U032 BottomRight;
    } ClipE;
    U032 Color0E;
    U032 Color1E;
    U032 WidthHeightInE;
    U032 WidthHeightOutE;
    U032 PointE;
    U032 MonochromeData01E;
*/
} cmd_nv3_gdi_rectangle_text;

/* This command could (at least) potentially speed-up our fill_span command... :-) */
//fixme: test this 2D command!
typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00af];
	uint32_t Color;				/* b0-31 is color */
	uint32_t reserved03[0x003e];
	struct {
		uint32_t Point0;			/* b0-15 is X, b16-31 is Y: starting coordinate */
		uint32_t Point1;			/* b0-15 is X, b16-31 is Y: ending coordinate */
	} Line[0x10];				/* command can handle upto 16 lines */
	struct {
		uint32_t Point0X;			/* b0-31 is X: starting coordinate */
		uint32_t Point0Y;			/* b0-31 is Y: starting coordinate */
		uint32_t Point1X;			/* b0-31 is X: ending coordinate */
		uint32_t Point1Y;			/* b0-31 is Y: ending coordinate */
	} Line32[0x08];				/* cmd can handle upto 8 lines with 32-bit coordinates */
	struct {
		uint32_t Point;			/* b0-15 is X, b16-31 is Y */
	} Polyline[0x20];			/* cmd can handle upto 32 points polylines */
	struct {
		uint32_t PointX;			/* b0-31 is X */
		uint32_t PointY;			/* b0-31 is Y */
	} Polyline32[0x10];			/* cmd can handle upto 16 point polylines with 32-bit coord's */
	struct {
		uint32_t Color;			/* b0-31 is color */
		uint32_t Point;			/* b0-15 is X, b16-31 is Y */
	} ColorPolyline[0x10];		/* cmd can handle upto 16 point polylines with individually
								 * colored sections */
} cmd_nv1_render_solid_lin;

/* Someone defined this in XFree once (as 'RivaRectangle') but never used it:
 * a handle should also be defined in the engine's init code, which was never done AFAIK.
 * The command could be very interesting for us, as it could potentially speed-up our
 * rectangle_fills :-) */
//fixme: test this 2D command!
typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00af];
	uint32_t Color;				/* b0-31 is color */
	uint32_t reserved03[0x003e];
	struct {
		uint32_t TopLeft;			/* b0-15 is left, b16-31 is top */
		uint32_t HeightWidth;		/* b0-15 is width, b16-31 is height */
	} Rectangle[0x10];			/* command can handle upto 16 rectangles */
} cmd_nv_render_solid_rectangle;/* nv1_render_solid_rectangle is identical */

//fixme: (setup in progress for DMA) complete or remove (this cmd is not used currently)
typedef struct {
} cmd_nv1_image_from_cpu;		/* 'pixmap': used in XFree 4.2.0, but not beyond.
								 * (Used for the 2D 'ImageWriteScanline' and
								 *  'ScanlineImageWriteRect' functions.)
								 * Is this command actually usefull? */

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t Format;				/* buffer colorspace */
	uint32_t Pitch;				/* b0-15 is source pitch, b16-31 is dest pitch */
	uint32_t OffsetSource;		/* b0-31 is source bufferadress offset */
	uint32_t OffsetDest;			/* b0-31 is dest bufferadress offset */
} cmd_nv4_surface;				/* nv10_context_surfaces_2d is identical as far as used */

/************************
 * 3D specific commands *
 ************************/

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00ae];
	uint32_t Colorkey;			/* texture colorkey */
	uint32_t Offset;				/* texture offset */
	uint32_t Format;				/* texture colorspace, size, and a lot more */
	uint32_t Filter;				/* texture filtering modes (used for scaling) */
	uint32_t Blend;				/* triangle blend: shade, perspective, specular.. */
	uint32_t Control;				/* triangle control: Z-enable, culling, dither.. */	
	uint32_t FogColor;			/* fog colorvalue */
	uint32_t reserved03[0x0039];
	struct {
		float ScreenX;			/* X */
		float ScreenY;			/* Y */
		float ScreenZ;			/* depth */
		float RWH;				/* eyeM */
		uint32_t Color;			/* b24-31 Alpha, b16-23 Red, b8-15 Green, b0-7 Blue */
		uint32_t Specular;		/* b24-31 Fog, b16-23 Red, b8-15 Green, b0-7 Blue */
		float TU;				/* texture S */
		float TV;				/* texture T */
	} TLVertex[0x10];			/* command can handle upto 16 textured, lit(?) vertexes */
	uint32_t TLVDrawPrim[0x40];	/* b20-31 is I5, b16-19 is I4, b12-15 is I3,
								 * b8-11 is I2, 4-7 is I1, b0-3 is I0:
								 * Ix is a TLVertex[Ix].
								 * So: define your (single) texture, define your
								 * vertexes, and then program TLVDrawPrim with the
								 * order to draw them.
								 * You can draw primitives consisting of sets of upto
								 * 6 out of 16 defined vertexes this way; and you can
								 * draw 64 sets maximum. */
} cmd_nv4_dx5_texture_triangle;	/* nv10_dx5_texture_triangle is identical */

typedef struct {
	uint32_t reserved00[0x0004];
	uint16 FifoFree;			/* little endian (FIFO internal register) */
	uint16 Nop;					/* little endian (FIFO internal register) */
	uint32_t reserved01[0x000b];
	uint32_t DMAPut;				/* b2-28 is DMA Put offset (FIFO internal register) */
	uint32_t DMAGet;				/* b2-28 is DMA Get offset (FIFO internal register) */
	uint32_t reserved02[0x00b0];	/* fixme? there's more here that's not used apparantly */
	uint32_t Pitch;				/* b16-31 is Z-buffer, b0-15 is colorbuffer pitch */
	uint32_t SetOffsetColor;		/* b0-31 is colorbuffer (renderbuffer) offset */
	uint32_t SetOffsetZeta;		/* b0-31 is Z-buffer (zeta buffer) offset */
} cmd_nv4_context_surfaces_argb_zs; /* nv10_context_surfaces_argb_zs is identical */

//fixme: fill this out...
typedef struct {
} cmd_nv4_dx6_multi_texture_triangle;/* nv10_dx6_multi_texture_triangle is identical? */


/************ DMA command defines ***********/

/* FIFO channels */
#define NV_GENERAL_FIFO_CH0		0x0000
#define NV_GENERAL_FIFO_CH1		0x2000
#define NV_GENERAL_FIFO_CH2		0x4000
#define NV_GENERAL_FIFO_CH3		0x6000
#define NV_GENERAL_FIFO_CH4		0x8000
#define NV_GENERAL_FIFO_CH5		0xa000
#define NV_GENERAL_FIFO_CH6		0xc000
#define NV_GENERAL_FIFO_CH7		0xe000

/* sub-command offsets within FIFO channels */
#define NV_GENERAL_DMAPUT							0x0040
#define NV_GENERAL_DMAGET							0x0044
#define NV_ROP5_SOLID_SETROP5						0x0300
#define NV_IMAGE_BLACK_RECTANGLE_TOPLEFT			0x0300
#define NV_IMAGE_PATTERN_SETCOLORFORMAT				0x0300
#define NV_IMAGE_PATTERN_SETSHAPE					0x0308
#define NV_IMAGE_PATTERN_SETCOLOR0					0x0310
#define NV_IMAGE_BLIT_SOURCEORG						0x0300
//fixme note: non-DMA acc is still using NV3_GDI_RECTANGLE_TEXT...
//which is just as fast as NV4_GDI_RECTANGLE_TEXT, but has a hardware fault for DMA!
#define NV4_GDI_RECTANGLE_TEXT_SETCOLORFORMAT		0x0300
#define NV4_GDI_RECTANGLE_TEXT_COLOR1A				0x03fc
#define NV4_GDI_RECTANGLE_TEXT_UCR0_LEFTTOP			0x0400
#define NV4_SURFACE_FORMAT							0x0300
#define NV_SCALED_IMAGE_FROM_MEMORY_SETCOLORFORMAT	0x0300
#define NV_SCALED_IMAGE_FROM_MEMORY_SOURCEORG		0x0308
#define NV_SCALED_IMAGE_FROM_MEMORY_SOURCESIZE		0x0400


/************************
 * 3D specific commands *
 ************************/

#define NV4_DX5_TEXTURE_TRIANGLE_COLORKEY			0x0300
#define NV4_DX5_TEXTURE_TRIANGLE_TLVERTEX(i)		0x0400 + (i << 5)
#define NV4_DX5_TEXTURE_TRIANGLE_TLVDRAWPRIM(i)		0x0600 + (i << 2)
#define NV4_CONTEXT_SURFACES_ARGB_ZS_PITCH			0x0308

#endif
