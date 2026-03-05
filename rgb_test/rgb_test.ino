#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include <esp_cache.h>

#define SCREEN_WIDTH 800

#if SCREEN_WIDTH == 800
#define SCREEN_HEIGHT 600
// good for SAMSUNG 213T 800x600 @ 56Hz
#define VIDEO_LCD_PIXEL_CLOCK_HZ     (36000000)
#define VIDEO_CLK_SRC                LCD_CLK_SRC_APLL
#define VIDEO_LCD_H_RES              800
#define VIDEO_LCD_V_RES              600
#define VIDEO_LCD_HFP                32
#define VIDEO_LCD_HSYNC              96
#define VIDEO_LCD_HBP                96
#define VIDEO_LCD_VFP                1
#define VIDEO_LCD_VSYNC              4
#define VIDEO_LCD_VBP                19
#define VIDEO_LCD_HSP                true
#define VIDEO_LCD_VSP                false
#define LCD_NUM_FB             1
#endif 

#if SCREEN_WIDTH == 640
#define SCREEN_HEIGHT 480
// good one
#define VIDEO_LCD_PIXEL_CLOCK_HZ     (26666666)
#define VIDEO_CLK_SRC                LCD_CLK_SRC_PLL160M
#define VIDEO_LCD_H_RES              640
#define VIDEO_LCD_V_RES              480
#define VIDEO_LCD_HFP                24
#define VIDEO_LCD_HSYNC              40
#define VIDEO_LCD_HBP                128
#define VIDEO_LCD_VFP                11
#define VIDEO_LCD_VSYNC              3
#define VIDEO_LCD_VBP                25
#define VIDEO_LCD_HSP                false
#define VIDEO_LCD_VSP                false
#define LCD_NUM_FB                   1
#endif

#define PIN_NUM_HSYNC          27
#define PIN_NUM_VSYNC          23
#define PIN_NUM_DE             -1
#define PIN_NUM_PCLK           -1
// RED 7 to 3
#define PIN_NUM_DATA0          33
#define PIN_NUM_DATA1          26
#define PIN_NUM_DATA2          48
#define PIN_NUM_DATA3          47
#define PIN_NUM_DATA4          46
// GREEN 7 to 2
#define PIN_NUM_DATA5          1
#define PIN_NUM_DATA6          36
#define PIN_NUM_DATA7          32
#define PIN_NUM_DATA8          0
#define PIN_NUM_DATA9          2
#define PIN_NUM_DATA10         3
// BLUE 7 to 3
#define PIN_NUM_DATA11         22
#define PIN_NUM_DATA12         5
#define PIN_NUM_DATA13         4
#define PIN_NUM_DATA14         20
#define PIN_NUM_DATA15         21

#undef COL16
#define COL16

#ifdef COL16
#define VIDEO_DATA_BUS_WIDTH         16
#define VIDEO_PIXEL_SIZE             2
#define VIDEO_LCD_DATA_LINES         16
#undef VIDEO_LCD_DATA_LINES_24
#define VIDEO_LCD_DATA_LINES_16
#undef VIDEO_LCD_DATA_LINES_24
#else
#define VIDEO_DATA_BUS_WIDTH         8
#define VIDEO_PIXEL_SIZE             1
#define VIDEO_LCD_DATA_LINES 8
#define VIDEO_LCD_DATA_LINES_8
#undef VIDEO_LCD_DATA_LINES_16
#undef VIDEO_LCD_DATA_LINES_24
#endif

#include "esp_clk_tree.h"
#include "hal/clk_tree_ll.h"

uint8_t charmap[] = {
  0b00000000,
  0b01111100,
  0b10000110,
  0b10001010,
  0b10010010,
  0b10100010,
  0b11000010,
  0b01111100,
  0b00000000,
  0b00000000,

  0b00000000,
  0b00010000,
  0b00110000,
  0b01010000,
  0b10010000,
  0b00010000,
  0b00010000,
  0b11111110,
  0b00000000,
  0b00000000,

  0b00000000,
  0b01111100,
  0b10000010,
  0b00000100,
  0b00001000,
  0b00010000,
  0b00100000,
  0b11111110,
  0b00000000,
  0b00000000,

  0b00000000,
  0b01111100,
  0b10000010,
  0b00000010,
  0b00001100,
  0b00000010,
  0b10000010,
  0b01111100,
  0b00000000,
  0b00000000,

  0b00000000,
  0b10001000,
  0b10001000,
  0b10001000,
  0b11111110,
  0b00001000,
  0b00001000,
  0b00001000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b11111110,
  0b10000000,
  0b10000000,
  0b01111100,
  0b00000010,
  0b00000010,
  0b11111100,
  0b00000000,
  0b00000000,

  0b00000000,
  0b01111110,
  0b10000000,
  0b10000000,
  0b11111100,
  0b10000010,
  0b10000010,
  0b01111100,
  0b00000000,
  0b00000000,

  0b00000000,
  0b11111110,
  0b00000010,
  0b00000100,
  0b00001000,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00000000,
  0b00000000,

  0b00000000,
  0b01111100,
  0b10000010,
  0b10000010,
  0b01111100,
  0b10000010,
  0b10000010,
  0b01111100,
  0b00000000,
  0b00000000,

  0b00000000,
  0b01111100,
  0b10000010,
  0b10000010,
  0b01111110,
  0b00000010,
  0b00000010,
  0b11111100,
  0b00000000,
  0b00000000,
};

void *p4_frame_buffer = NULL;

bool videoInitWaveP4(void) {
    esp_err_t err;

    // RED5, 4 and 3 need LDO4 powered at 3.3V
    // be carefull, this LDO is also used by SDMMC at 3.3V or 1.8V -> can alter RED colors ...
    printf("Set LDO 4\n");
    esp_ldo_channel_handle_t ldo_vga_phy = NULL;
    esp_ldo_channel_config_t ldo_vga_phy_config = {
        .chan_id = 4,
        .voltage_mv = 3300,
        .flags = {
          .adjustable = 0,
        },
    };
    err = esp_ldo_acquire_channel(&ldo_vga_phy_config, &ldo_vga_phy);
    if (err == 0) {
      printf("LDO 4 VGA Powered on");
    } else {
      printf("LDO 4 VGA Powering failed");
    }

  
    printf("Install RGB LCD panel driver\n");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = VIDEO_CLK_SRC, 
        // LCD_CLK_SRC_APLL
        // LCD_CLK_SRC_DEFAULT 
        // LCD_CLK_SRC_PLL160M
        .timings = {
            .pclk_hz = VIDEO_LCD_PIXEL_CLOCK_HZ,
            .h_res = VIDEO_LCD_H_RES,
            .v_res = VIDEO_LCD_V_RES,
            .hsync_pulse_width = VIDEO_LCD_HSYNC,
            .hsync_back_porch = VIDEO_LCD_HBP,
            .hsync_front_porch = VIDEO_LCD_HFP,
            .vsync_pulse_width = VIDEO_LCD_VSYNC,
            .vsync_back_porch = VIDEO_LCD_VBP,
            .vsync_front_porch = VIDEO_LCD_VFP,
            .flags = {
                .hsync_idle_low = VIDEO_LCD_HSP,
                .vsync_idle_low = VIDEO_LCD_VSP,
                .de_idle_high = false,
                .pclk_active_neg = true,
                .pclk_idle_high = false,
            },
        },
        .data_width = VIDEO_DATA_BUS_WIDTH,
        .bits_per_pixel = 0,
        .num_fbs = LCD_NUM_FB,
        .bounce_buffer_size_px = 0,
        .dma_burst_size = 64,
        .hsync_gpio_num = PIN_NUM_HSYNC,
        .vsync_gpio_num = PIN_NUM_VSYNC,
        .de_gpio_num = PIN_NUM_DE,
        .pclk_gpio_num = PIN_NUM_PCLK,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
#ifdef COL16
            // high byte -> mapping for Quickdraw 15 bits depth framebuffer (see basilisk II emulator)
            PIN_NUM_DATA6,  // GREEN 6 bit 0
            PIN_NUM_DATA5,  // GREEN 7 bit 1
            
            PIN_NUM_DATA4,  // RED 3   bit 2
            PIN_NUM_DATA3,  // RED 4   bit 3
            PIN_NUM_DATA2,  // RED 5   bit 4
            PIN_NUM_DATA1,  // RED 6   bit 5
            PIN_NUM_DATA0,  // RED 7   bit 6
            -1,             // not used
            // low byte
            PIN_NUM_DATA15, // BLUE 3  bit 0
            PIN_NUM_DATA14, // BLUE 4  bit 1
            PIN_NUM_DATA13, // BLUE 5  bit 2
            PIN_NUM_DATA12, // BLUE 6  bit 3
            PIN_NUM_DATA11, // BLUE 7  bit 4
            
            PIN_NUM_DATA9,  // GREEN 3 bit 5
            PIN_NUM_DATA8,  // GREEN 4 bit 6
            PIN_NUM_DATA7,  // GREEN 5 bit 7
#else
            PIN_NUM_DATA12,   // BLUE6
            PIN_NUM_DATA11,   // BLUE7
            PIN_NUM_DATA7,    // GREEN5
            PIN_NUM_DATA6,    // GREEN6
            PIN_NUM_DATA5,    // GREEN7
            PIN_NUM_DATA2,    // RED5
            PIN_NUM_DATA1,    // RED6
            PIN_NUM_DATA0     // RED7
#endif
        },
        .flags = {
            .disp_active_low = false,
            .refresh_on_demand = false,
            .fb_in_psram = true, // allocate frame buffer in PSRAM
            .double_fb = false,
            .no_fb = false,
            .bb_invalidate_cache = true,
        },
    };

#if VIDEO_CLK_SRC == LCD_CLK_SRC_APLL

    uint32_t freq = 0;
    clk_ll_apll_enable();

    err = esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_APLL, ESP_CLK_TREE_SRC_FREQ_PRECISION_EXACT, &freq);
    printf("Get APLL freq %d Hz\n", freq);
    // apll_freq = xtal_freq * (4 + sdm2 + sdm1/256 + sdm0/65536)/((o_div + 2) * 2)
    // apll_freq = 40MHz * (4+4+0+0)/(0+2)*2 = 80 MHz
    uint32_t o_div;     // Frequency divider, 0..31
    uint32_t sdm0;      // Frequency adjustment parameter, 0..255
    uint32_t sdm1;      // Frequency adjustment parameter, 0..255
    uint32_t sdm2;      // Frequency adjustment parameter, 0..63
    clk_ll_apll_get_config(&o_div, &sdm0, &sdm1, &sdm2);
    printf("Got APLL config div: %d sdm0: %d sdm1: %d sdm2: %d\n", o_div, sdm0, sdm1, sdm2);

    o_div = 3;
    sdm0 = 0;
    sdm1 = 0;
    sdm2 = 14;
    printf("Set APLL config to div: %d sdm0: %d sdm1: %d sdm2: %d\n", o_div, sdm0, sdm1, sdm2);
    clk_ll_apll_set_config(o_div, sdm0, sdm1, sdm2);
    clk_ll_apll_get_config(&o_div, &sdm0, &sdm1, &sdm2);
    printf("Got APLL config div: %d sdm0: %d sdm1: %d sdm2: %d\n", o_div, sdm0, sdm1, sdm2);


    printf("APLL enabled\n");

    err = esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_APLL, ESP_CLK_TREE_SRC_FREQ_PRECISION_EXACT, &freq);
    printf("APLL freq now %d Hz\n", freq);
#endif

    printf("Create RGB LCD panel ");
    err = esp_lcd_new_rgb_panel(&panel_config, &panel_handle);
    printf("%d\n", err);

    printf("Reset RGB LCD panel ");
    err = esp_lcd_panel_reset(panel_handle);
    printf("%d\n", err);
    delay(100);
    printf("Init RGB LCD panel ");
    err = esp_lcd_panel_init(panel_handle);
    printf("%d\n", err);
    printf("Get FB RGB LCD panel ");
    err = esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 1, (void **)&p4_frame_buffer);
    printf("%d FB at %08X\n", err, p4_frame_buffer);
    printf("Initialize RGB LCD panel Done\n");
    
    memset(p4_frame_buffer, 0x00, VIDEO_LCD_H_RES*VIDEO_LCD_V_RES*VIDEO_PIXEL_SIZE);

    // sync caches on vsync to ensure good display by the DMA
    esp_cache_msync(p4_frame_buffer, (VIDEO_LCD_H_RES*VIDEO_LCD_V_RES*VIDEO_PIXEL_SIZE + 0x3F) & 0xFFFFC0, ESP_CACHE_MSYNC_FLAG_DIR_C2M);

    return true;
}

uint16_t rgb888_to_rgb565(uint16_t r, uint16_t g, uint16_t b) {
    return (g >> 6) | ((r >> 3) << 2) | (((b >> 3) | (g >> 3) << 5) << 8);        
}

void displayat(int x, int y, int v, uint16_t col0, uint16_t col1) {
  uint16_t *pos1;
  uint16_t *pos2;
  if (y < (SCREEN_HEIGHT/2)) {
    // logical start address to write -> display is shifted 8 pixels right
    pos1 = (uint16_t*)(p4_frame_buffer) + y*SCREEN_WIDTH + x;
  } else {
    // corrected start address to write, offset of 8 pixels ... -> display is correcly aligned
    pos1 = (uint16_t*)(p4_frame_buffer) - 8 + y*SCREEN_WIDTH + x;
  }
  pos2 = pos1 + SCREEN_WIDTH;
  uint8_t *car = charmap+v*10;
  for (int y = 0; y < 10; y++) {
    uint8_t d = *car++;
    for (int x = 0; x < 8; x++) {
      if (d & 0x80) {
        *pos1++ = col1; *pos2++ = col1;
      } else {
        *pos1++ = col0; *pos2++ = col0;
      }
      d = d << 1;
    }
    pos1 += 2*SCREEN_WIDTH - 8;
    pos2 += 2*SCREEN_WIDTH - 8;
  }
  esp_cache_msync(p4_frame_buffer, (VIDEO_LCD_H_RES*VIDEO_LCD_V_RES*VIDEO_PIXEL_SIZE + 0x3F) & 0xFFFFC0, ESP_CACHE_MSYNC_FLAG_DIR_C2M);
}

void setup() {
  videoInitWaveP4();

}

void loop() {
  int loop = 0;
  do {
    uint8_t c = 255;
    uint16_t color0 = 0;
    uint16_t color1 = 0xFFFF; // rgb888_to_rgb565(255, 255, 255);
    for (int y = 0; y < SCREEN_HEIGHT; y+=20) {
      switch(loop % 3) {
        case 0:
          color0 = rgb888_to_rgb565(c, 0, 0); break;
        case 1:
          color0 = rgb888_to_rgb565(0, c, 0); break;
        default:
          color0 = rgb888_to_rgb565(0, 0, c); break;
      }
      int v = 0;
      for (int x = 0; x < SCREEN_WIDTH; x+= 8 ) {
        displayat(x, y, v, color0, color1);
        v = (v+1) % 10;
      }
      c -= 8;
    }
    loop ++;
    esp_cache_msync(p4_frame_buffer, (VIDEO_LCD_H_RES*VIDEO_LCD_V_RES*VIDEO_PIXEL_SIZE + 0x3F) & 0xFFFFC0, ESP_CACHE_MSYNC_FLAG_DIR_C2M);
    delay(2000);
  } while (true);
}
