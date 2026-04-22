#ifndef NOTES_H
#define NOTES_H
#include "stm32f1xx_hal.h"

typedef struct {
    uint16_t freq;       // частота в Гц или NOTE_REST
    float duration;      // длительность в долях (0.25 = четверть)
} Note_t;

typedef struct {
    float length;       // длительность мелодии в количестве нот
    const Note_t *sequence;   // последовательность нот
} Melody_t;


// Длительности (в долях от целой ноты)
#define NOTE_WHOLE      1.0
#define NOTE_HALF       0.5
#define NOTE_QUARTER    0.25
#define NOTE_EIGHTH     0.125
#define NOTE_SIXTEENTH  0.0625
#define NOTE_THIRTYSECOND 0.03125

#define NOTE_WHOLE_DOT      1.5
#define NOTE_HALF_DOT       0.75
#define NOTE_QUARTER_DOT    0.375
#define NOTE_EIGHTH_DOT     0.1875
#define NOTE_SIXTEENTH_DOT  0.09375

// Ноты (частота, Гц) - диапазон 700-8000 Гц для SCS-17-S
// Октавы смещены: старая 4-я = новая 0-я, старая 5-я = новая 1-я и т.д.

// 0-я октава (бывшая 4-я)
#define NOTE_B0  494
#define NOTE_C1  523
#define NOTE_CS1 554
#define NOTE_D1  587
#define NOTE_DS1 622
#define NOTE_E1  659
#define NOTE_F1  698
#define NOTE_FS1 740
#define NOTE_G1  784
#define NOTE_GS1 831
#define NOTE_A1  880
#define NOTE_AS1 932
#define NOTE_B1  988

// 2-я октава (бывшая 6-я)
#define NOTE_C2  1047
#define NOTE_CS2 1109
#define NOTE_D2  1175
#define NOTE_DS2 1245
#define NOTE_E2  1319
#define NOTE_F2  1397
#define NOTE_FS2 1480
#define NOTE_G2  1568
#define NOTE_GS2 1661
#define NOTE_A2  1760
#define NOTE_AS2 1865
#define NOTE_B2  1976

// 3-я октава (бывшая 7-я)
#define NOTE_C3  2093
#define NOTE_CS3 2217
#define NOTE_D3  2349
#define NOTE_DS3 2489
#define NOTE_E3  2637
#define NOTE_F3  2794
#define NOTE_FS3 2960
#define NOTE_G3  3136
#define NOTE_GS3 3322
#define NOTE_A3  3520
#define NOTE_AS3 3729
#define NOTE_B3  3951

// 4-я октава (бывшая 8-я)
#define NOTE_C4  4186
#define NOTE_CS4 4435
#define NOTE_D4  4699
#define NOTE_DS4 4978
#define NOTE_E4  5274
#define NOTE_F4  5588
#define NOTE_FS4 5920
#define NOTE_G4  6272
#define NOTE_GS4 6645
#define NOTE_A4  7040
#define NOTE_AS4 7459
#define NOTE_B4  7902

// Сольфеджио с новыми октавами
// 0-я октава
#define SI0  NOTE_B0   // 494 Hz

// 1-я октава
#define DO1  NOTE_C1   // 523 Hz
#define RE1  NOTE_D1   // 587 Hz
#define MI1  NOTE_E1   // 659 Hz
#define FA1  NOTE_F1   // 698 Hz
#define SOL1 NOTE_G1   // 784 Hz
#define LA1  NOTE_A1   // 880 Hz
#define SI1  NOTE_B1   // 988 Hz

// 2-я октава
#define DO2  NOTE_C2   // 1047 Hz
#define RE2  NOTE_D2   // 1175 Hz
#define MI2  NOTE_E2   // 1319 Hz
#define FA2  NOTE_F2   // 1397 Hz
#define SOL2 NOTE_G2   // 1568 Hz
#define LA2  NOTE_A2   // 1760 Hz
#define SI2  NOTE_B2   // 1976 Hz

// 3-я октава
#define DO3  NOTE_C3   // 2093 Hz
#define RE3  NOTE_D3   // 2349 Hz
#define MI3  NOTE_E3   // 2637 Hz
#define FA3  NOTE_F3   // 2794 Hz
#define SOL3 NOTE_G3   // 3136 Hz
#define LA3  NOTE_A3   // 3520 Hz
#define SI3  NOTE_B3   // 3951 Hz

// 4-я октава
#define DO4  NOTE_C4   // 4186 Hz
#define RE4  NOTE_D4   // 4699 Hz
#define MI4  NOTE_E4   // 5274 Hz
#define FA4  NOTE_F4   // 5588 Hz
#define SOL4 NOTE_G4   // 6272 Hz
#define LA4  NOTE_A4   // 7040 Hz
#define SI4  NOTE_B4   // 7902 Hz

// Диезы для 1-й октавы
#define DO1s NOTE_CS1  // 554 Hz
#define RE1s NOTE_DS1  // 622 Hz
#define FA1s NOTE_FS1  // 740 Hz
#define SOL1s NOTE_GS1 // 831 Hz
#define LA1s NOTE_AS1  // 932 Hz

// Диезы для 2-й октавы
#define DO2s NOTE_CS2  // 1109 Hz
#define RE2s NOTE_DS2  // 1245 Hz
#define FA2s NOTE_FS2  // 1480 Hz
#define SOL2s NOTE_GS2 // 1661 Hz
#define LA2s NOTE_AS2  // 1865 Hz

// Диезы для 3-й октавы
#define DO3s NOTE_CS3  // 2217 Hz
#define RE3s NOTE_DS3  // 2489 Hz
#define FA3s NOTE_FS3  // 2960 Hz
#define SOL3s NOTE_GS3 // 3322 Hz
#define LA3s NOTE_AS3  // 3729 Hz

// Диезы для 4-й октавы
#define DO4s NOTE_CS4  // 4435 Hz
#define RE4s NOTE_DS4  // 4978 Hz
#define FA4s NOTE_FS4  // 5920 Hz
#define SOL4s NOTE_GS4 // 6645 Hz
#define LA4s NOTE_AS4  // 7459 Hz

#define NOTE_REST 0


#endif
