/**
 * @filename ugfont.c
 * @brief a component for UVGUI, you can also only use this lib to display glyph in your device.
 * @author LXG.firestaradmin
 * @version V1.0.0
 * @email 791836949@qq.com
 */
#include "ugfont.h"


/*******************
 * static variable.
*******************/
// static __ugfont_fill_bitmap_t fill_bitmap_p;

/**********************
 *  CONST TABLE
 **********************/
 /* This is for phrasing font which different bpps. */
const uint8_t _ug_bpp1_opa_table[2]  = {0, 255};          /*Opacity mapping with bpp = 1 (Just for compatibility)*/
const uint8_t _ug_bpp2_opa_table[4]  = {0, 85, 170, 255}; /*Opacity mapping with bpp = 2*/

const uint8_t _ug_bpp3_opa_table[8]  = {0, 36,  73, 109,   /*Opacity mapping with bpp = 3*/
                                        146, 182,  219, 255
                                       };

const uint8_t _ug_bpp4_opa_table[16] = {0,  17, 34,  51,  /*Opacity mapping with bpp = 4*/
                                        68, 85, 102, 119,
                                        136, 153, 170, 187,
                                        204, 221, 238, 255
                                       };
const uint8_t _ug_bpp8_opa_table[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                         16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                                         32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                                         48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
                                         64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                                         80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
                                         96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
                                         112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
                                         128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
                                         144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
                                         160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
                                         176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
                                         192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
                                         208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
                                         224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
                                         240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
                                        };

										
/*******************
 * static func.
*******************/
// static void draw_letter_normal(ug_coord_t pos_x, ug_coord_t pos_y, ug_font_glyph_dsc_t * g, lcd_color_t color, lcd_color_t back_color, const uint8_t * map_p);
static void draw_letter_normal(LcdDevice *dev, ug_coord_t pos_x, ug_coord_t pos_y, ug_font_glyph_dsc_t *g, lcd_color_t color, lcd_color_t back_color, const uint8_t *map_p);
static unsigned int unicode_to_utf8(unsigned char *buf, unsigned int unicode);
static unsigned int utf8_to_unicode(unsigned int *unicode, const unsigned char *word);



void ug_draw_str(LcdDevice* dev, uint16_t x,uint16_t y, const ug_font_t * font_p, lcd_color_t color, lcd_color_t backcolor, const unsigned char* str)
// void ug_draw_str(const ug_point_t * pos_p, const ug_font_t * font_p, lcd_color_t color, lcd_color_t backcolor, const unsigned char* str)
{
	if(str == NULL) return;
	
	uint32_t bytes_num = 0;
	uint32_t unicode = 0;
	uint16_t letter_adv_w = 0;
	/*character's coordinates*/
	ug_point_t pos = {
		.x = x,
		.y = y
	};
	
	ug_font_glyph_dsc_t g;
	bool g_ret;
	
	while(*str != '\0'){
		
		if(*str == '\t'){	/*if character is tab*/
			bytes_num = 1;
			g_ret = font_p->get_glyph_dsc(font_p, &g, ' ', '\0');
			if(g_ret == false) {
				str += bytes_num;
				letter_adv_w = 0;
				bytes_num = 0;
				continue;
			}
			letter_adv_w = g.adv_w * UG_TAB_SIZE; 
		}
		else if(*str == '\r' || *str == '\n'){	/*change line*/
			bytes_num = 1;
			y +=  font_p->line_height;
			pos.x = x;
			
		}
		else{
			bytes_num = utf8_to_unicode(&unicode, str);
			g_ret = font_p->get_glyph_dsc(font_p, &g, unicode, '\0');
			if(g_ret == false) {
				str += bytes_num;
				letter_adv_w = 0;
				bytes_num = 0;
				continue;
			}
			letter_adv_w = g.adv_w; 
			
			ug_draw_letter(dev, pos.x, pos.y, font_p, color, backcolor, unicode);
		}
		
		pos.x += letter_adv_w;
		str += bytes_num;
		letter_adv_w = 0;
		bytes_num = 0;

	}
}

void ug_draw_letter(LcdDevice* dev, uint16_t x,uint16_t y, const ug_font_t * font_p, lcd_color_t color, lcd_color_t backcolor, uint32_t letter)
// void ug_draw_letter(const ug_point_t * pos_p, const ug_font_t * font_p, lcd_color_t color, lcd_color_t backcolor, uint32_t letter)
{

    if(font_p == NULL) {
        return;
    }
	uint32_t unicode = letter;
    ug_font_glyph_dsc_t g;
    bool g_ret = font_p->get_glyph_dsc(font_p, &g, letter, '\0');
    if(g_ret == false)  {
        /* Add waring if the dsc is not found
         * but do not print warning for non printable ASCII chars (e.g. '\n')*/
        if(letter >= 0x20) {
            UG_LOG_WARN("ug_draw_letter: glyph dsc. not found");
        }
        return;
    }

    /* Don't draw anything if the character is empty. E.g. space */
    if((g.box_h == 0) && (g.box_w == 0)) return;

	/*pos_xy :字模在屏幕上的(左上角)坐标（去除空白区域的字模）*/
    ug_coord_t pos_x = x + g.ofs_x;
    ug_coord_t pos_y = y + (font_p->line_height - font_p->base_line) - g.box_h - g.ofs_y;

	/*get glyph bitmap*/
    const uint8_t * map_p = font_p->get_glyph_bitmap(font_p, letter);
    if(map_p == NULL) {
        UG_LOG_WARN("ug_draw_letter: character's bitmap not found");
        return;
    }

	draw_letter_normal(dev, pos_x, pos_y, &g, color, backcolor, map_p);

}



static void draw_letter_normal(LcdDevice* dev, ug_coord_t pos_x, ug_coord_t pos_y, ug_font_glyph_dsc_t * g, lcd_color_t color, lcd_color_t back_color, const uint8_t * map_p)
{
#ifdef USE_UVGUI_LIB
	//TODO: malloc mem of glyph_rect dynamically.
#else
	static uint16_t glyph_rect_buf[1000];
#endif
	
	const uint8_t * bpp_opa_table_p = _ug_bpp1_opa_table;
	uint16_t box_h = g->box_h;
	uint16_t box_w = g->box_w;
	
	
	uint32_t glyph_px_cnt = box_h * box_w;	/*pixels cnt of glyph*/
	uint8_t bpp = g->bpp;
	
	
	//lcd_color_t fill_blank = {.full = 0xFFFF};	//white

	uint16_t col = 0 ,row = 0;
	uint16_t bit = 0;
	uint8_t bitmask = 0x00;
	switch(bpp){
		case 1:
			bpp_opa_table_p = _ug_bpp1_opa_table;
			bitmask = 0x01;
			break;
		case 4:
			bpp_opa_table_p = _ug_bpp4_opa_table;
			bitmask = 0x0F;
			break;
		case 2:
			bpp_opa_table_p = _ug_bpp2_opa_table;
			bitmask = 0x03;
			break;
		case 8:
			bpp_opa_table_p = _ug_bpp8_opa_table;
			bitmask = 0xFF;
			break;
		default:
			UG_LOG_ERR("no supporting to this bpp!");
			bpp_opa_table_p = _ug_bpp1_opa_table;
            return;
            break;
    }
	
	uint32_t px_cnt = 0;
	uint8_t byte = 0;
    uint16_t _fillcolor = 0;
    for (uint32_t i = 0; i < glyph_px_cnt; i++)
    {
        byte = (*map_p >> ((8-bpp) - bit)) & bitmask;
        // _fillcolor = UG_COLOR_RED.full;
        // _fillcolor = color.full;
        _fillcolor = ug_color_lighten(color, 255 - bpp_opa_table_p[byte]).full;
        glyph_rect_buf[i] = byte > 0 ? _fillcolor : back_color.full;
        // glyph_rect_buf[i] = (bpp_opa_table_p[byte]) ? ug_color_lighten(color, 255 - bpp_opa_table_p[byte]).full : back_color.full;
		bit += bpp;
		px_cnt++;
		if(bit == 8) {
			bit = 0;
			map_p ++;
		}
		else if(px_cnt == box_w){
			bit = 0;
			px_cnt = 0;
			map_p ++;
		}
    }

#ifdef USE_UVGUI_LIB

#else
	
    if(dev->vram != NULL)
    {
        lcd_ram_fill_bitmap(dev, pos_x, pos_y, box_w, box_h, glyph_rect_buf);
    }
    else{
        lcd_draw_sw_rgb565_swap(glyph_rect_buf, box_w * box_h);
        lcd_xfer_buf(dev, pos_x, pos_y, box_w, box_h, glyph_rect_buf, box_w * box_h * 2);
    }

#endif
}


/**
 * @brief  Converter one charcater UTF-8 to Unicode.
 * UTF-8编码规则：如果只有一个字节则其最高二进制位为0；
 * 如果是多字节，其第一个字节从最高位开始，连续的二进制位值为1的个数决定了其编码的字节数，其余各字节均以10开头。
 * @param word the character to be converted
 * @return the character bytes_num in UTF-8.
*/
static unsigned int utf8_to_unicode(unsigned int *unicode, const unsigned char *word)
{
    if(word == NULL)
        return 0;
    const unsigned char *pbyte = word;
    unsigned char byte ;
    int byte_num = 1 , i = 0;
    unsigned int res_unicode = 0;

    byte = *pbyte;

    if(!(byte & 0x80)){ /*0000 ~ 007F*/
        res_unicode = byte & 0x7F;
    }
    else {  /*007F ~ 7FFF FFFF*/
        byte <<= 1;
        while(byte & 0x80)
            byte_num ++, byte <<= 1;

        res_unicode += *pbyte & (0xFF >> (byte_num + 1));
        for(i = 1; i < byte_num; i++){
            res_unicode <<= 6;
            res_unicode += pbyte[i] & 0x3F;
        }
    }

    *unicode = res_unicode;
	return byte_num;
}


/**
 * @brief  Converter Unicode to UTF8.
 * UTF-8编码规则：如果只有一个字节则其最高二进制位为0；
 * 如果是多字节，其第一个字节从最高位开始，连续的二进制位值为1的个数决定了其编码的字节数，其余各字节均以10开头。
 * @param buf where store the UTF-8 data.
 * @param unicode which to be converted.
 * @return the bytes of the character in UTF-8 need.
 * 			[unicode range]         [bytes]
            0000 ~ 007F             1
            0080 ~ 07FF             2
            0800 ~ FFFF             3
            1 0000 ~ 1F FFFF        4
            20 0000 ~ 3FF FFFF      5
            400 0000 ~ 7FFF FFFF    6
*/
static unsigned int unicode_to_utf8(unsigned char *buf, unsigned int unicode)
{
    if(unicode == 0)
        return 0;
    unsigned int unicode_temp;
    int byte_num = 0, i;
    if(unicode <= 0x7F){
        *buf = unicode;
        return 1;
    }
    else{
        if(unicode < 0x7FF) byte_num = 2;
        else if(unicode < 0xFFFF) byte_num = 3;
        else if(unicode < 0x1FFFFF) byte_num = 4;
        else if(unicode < 0x3FFFFF) byte_num = 5;
        else byte_num = 6;

        for(i = byte_num-1; i > 0; i--){
            buf[i] = 0x80 + (unicode & 0x3F);
            unicode >>= 6;
        }
        buf[i] = (0xFF << (8 - byte_num)) + unicode ;

        return byte_num;
    }

}

#ifdef USE_UVGUI_LIB

#else
// void ug_font_register_fill_bitmap_func(__ugfont_fill_bitmap_t func)
// {
// 	fill_bitmap_p = func;
// }

lcd_color_t ug_color_lighten(lcd_color_t c, ug_opa_t lvl)
{
    return ug_color_mix(UG_COLOR_WHITE, c, lvl);
}

lcd_color_t ug_color_lighten_with_backcolor(lcd_color_t c, lcd_color_t backcolor, ug_opa_t lvl)
{
    return ug_color_mix(backcolor, c, lvl);
}


lcd_color_t ug_color_darken(lcd_color_t c, ug_opa_t lvl)
{
    return ug_color_mix(UG_COLOR_BLACK, c, lvl);
}

/**
 * @brief return a color changed brightness.
 * @param lvl range in 0~255. 0 is most dark.
*/
inline lcd_color_t ug_color_set_brightness(lcd_color_t c, ug_opa_t lvl)
{
	uint8_t brightness = lvl;
	return UG_COLOR_MAKE((uint32_t)UG_COLOR_GET_R(c) * (uint32_t)brightness / 255, (uint32_t)UG_COLOR_GET_G(c) * (uint32_t)brightness / 255, (uint32_t)UG_COLOR_GET_B(c) * (uint32_t)brightness / 255);

}
#endif


