/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// d_scan.c
//
// Portable C scan-level rasterization code, all pixel depths.

#include "r_local.h"
#include "r_dither.h"

//qb:  static vars for several functions
//static int          count, spancount;
//static byte         *pbase, *pdest;
//static fixed16_t    s, t, snext, tnext, sstep, tstep;
//static float        sdivz, tdivz, zi, z, du, dv, spancountminus1;
//static float        sdivzstepu, tdivzstepu, zistepu;
//static int		    izi, izistep; // mankrip
//static short		*pz; // mankrip

//static byte	*r_turb_pbase, *r_turb_pdest;
//static fixed16_t		r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
//static int				r_turb_spancount;

int				*r_turb_turb;

#if	!id386

#define d_srcbpp 8
#define d_dstbpp 8
#include "r_scaninc.h"

#define d_srcbpp 8
#define d_dstbpp 32
#include "r_scaninc.h"

#define d_srcbpp 32
#define d_dstbpp 32
#include "r_scaninc.h"

#endif	// !id386

/*
=============
D_WarpScreen

this performs a slight compression of the screen at the same time as
the sine warp, to keep the edges from wrapping
=============
*/
void D_WarpScreen(void)
{
	if (0) { // 8bpp
		D_WarpScreen_8to8();
	}
	else {
		D_WarpScreen_32to32();
	}
}


#if	!id386

void D_DrawSpans16(const spanrast_t* sr)
{
	if (!sr->vwrite) {
		return;
	}

	if (sr->sfill) {
		if (sr->bpp == 8)
			D_FlatFillSurface_8to32(sr, sr->color);
		else
			D_FlatFillSurface_32to32(sr, sr->color);
		return;
	}

	 if (sr->warp) {
		if (sr->bpp == 8)
			Turbulent8_8to32(sr);
		else
			Turbulent8_32to32(sr);
	}
	else if (sr->turb) {
		// textures that aren't warping are just flowing. Use NonTurbulent8 instead
		if (sr->bpp == 8)
			NonTurbulent8_8to32(sr);
		else
			NonTurbulent8_32to32(sr);
	}
	else {
		if (sr->bpp == 8)
			D_DrawSpans16_8to32(sr);
		else
			D_DrawSpans16_32to32(sr);
	}
}

#endif


#if	!id386

/*
=============
D_DrawZSpans
=============
*/
void D_DrawZSpans(const spanrast_t* sr)
{
	espan_t* pspan;
	short			*pdest;
	unsigned		ltemp;
	int          count, spancount;
	float zi, du, dv;
	int		    izi, izistep; // mankrip

	if (!sr->zwrite) {
		return;
	}

	pspan = sr->span;
	izistep = sr->izistepu;

	pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

	count = pspan->count;

	// calculate the initial 1/z
	du = (float)pspan->u;
	dv = (float)pspan->v;

	zi = sr->d_ziorigin + dv * sr->d_zistepv + du * sr->d_zistepu;
	// we count on FP exceptions being turned off to avoid range problems
	izi = (int)(zi * 0x8000 * 0x10000);

	if ((long)pdest & 0x02)
	{
		*pdest++ = (short)(izi >> 16);
		izi += izistep;
		count--;
	}

	if ((spancount = count >> 1) > 0)
	{
		do
		{
			ltemp = izi >> 16;
			izi += izistep;
			ltemp |= izi & 0xFFFF0000;
			izi += izistep;
			*(int*)pdest = ltemp;
			pdest += 2;
		} while (--spancount > 0);
	}

	if (count & 1)
		*pdest = (short)(izi >> 16);
}

#endif

