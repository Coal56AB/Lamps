#ifndef NOTES_H
#define NOTES_H
#include "stm32f1xx_hal.h"

typedef struct {
    float freq;          // частота в Гц или NOTE_REST
    float duration;      // длительность в долях (0.25 = четверть)
} Note_t;

// Один шаг: [0] = PA8, [1] = PB0.
typedef Note_t MelodyStep_t[2];

typedef struct {
    uint16_t length;            // количество шагов мелодии
    const MelodyStep_t *sequence;
    uint8_t track_volume[2];    // громкость дорожек [0..10]
} Melody_t;


// Длительности (в долях от целой ноты)
#define NOTE_WHOLE      1.0
#define NOTE_HALF       0.5
#define NOTE_QUARTER    0.25
#define NOTE_EIGHTH     0.125
#define NOTE_SIXTEENTH  0.0625
#define NOTE_THIRTYSECOND 0.03125
#define NOTE_QUARTER_TRIPLET (1.0f / 6.0f)
#define NOTE_EIGHTH_TRIPLET  (1.0f / 12.0f)

#define NOTE_WHOLE_DOT      1.5
#define NOTE_HALF_DOT       0.75
#define NOTE_QUARTER_DOT    0.375
#define NOTE_EIGHTH_DOT     0.1875
#define NOTE_SIXTEENTH_DOT  0.09375

// Ноты (частота, Гц) - диапазон 700-8000 Гц для SCS-17-S
// Октавы смещены: старая 4-я = новая 0-я, старая 5-я = новая 1-я и т.д.

// 00-я октава (бывшая 3-я)
#define NOTE_B00 (249.19f)
#define NOTE_C0  (264.01f)
#define NOTE_CS0 (279.70f)
#define NOTE_D0  (296.33f)
#define NOTE_DS0 (313.96f)
#define NOTE_E0  (332.62f)
#define NOTE_F0  (352.40f)
#define NOTE_FS0 (373.36f)
#define NOTE_G0  (395.56f)
#define NOTE_GS0 (419.08f)
#define NOTE_A0  (440.00f)
#define NOTE_AS0 (470.40f)
#define NOTE_B0  (498.37f)

// 0-я октава (бывшая 4-я)
#define NOTE_C1  (NOTE_C0*2)
#define NOTE_CS1 (NOTE_CS0*2)
#define NOTE_D1  (NOTE_D0*2)
#define NOTE_DS1 (NOTE_DS0*2)
#define NOTE_E1  (NOTE_E0*2)
#define NOTE_F1  (NOTE_F0*2)
#define NOTE_FS1 (NOTE_FS0*2)
#define NOTE_G1  (NOTE_G0*2)
#define NOTE_GS1 (NOTE_GS0*2)
#define NOTE_A1  (NOTE_A0*2)
#define NOTE_AS1 (NOTE_AS0*2)
#define NOTE_B1  (NOTE_B0*2)

// 2-я октава (бывшая 6-я)
#define NOTE_C2  (NOTE_C1*2)
#define NOTE_CS2 (NOTE_CS1*2)
#define NOTE_D2  (NOTE_D1*2)
#define NOTE_DS2 (NOTE_DS1*2)
#define NOTE_E2  (NOTE_E1*2)
#define NOTE_F2  (NOTE_F1*2)
#define NOTE_FS2 (NOTE_FS1*2)
#define NOTE_G2  (NOTE_G1*2)
#define NOTE_GS2 (NOTE_GS1*2)
#define NOTE_A2  (NOTE_A1*2)
#define NOTE_AS2 (NOTE_AS1*2)
#define NOTE_B2  (NOTE_B1*2)

// 3-я октава (бывшая 7-я)
#define NOTE_C3  (NOTE_C2*2)
#define NOTE_CS3 (NOTE_CS2*2)
#define NOTE_D3  (NOTE_D2*2)
#define NOTE_DS3 (NOTE_DS2*2)
#define NOTE_E3  (NOTE_E2*2)
#define NOTE_F3  (NOTE_F2*2)
#define NOTE_FS3 (NOTE_FS2*2)
#define NOTE_G3  (NOTE_G2*2)
#define NOTE_GS3 (NOTE_GS2*2)
#define NOTE_A3  (NOTE_A2*2)
#define NOTE_AS3 (NOTE_AS2*2)
#define NOTE_B3  (NOTE_B2*2)

// 4-я октава (бывшая 8-я)
#define NOTE_C4  (NOTE_C3*2)
#define NOTE_CS4 (NOTE_CS3*2)
#define NOTE_D4  (NOTE_D3*2)
#define NOTE_DS4 (NOTE_DS3*2)
#define NOTE_E4  (NOTE_E3*2)
#define NOTE_F4  (NOTE_F3*2)
#define NOTE_FS4 (NOTE_FS3*2)
#define NOTE_G4  (NOTE_G3*2)
#define NOTE_GS4 (NOTE_GS3*2)
#define NOTE_A4  (NOTE_A3*2)
#define NOTE_AS4 (NOTE_AS3*2)
#define NOTE_B4  (NOTE_B3*2)

// Сольфеджио с новыми октавами
// 0-я октава
#define SI00  NOTE_B00   // 249.19 Hz

// 0-я октава
#define DO0  NOTE_C0   // 264.01 Hz
#define RE0  NOTE_D0   // 296.33 Hz
#define MI0  NOTE_E0   // 332.62 Hz
#define FA0  NOTE_F0   // 352.40 Hz
#define SOL0 NOTE_G0   // 395.56 Hz
#define LA0  NOTE_A0   // 440.00 Hz
#define SI0  NOTE_B0   // 498.37 Hz

// 1-я октава
#define DO1  NOTE_C1   // 528.02 Hz
#define RE1  NOTE_D1   // 592.66 Hz
#define MI1  NOTE_E1   // 665.24 Hz
#define FA1  NOTE_F1   // 704.80 Hz
#define SOL1 NOTE_G1   // 791.12 Hz
#define LA1  NOTE_A1   // 880.00 Hz
#define SI1  NOTE_B1   // 996.74 Hz

// 2-я октава
#define DO2  NOTE_C2   // 1056.04 Hz
#define RE2  NOTE_D2   // 1185.32 Hz
#define MI2  NOTE_E2   // 1330.48 Hz
#define FA2  NOTE_F2   // 1409.60 Hz
#define SOL2 NOTE_G2   // 1582.24 Hz
#define LA2  NOTE_A2   // 1760.00 Hz
#define SI2  NOTE_B2   // 1993.48 Hz

// 3-я октава
#define DO3  NOTE_C3   // 2112.08 Hz
#define RE3  NOTE_D3   // 2370.64 Hz
#define MI3  NOTE_E3   // 2660.96 Hz
#define FA3  NOTE_F3   // 2819.20 Hz
#define SOL3 NOTE_G3   // 3164.48 Hz
#define LA3  NOTE_A3   // 3520.00 Hz
#define SI3  NOTE_B3   // 3986.96 Hz

// 4-я октава
#define DO4  NOTE_C4   // 4224.16 Hz
#define RE4  NOTE_D4   // 4741.28 Hz
#define MI4  NOTE_E4   // 5321.92 Hz
#define FA4  NOTE_F4   // 5638.40 Hz
#define SOL4 NOTE_G4   // 6328.96 Hz
#define LA4  NOTE_A4   // 7040.00 Hz
#define SI4  NOTE_B4   // 7973.92 Hz

// Диезы для 0-й октавы
#define DO0s NOTE_CS0  // 559.40 Hz
#define RE0s NOTE_DS0  // 627.92 Hz
#define FA0s NOTE_FS0  // 746.72 Hz
#define SOL0s NOTE_GS0 // 838.16 Hz
#define LA0s NOTE_AS0  // 940.80 Hz

// Диезы для 1-й октавы
#define DO1s NOTE_CS1  // 559.40 Hz
#define RE1s NOTE_DS1  // 627.92 Hz
#define FA1s NOTE_FS1  // 746.72 Hz
#define SOL1s NOTE_GS1 // 838.16 Hz
#define LA1s NOTE_AS1  // 940.80 Hz

// Диезы для 2-й октавы
#define DO2s NOTE_CS2  // 1118.80 Hz
#define RE2s NOTE_DS2  // 1255.84 Hz
#define FA2s NOTE_FS2  // 1493.44 Hz
#define SOL2s NOTE_GS2 // 1676.32 Hz
#define LA2s NOTE_AS2  // 1881.60 Hz

// Диезы для 3-й октавы
#define DO3s NOTE_CS3  // 2237.60 Hz
#define RE3s NOTE_DS3  // 2511.68 Hz
#define FA3s NOTE_FS3  // 2986.88 Hz
#define SOL3s NOTE_GS3 // 3352.64 Hz
#define LA3s NOTE_AS3  // 3763.20 Hz

// Диезы для 4-й октавы
#define DO4s NOTE_CS4  // 4475.20 Hz
#define RE4s NOTE_DS4  // 5023.36 Hz
#define FA4s NOTE_FS4  // 5973.76 Hz
#define SOL4s NOTE_GS4 // 6705.28 Hz
#define LA4s NOTE_AS4  // 7526.40 Hz

#define NOTE_REST 0.0f


#endif
