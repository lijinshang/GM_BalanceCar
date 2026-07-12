#ifndef BUZZER_SOUND_H
#define BUZZER_SOUND_H

#include "main.h"
#include "cmsis_os.h"

typedef enum
{
	SND_NONE = -1,
	SND_CONNECTION = 0,
	SND_DISCONNECTION,
	SND_BUTTON_PUSHED,
	SND_MODE1,
	SND_MODE2,
	SND_MODE3,
	SND_SURPRISE,
	SND_JUMP,
	SND_OHOOH,
	SND_OHOOH2,
	SND_CUDDLY,
	SND_SLEEPING,
	SND_HAPPY,
	SND_SUPER_HAPPY,
	SND_HAPPY_SHORT,
	SND_SAD,
	SND_ERROR,
	SND_CONFUSED,
	SND_FART1,
	SND_FART2,
	SND_FART3,
	SND_BEEP,
	SND_SIREN,
	SND_PIRATES,
	SND_STAR,
	SND_ODE,
	SND_BIRTHDAY,
	SND_CANON,
	SND_JINGLE,
	SND_LAMB,
	SND_AULD,
	SND_WEDDING,
	SND_DANUBE,
	SND_ELISE,
} SoundType;

#define NOTE_C0     16.35
#define NOTE_Db0    17.32
#define NOTE_D0     18.35
#define NOTE_Eb0    19.45
#define NOTE_E0     20.60
#define NOTE_F0     21.83
#define NOTE_Gb0    23.12
#define NOTE_G0     24.50
#define NOTE_Ab0    25.96
#define NOTE_A0     27.50
#define NOTE_Bb0    29.14
#define NOTE_B0     30.87
#define NOTE_C1     32.70
#define NOTE_Db1    34.65
#define NOTE_D1     36.71
#define NOTE_Eb1    38.89
#define NOTE_E1     41.20
#define NOTE_F1     43.65
#define NOTE_Gb1    46.25
#define NOTE_G1     49.00
#define NOTE_Ab1    51.91
#define NOTE_A1     55.00
#define NOTE_Bb1    58.27
#define NOTE_B1     61.74
#define NOTE_C2     65.41
#define NOTE_Db2    69.30
#define NOTE_D2     73.42
#define NOTE_Eb2    77.78
#define NOTE_E2     82.41
#define NOTE_F2     87.31
#define NOTE_Gb2    92.50
#define NOTE_G2     98.00
#define NOTE_Ab2    103.83
#define NOTE_A2     110.00
#define NOTE_Bb2    116.54
#define NOTE_B2     123.47
#define NOTE_C3     130.81
#define NOTE_Db3    138.59
#define NOTE_D3     146.83
#define NOTE_Eb3    155.56
#define NOTE_E3     164.81
#define NOTE_F3     174.61
#define NOTE_Gb3    185.00
#define NOTE_G3     196.00
#define NOTE_Ab3    207.65
#define NOTE_A3     220.00
#define NOTE_Bb3    233.08
#define NOTE_B3     246.94
#define NOTE_C4     261.63
#define NOTE_Db4    277.18
#define NOTE_D4     293.66
#define NOTE_Eb4    311.13
#define NOTE_E4     329.63
#define NOTE_F4     349.23
#define NOTE_Gb4    369.99
#define NOTE_G4     392.00
#define NOTE_Ab4    415.30
#define NOTE_A4     440.00
#define NOTE_Bb4    466.16
#define NOTE_B4     493.88
#define NOTE_C5     523.25
#define NOTE_Db5    554.37
#define NOTE_D5     587.33
#define NOTE_Eb5    622.25
#define NOTE_E5     659.26
#define NOTE_F5     698.46
#define NOTE_Gb5    739.99
#define NOTE_G5     783.99
#define NOTE_Ab5    830.61
#define NOTE_A5     880.00
#define NOTE_Bb5    932.33
#define NOTE_B5     987.77
#define NOTE_C6     1046.50
#define NOTE_Db6    1108.73
#define NOTE_D6     1174.66
#define NOTE_Eb6    1244.51
#define NOTE_E6     1318.51
#define NOTE_F6     1396.91
#define NOTE_Gb6    1479.98
#define NOTE_G6     1567.98
#define NOTE_Ab6    1661.22
#define NOTE_A6     1760.00
#define NOTE_Bb6    1864.66
#define NOTE_B6     1975.53
#define NOTE_C7     2093.00
#define NOTE_Db7    2217.46
#define NOTE_D7     2349.32
#define NOTE_Eb7    2489.02
#define NOTE_E7     2637.02
#define NOTE_F7     2793.83
#define NOTE_Gb7    2959.96
#define NOTE_G7     3135.96
#define NOTE_Ab7    3322.44
#define NOTE_A7     3520.00
#define NOTE_Bb7    3729.31
#define NOTE_B7     3951.07
#define NOTE_C8     4186.01
#define NOTE_Db8    4434.92


#define NOTE_GS3    208.00
#define NOTE_DS4    311.00
#define NOTE_FS4    370.00
#define NOTE_AS4    466.00
#define NOTE_CS5    554.00
#define NOTE_FS5    740.00


typedef struct
{
	TIM_HandleTypeDef* htim;
	uint32_t channel;
	uint32_t timer_freq;
} BuzzerControl;

extern BuzzerControl buzzer_ctrl;

extern osMessageQId BuzzerQueueHandle;

void buzzer_init(BuzzerControl* ctrl, TIM_HandleTypeDef* htim, uint32_t channel, uint32_t timer_freq);
void buzzer_play_sound(BuzzerControl* ctrl, SoundType sound);
void buzzer_play_sound_handle_in_task(BuzzerControl* ctrl, SoundType sound);
void buzzer_stop(BuzzerControl* ctrl);

#endif /* BUZZER_SOUND_H */
