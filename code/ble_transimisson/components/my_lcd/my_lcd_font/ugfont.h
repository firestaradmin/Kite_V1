/**
 * @file ugfont.h
 * @author @firestaradmin
 * @brief 
 * @version v0.0.1
 * @date 2024/05/14
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/05/14: v0.0.1 Initial Version
 *  - ----/--/--: --
 */
#ifndef __UGFONT_H__
#define __UGFONT_H__



#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "my_lcd_define.h"
#include "mylcd.h"



#define UG_FONT_DECLARE(font_name) extern struct _ug_font_struct font_name;


/*********************** User config ***********************/
/*if used in uvgui, enable this macro.*/
//#define USE_UVGUI_LIB

#define UG_TAB_SIZE 4

/*********************** User config END ***********************/


/* declare a custom font */
UG_FONT_DECLARE(ug_font_consolas_16px_15)





/**********************
 *      TYPEDEFS
 **********************/
#ifdef USE_UVGUI_LIB

#else
	#define UG_LOG_WARN(x)  
	#define UG_LOG_ERR(x)  
	
	
	
	typedef uint8_t ug_opa_t;	
	typedef  uint16_t ug_coord_t;
	/**
	 * Represents a point on the screen.
	 */
	typedef struct {
		ug_coord_t x;
		ug_coord_t y;
	} ug_point_t;
	
	
	
	// typedef void (*__ugfont_fill_bitmap_t)(ug_coord_t xs, ug_coord_t ys , ug_coord_t w, ug_coord_t h, const lcd_color_t* buf) ;
	
	
	// void ug_font_register_fill_bitmap_func(__ugfont_fill_bitmap_t func);
	
	
#endif /*END of USE_UVGUI_LIB*/


/*------------------
 * General types
 *-----------------*/

/** Describes the properties of a glyph. */
typedef struct {
    uint16_t adv_w; /**< The glyph needs this space. Draw the next glyph after this width. 8 bit integer, 4 bit fractional */
    uint16_t box_w;  /**< Width of the glyph's bounding box*/
    uint16_t box_h;  /**< Height of the glyph's bounding box*/
    int16_t ofs_x;   /**< x offset of the bounding box*/
    int16_t ofs_y;  /**< y offset of the bounding box*/
    uint8_t bpp;   /**< Bit-per-pixel: 1, 2, 4, 8*/
} ug_font_glyph_dsc_t;




/** Describe the properties of a font*/
typedef struct _ug_font_struct {
    /** Get a glyph's  descriptor from a font*/
    bool (*get_glyph_dsc)(const struct _ug_font_struct *, ug_font_glyph_dsc_t *, uint32_t letter, uint32_t letter_next);
    /** Get a glyph's bitmap from a font*/
    const uint8_t * (*get_glyph_bitmap)(const struct _ug_font_struct *, uint32_t);

    /*Pointer to the font in a font pack (must have the same line height)*/
    ug_coord_t line_height;         /**< The real line height where any text fits*/
    ug_coord_t base_line;           /**< Base line measured from the top of the line_height*/
   
    void * dsc;                     /**< Store implementation specific or run_time data or caching here*/

} ug_font_t;

/** This describes a glyph. */
typedef struct {
#if UG_FONT_FMT_TXT_LARGE == 0
    uint32_t bitmap_index : 20;     /**< Start index of the bitmap. A font can be max 1 MB. */
    uint32_t adv_w : 12;            /**< Draw the next glyph after this width. 8.4 format (real_value * 16 is stored). */
    uint8_t box_w;                  /**< Width of the glyph's bounding box*/
    uint8_t box_h;                  /**< Height of the glyph's bounding box*/
    int8_t ofs_x;                   /**< x offset of the bounding box*/
    int8_t ofs_y;                  /**< y offset of the bounding box. Measured from the top of the line*/
#else
    uint32_t bitmap_index;          /**< Start index of the bitmap. A font can be max 4 GB. */
    uint32_t adv_w;                 /**< Draw the next glyph after this width. 28.4 format (real_value * 16 is stored). */
    uint16_t box_w;                  /**< Width of the glyph's bounding box*/
    uint16_t box_h;                  /**< Height of the glyph's bounding box*/
    int16_t ofs_x;                   /**< x offset of the bounding box*/
    int16_t ofs_y;                  /**< y offset of the bounding box. Measured from the top of the line*/
#endif
} ug_font_fmt_txt_glyph_dsc_t;

/** Format of font character map. */
enum {
    UG_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    UG_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    UG_FONT_FMT_TXT_CMAP_SPARSE_TINY,
    UG_FONT_FMT_TXT_CMAP_SPARSE_FULL,
};


typedef uint8_t ug_font_fmt_txt_cmap_type_t;
/* Map codepoints to a `glyph_dsc`s
 * Several formats are supported to optimize memory usage
 */
typedef struct {
    /** First Unicode character for this range */
    uint32_t range_start;

    /** Number of Unicode characters related to this range.
     * Last Unicode character = range_start + range_length - 1*/
    uint16_t range_length;

    /** First glyph ID (array index of `glyph_dsc`) for this range */
    uint16_t glyph_id_start;


    const uint16_t * unicode_list;

    const void * glyph_id_ofs_list;

    /** Length of `unicode_list` and/or `glyph_id_ofs_list`*/
    uint16_t list_length;

    /** Type of this character map*/
    ug_font_fmt_txt_cmap_type_t type;
} ug_font_fmt_txt_cmap_t;

/*Describe store additional data for fonts */
typedef struct {
    /*The bitmaps of all glyphs*/
    const uint8_t * glyph_bitmap;

    /*Describe the glyphs*/
    const ug_font_fmt_txt_glyph_dsc_t * glyph_dsc;

    /* Map the glyphs to Unicode characters.
     * Array of `ug_font_cmap_fmt_txt_t` variables*/
    const ug_font_fmt_txt_cmap_t * cmaps;

    /* Store kerning values.
     * Can be  `ug_font_fmt_txt_kern_pair_t *  or `ug_font_kern_classes_fmt_txt_t *`
     * depending on `kern_classes`
     */
    const void * kern_dsc;

    /*Scale kern values in 12.4 format*/
    uint16_t kern_scale;

    /*Number of cmap tables*/
    uint16_t cmap_num       : 10;

    /*Bit per pixel: 1, 2, 3, 4, 8*/
    uint16_t bpp            : 4;

    /*Type of `kern_dsc`*/
    uint16_t kern_classes   : 1;

    /*
     * storage format of the bitmap
     * from `ug_font_fmt_txt_bitmap_format_t`
     */
    uint16_t bitmap_format  : 2;

    /*Cache the last letter and is glyph id*/
    uint32_t last_letter;
    uint32_t last_glyph_id;

} ug_font_fmt_txt_dsc_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/



/**
 * Get the line height of a font. All characters fit into this height
 * @param font_p pointer to a font
 * @return the height of a font
 */
static inline ug_coord_t ug_font_get_line_height(const ug_font_t * font_p)
{
    return font_p->line_height;
}






/**********************
 * GLOBAL FUNCTIONS
 **********************/
struct LcdDevice;
// /** draw a letter */
// void ug_draw_letter(const ug_point_t * pos_p, const ug_font_t * font_p, lcd_color_t color, lcd_color_t backcolor, uint32_t letter);
// /** draw a string */
// void ug_draw_str(const ug_point_t * pos_p, const ug_font_t * font_p, lcd_color_t color, lcd_color_t backcolor, const unsigned char* str);
void ug_draw_letter(LcdDevice *dev, uint16_t x, uint16_t y, const ug_font_t *font_p, lcd_color_t color, lcd_color_t backcolor, uint32_t letter);

void ug_draw_str(LcdDevice *dev, uint16_t x, uint16_t y, const ug_font_t *font_p, lcd_color_t color, lcd_color_t backcolor, const unsigned char *str);
#ifdef USE_UVGUI_LIB

#else
	// void ug_font_register_fill_bitmap_func(__ugfont_fill_bitmap_t func);
	
	
	lcd_color_t ug_color_lighten(lcd_color_t c, ug_opa_t lvl);
	lcd_color_t ug_color_darken(lcd_color_t c, ug_opa_t lvl);
	lcd_color_t ug_color_lighten_with_backcolor(lcd_color_t c, lcd_color_t backcolor, ug_opa_t lvl);
	
	inline lcd_color_t ug_color_set_brightness(lcd_color_t c, ug_opa_t lvl);
#endif /*END of USE_UVGUI_LIB*/









#ifdef __cplusplus
} /* extern "C" */
#endif





#endif
