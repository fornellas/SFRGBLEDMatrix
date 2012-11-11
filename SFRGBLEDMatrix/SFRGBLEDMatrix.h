#ifndef SFRGBLEDMatrix_h
#define SFRGBLEDMatrix_h

#include <Arduino.h>

//
// Proper wiring and coordinates
//

/*

        //=============================\\
       //                               \\
      //  //=========================\\  \\
     //  //                           \\  \\
    //  //   +--> 0x0                  \\  \\
   //  //   /                           \\  \\
   ||  ||   +---+---+---+---+ ---> 23x0  ||  ||
   \\  \\==>| 8 | 9 | 10| 11|            //  //
    \\      +---+---+---+---+           //  //
     \\====>| 4 | 5 | 6 | 7 |>========//  //
            +---+---+---+---+             //
  Wires ==> | 0 | 1 | 2 | 3 |>===========//
            +---+---+---+---+ ---> 31x23

*/

//
// Colors
//

/*
  Data packing
  +-----------+-----------+
  | 0000 RRRR | GGGG BBBB |
  |   byte0   |   byte1   |
  +-----------+-----------+
*/

typedef uint16_t Color;

// Size constants
#define BITS_PER_COLOR 4
#define MAX_C 15
#define MID_C 7
#define DISP_LEN byte(8)

// Generate a Color, given RGB values
#define RGB(r,g,b) (((r)<<(BITS_PER_COLOR*2))|((g)<<BITS_PER_COLOR)|(b))

// Gamma correction
// 2.5 Gamma correction
static unsigned char gamma25[] PROGMEM = {0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 7, 9, 10, 13, 15};
#define RGB_GAMMA(r,g,b) (((pgm_read_byte(&gamma25[r]))<<(BITS_PER_COLOR*2))|((pgm_read_byte(&gamma25[g]))<<BITS_PER_COLOR)|(pgm_read_byte(&gamma25[b])))
#define V_GAMMA(v) pgm_read_byte(&gamma25[v])

// Extract color component from Color
#define GET_RED(c) (((c)&(0x0F<<(BITS_PER_COLOR*2)))>>(BITS_PER_COLOR*2))
#define GET_GREEN(c) (((c)&(0x0F<<BITS_PER_COLOR))>>BITS_PER_COLOR)
#define GET_BLUE(c) ((c)&0x0F)

// Some colors
#define	BLACK		RGB(0,	  0,    0    )
#define WHITE		RGB(MAX_C,MAX_C,MAX_C)

#define RED		RGB(MAX_C,0,    0    )
#define RED_MAGENTA	RGB(MAX_C,0,    MID_C)
#define RED_YELLOW	RGB(MAX_C,MID_C,0    )

#define GREEN		RGB(0,    MAX_C,0    )
#define GREEN_CYAN	RGB(0,    MAX_C,MID_C)
#define GREEN_YELLOW	RGB(MID_C,MAX_C,0    )

#define BLUE		RGB(0,    0,    MAX_C)
#define BLUE_CYAN	RGB(0,    MID_C,MAX_C)
#define BLUE_MAGENTA	RGB(MID_C,0,    MAX_C)

#define CYAN		RGB(0,    MAX_C,MAX_C)
#define MAGENTA		RGB(MAX_C,0,    MAX_C)
#define YELLOW		RGB(MAX_C,MAX_C,0    )

// Aliases
#define PINK RED_MAGENTA
#define ORANGE RED_YELLOW

//
// Class
//

class SFRGBLEDMatrix {
  private:
    // Slave selec pin
    uint8_t pinSS;
    // Number of displays
    uint8_t dispCount;
    // Frame buffer
    uint16_t frameBuffSize;
    uint8_t *frameBuff;
    // Helper function to draw pixel
    void paintColor(uint8_t x, uint8_t y, uint16_t colorOffset, uint8_t value);
    // Coordinate adjustments for paintPixel()
    int recAdjStart;
    int recAdjIncr;
    // Gamma
    boolean useGamma;
  public:
    // Useful variables
    uint8_t width;
    uint8_t height;
    uint16_t pixels;
    // Constructor / destructor
    SFRGBLEDMatrix(const uint8_t pinSS, const uint8_t numDispHoriz, const uint8_t numDispVert);
    ~SFRGBLEDMatrix();
    // Must be called before show() if SPI configuration was changed after SFRGBLEDMatrix()
    void setupSPI();
    // Must be called before show() if SPI / SS pins configuration were changed after SFRGBLEDMatrix()
    void setupPINs(); 
    // Send buffer to the screens
    void show(); 
    // Character drawing. Only size=4 or size=5 implemented for now
    void print(const Color color, const int x, const int y, const uint8_t size, const char c);
    void print(const Color color, const int x, const int y, const uint8_t size, const char *s);
    void print_PF(const Color color, const int x, const int y, const uint8_t size, PGM_P s);
    // paint single pixel
    void paintPixel(const Color color, const int x, const int y);
    // fill screen with one color
    void fill(const Color color);
    // same as fill(BLACK)
    void clear();
    // draw line
    void line(const Color color, const int x0, const int y0, const int x1, const int y1);
    // draw box
    void box(const Color color, const int x0, const int y0, const int x1, const int y1);
    // Enable / disable gamma correction
    void gamma(boolean state);
};

#endif
