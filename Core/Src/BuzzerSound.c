/*****************************************************************************
程序名称：音乐蜂鸣驱动
程 序 员：李锦上 Email：lijinshang@126.com
使用说明：
// 初始化蜂鸣器
// 参数：定时器句柄, PWM通道, 定时器时钟频率(Hz)
buzzer_init(&buzzer_ctrl, &htim11, TIM_CHANNEL_1, 1000000);

// 播放连接音
buzzer_play_sound(&buzzer_ctrl, SND_CONNECTION);
osDelay(1000);

// 播放断开音
buzzer_play_sound(&buzzer_ctrl, SND_DISCONNECTION);
osDelay(1000);

// 播放按钮音
buzzer_play_sound(&buzzer_ctrl, SND_BUTTON_PUSHED);
osDelay(1000);



修改历史：
2025-6-24：移植于Arduino库 有改动
******************************************************************************/
#include "BuzzerSound.h"
#include <math.h>
#include <string.h>

BuzzerControl buzzer_ctrl;

typedef struct
{
	const float *melody;
	const uint16_t *duration;
	uint32_t notes;
} MusicSegment;

static const float pirate_melody[] =
{
	NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
	NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0, NOTE_A4, NOTE_G4, NOTE_A4, 0
};
static const uint16_t pirate_duration[] =
{
	125, 125, 250, 125, 125, 125, 125, 250, 125, 125,
	125, 125, 250, 125, 125, 125, 125, 375, 250
};
static const MusicSegment music_pirates =
{
	pirate_melody, pirate_duration,
	sizeof(pirate_melody)/sizeof(pirate_melody[0])
};

static const float star_melody[] =
{
	NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4, 0,
	NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4, 0,
	NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, 0,
	NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, 0,
	NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4, 0,
	NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4, 0
};
static const uint16_t star_duration[] =
{
	200, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 800, 200
};
static const MusicSegment music_star =
{
	star_melody, star_duration,
	sizeof(star_melody)/sizeof(star_melody[0])
};

static const float ode_melody[] =
{
	NOTE_E4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, 0,
	NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, 0,
	NOTE_E4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, 0,
	NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_C4, 0
};
static const uint16_t ode_duration[] =
{
	200, 200, 200, 200, 200, 200, 200, 200, 200,
	200, 200, 200, 200, 300, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 200, 200, 200,
	200, 200, 200, 200, 300, 200, 800, 200
};
static const MusicSegment music_ode =
{
	ode_melody, ode_duration,
	sizeof(ode_melody)/sizeof(ode_melody[0])
};

static const float birthday_melody[] =
{
	NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4, 0,
	NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4, 0,
	NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4, 0,
	NOTE_AS4, NOTE_AS4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4, 0
};
static const uint16_t birthday_duration[] =
{
	200, 100, 300, 300, 300, 600, 200,
	200, 100, 300, 300, 300, 600, 200,
	200, 100, 300, 300, 300, 300, 600, 200,
	200, 100, 300, 300, 400, 800, 200
};
static const MusicSegment music_birthday =
{
	birthday_melody, birthday_duration,
	sizeof(birthday_melody)/sizeof(birthday_melody[0])
};

static const float canon_melody[] =
{
	NOTE_D4, NOTE_A4, NOTE_B4, NOTE_FS4, NOTE_G4, NOTE_D4, 0,
	NOTE_G4, NOTE_A4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_FS4, NOTE_G4, NOTE_A4, 0,
	NOTE_D5, NOTE_A4, NOTE_B4, NOTE_FS4, NOTE_G4, NOTE_D4, 0,
	NOTE_G4, NOTE_A4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_FS4, NOTE_G4, NOTE_A4, 0
};
static const uint16_t canon_duration[] =
{
	300, 300, 300, 300, 300, 300, 100,
	300, 600, 200, 200, 200, 200, 200, 400, 200,
	300, 300, 300, 300, 300, 300, 100,
	300, 600, 200, 200, 200, 200, 200, 800, 200
};
static const MusicSegment music_canon =
{
	canon_melody, canon_duration,
	sizeof(canon_melody)/sizeof(canon_melody[0])
};

static const float jingle_melody[] =
{
	NOTE_E4, NOTE_E4, NOTE_E4, 0,
	NOTE_E4, NOTE_E4, NOTE_E4, 0,
	NOTE_E4, NOTE_G4, NOTE_C4, NOTE_D4, NOTE_E4, 0,
	NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_E4, 0,
	NOTE_E4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, 0, NOTE_G4, 0
};
static const uint16_t jingle_duration[] =
{
	300, 300, 600, 200,
	300, 300, 600, 200,
	300, 300, 300, 300, 600, 200,
	200, 200, 300, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 600, 100, 800, 200
};
static const MusicSegment music_jingle =
{
	jingle_melody, jingle_duration,
	sizeof(jingle_melody)/sizeof(jingle_melody[0])
};

static const float lamb_melody[] =
{
	NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, 0,
	NOTE_E4, NOTE_E4, NOTE_E4, 0,
	NOTE_D4, NOTE_D4, NOTE_D4, 0,
	NOTE_E4, NOTE_G4, NOTE_G4, 0,
	NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, 0,
	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, 0,
	NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_C4, 0
};
static const uint16_t lamb_duration[] =
{
	200, 200, 200, 200, 100,
	200, 200, 400, 100,
	200, 200, 400, 100,
	200, 200, 400, 100,
	200, 200, 200, 200, 100,
	200, 200, 200, 400, 100,
	200, 200, 200, 200, 800, 200
};
static const MusicSegment music_lamb =
{
	lamb_melody, lamb_duration,
	sizeof(lamb_melody)/sizeof(lamb_melody[0])
};

static const float auld_melody[] =
{
	NOTE_C4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_F4, NOTE_G4, 0,
	NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_E4, NOTE_D4, 0,
	NOTE_C4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_F4, NOTE_G4, 0,
	NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_D4, NOTE_C4, 0
};
static const uint16_t auld_duration[] =
{
	400, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 200, 200, 400, 200,
	400, 200, 200, 200, 200, 200, 400, 200,
	200, 200, 200, 200, 300, 200, 800, 200
};
static const MusicSegment music_auld =
{
	auld_melody, auld_duration,
	sizeof(auld_melody)/sizeof(auld_melody[0])
};

static const float wedding_melody[] =
{
	NOTE_C4, NOTE_G4, NOTE_C4, NOTE_G4, 0,
	NOTE_C4, NOTE_G4, NOTE_C4, NOTE_G4, 0,
	NOTE_E4, NOTE_G4, NOTE_E4, NOTE_G4, 0,
	NOTE_C4, NOTE_G4, NOTE_C4, NOTE_G4, 0,
	NOTE_C5, NOTE_G4, NOTE_E4, NOTE_G4, 0,
	NOTE_C5, NOTE_G4, NOTE_E4, NOTE_G4, 0
};
static const uint16_t wedding_duration[] =
{
	300, 300, 300, 300, 100,
	300, 300, 300, 300, 100,
	300, 300, 300, 600, 100,
	300, 300, 300, 300, 100,
	300, 300, 300, 300, 100,
	300, 300, 300, 800, 200
};
static const MusicSegment music_wedding =
{
	wedding_melody, wedding_duration,
	sizeof(wedding_melody)/sizeof(wedding_melody[0])
};

static const float danube_melody[] =
{
	NOTE_D4, NOTE_A4, NOTE_B4, NOTE_FS4, 0,
	NOTE_G4, NOTE_D4, NOTE_G4, NOTE_A4, 0,
	NOTE_D5, NOTE_D5, NOTE_E5, NOTE_FS5, 0,
	NOTE_G5, NOTE_FS5, NOTE_E5, NOTE_D5, 0,
	NOTE_CS5, NOTE_B4, NOTE_A4, NOTE_G4, 0,
	NOTE_FS4, NOTE_E4, NOTE_D4, 0
};
static const uint16_t danube_duration[] =
{
	400, 200, 200, 400, 100,
	300, 300, 300, 600, 100,
	300, 200, 200, 300, 100,
	300, 200, 200, 300, 100,
	300, 200, 200, 300, 100,
	300, 300, 800, 200
};
static const MusicSegment music_danube =
{
	danube_melody, danube_duration,
	sizeof(danube_melody)/sizeof(danube_melody[0])
};

static const float elise_melody[] =
{
	NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_DS4, 0,
	NOTE_E4, NOTE_B3, NOTE_D4, NOTE_C4, 0,
	NOTE_A3, NOTE_C3, NOTE_E3, NOTE_A3, 0,
	NOTE_B3, NOTE_E3, NOTE_GS3, NOTE_B3, NOTE_C4, 0,
	NOTE_E4, NOTE_DS4, NOTE_E4, NOTE_DS4, 0,
	NOTE_E4, NOTE_B3, NOTE_D4, NOTE_C4, 0,
	NOTE_A3, NOTE_C3, NOTE_E3, NOTE_A3, 0,
	NOTE_B3, NOTE_E3, NOTE_C4, NOTE_B3, NOTE_A3, 0
};
static const uint16_t elise_duration[] =
{
	200, 200, 200, 200, 100,
	200, 200, 200, 600, 100,
	300, 300, 300, 300, 100,
	300, 300, 300, 300, 600, 100,
	200, 200, 200, 200, 100,
	200, 200, 200, 600, 100,
	300, 300, 300, 300, 100,
	300, 300, 300, 300, 800, 200
};
static const MusicSegment music_elise =
{
	elise_melody, elise_duration,
	sizeof(elise_melody)/sizeof(elise_melody[0])
};


void buzzer_init(BuzzerControl* ctrl, TIM_HandleTypeDef* htim, uint32_t channel, uint32_t timer_freq)
{
    ctrl->htim = htim;
    ctrl->channel = channel;
    ctrl->timer_freq = timer_freq;

    if (HAL_TIM_PWM_Start(ctrl->htim, ctrl->channel) != HAL_OK)
    {
    }
    __HAL_TIM_DISABLE_IT(ctrl->htim, TIM_IT_UPDATE);
    __HAL_TIM_DISABLE_IT(ctrl->htim, TIM_IT_CC1);
}

void buzzer_stop(BuzzerControl* ctrl)
{
    __HAL_TIM_SET_COMPARE(ctrl->htim, ctrl->channel, 0);
}

static void play_tone(BuzzerControl* ctrl, float freq, uint32_t duration_ms)
{
    if (freq < 20 || duration_ms == 0)
    {
        osDelay(duration_ms);
        return;
    }

    uint32_t period_ticks = (uint32_t)(ctrl->timer_freq / freq);
    uint32_t pulse_ticks = period_ticks / 2;

    __HAL_TIM_SET_AUTORELOAD(ctrl->htim, period_ticks - 1);
    __HAL_TIM_SET_COMPARE(ctrl->htim, ctrl->channel, pulse_ticks);

    if (duration_ms > 0)
    {
        osDelay(duration_ms);
    }

    __HAL_TIM_SET_COMPARE(ctrl->htim, ctrl->channel, 0);
}

static void bend_tones(BuzzerControl* ctrl, float start_freq, float end_freq,
                       float step_factor, uint32_t duration, uint16_t pause_ms)
{
    if (start_freq < 20 || end_freq < 20) return;

    if (start_freq < end_freq)
    {
        for (float freq = start_freq; freq < end_freq; freq *= step_factor)
        {
            play_tone(ctrl, freq, duration);
            if (pause_ms > 0)
            {
                osDelay(pause_ms);
            }
        }
    }
    else
    {
        for (float freq = start_freq; freq > end_freq; freq /= step_factor)
        {
            play_tone(ctrl, freq, duration);
            if (pause_ms > 0)
            {
                osDelay(pause_ms);
            }
        }
    }
}

static void play_music(BuzzerControl* ctrl, const MusicSegment *music)
{
    for (uint32_t i = 0; i < music->notes; i++)
    {
        if (music->melody[i] > 0)
        {
            play_tone(ctrl, music->melody[i], music->duration[i]);
        }
        else
        {
            osDelay(music->duration[i]);
        }
    }
}

void buzzer_play_sound(BuzzerControl* ctrl, SoundType sound)
{
    (void)ctrl;
    if (BuzzerQueueHandle != NULL)
    {
        osStatus res = osMessagePut(BuzzerQueueHandle, (uint32_t)sound, 0);
        if (res != osOK)
        {
            (void)osMessageGet(BuzzerQueueHandle, 0);
            (void)osMessagePut(BuzzerQueueHandle, (uint32_t)sound, 0);
        }
    }
}

void buzzer_play_sound_handle_in_task(BuzzerControl* ctrl, SoundType sound)
{
    switch (sound)
    {
    case SND_CONNECTION:
        play_tone(ctrl, NOTE_E5, 50);
        osDelay(30);
        play_tone(ctrl, NOTE_E6, 55);
        osDelay(25);
        play_tone(ctrl, NOTE_A6, 60);
        break;

    case SND_DISCONNECTION:
        play_tone(ctrl, NOTE_E5, 50);
        osDelay(30);
        play_tone(ctrl, NOTE_A6, 55);
        osDelay(25);
        play_tone(ctrl, NOTE_E6, 50);
        osDelay(60);
        break;

    case SND_BUTTON_PUSHED:
        bend_tones(ctrl, NOTE_E6, NOTE_G6, 1.03f, 20, 2);
        osDelay(30);
        bend_tones(ctrl, NOTE_E6, NOTE_D7, 1.04f, 10, 2);
        break;

    case SND_MODE1:
        bend_tones(ctrl, NOTE_E6, NOTE_A6, 1.02f, 30, 10);
        break;

    case SND_MODE2:
        bend_tones(ctrl, NOTE_G6, NOTE_D7, 1.03f, 30, 10);
        break;

    case SND_MODE3:
        play_tone(ctrl, NOTE_E6, 50);
        osDelay(100);
        play_tone(ctrl, NOTE_G6, 50);
        osDelay(80);
        play_tone(ctrl, NOTE_D7, 300);
        break;

    case SND_SURPRISE:
        bend_tones(ctrl, 800, 2150, 1.02f, 10, 1);
        bend_tones(ctrl, 2149, 800, 1.03f, 7, 1);
        break;

    case SND_JUMP:
        bend_tones(ctrl, 880, 2000, 1.04f, 8, 3);
        osDelay(200);
        break;

    case SND_OHOOH:
        bend_tones(ctrl, 880, 2000, 1.04f, 8, 3);
        osDelay(200);
        for (float freq = 880; freq < 2000; freq *= 1.04f)
        {
            play_tone(ctrl, NOTE_B5, 5);
            osDelay(10);
        }
        break;

    case SND_OHOOH2:
        bend_tones(ctrl, 1880, 3000, 1.03f, 8, 3);
        osDelay(200);
        for (float freq = 1880; freq < 3000; freq *= 1.03f)
        {
            play_tone(ctrl, NOTE_C6, 10);
            osDelay(10);
        }
        break;

    case SND_CUDDLY:
        bend_tones(ctrl, 700, 900, 1.03f, 16, 4);
        bend_tones(ctrl, 899, 650, 1.01f, 18, 7);
        break;

    case SND_SLEEPING:
        bend_tones(ctrl, 100, 500, 1.04f, 10, 10);
        osDelay(500);
        bend_tones(ctrl, 400, 100, 1.04f, 10, 1);
        break;

    case SND_HAPPY:
        bend_tones(ctrl, 1500, 2500, 1.05f, 20, 8);
        bend_tones(ctrl, 2499, 1500, 1.05f, 25, 8);
        break;

    case SND_SUPER_HAPPY:
        bend_tones(ctrl, 2000, 6000, 1.05f, 8, 3);
        osDelay(50);
        bend_tones(ctrl, 5999, 2000, 1.05f, 13, 2);
        break;

    case SND_HAPPY_SHORT:
        bend_tones(ctrl, 1500, 2000, 1.05f, 15, 8);
        osDelay(100);
        bend_tones(ctrl, 1900, 2500, 1.05f, 10, 8);
        break;

    case SND_SAD:
        bend_tones(ctrl, 880, 669, 1.02f, 20, 200);
        break;

    case SND_CONFUSED:
        bend_tones(ctrl, 1000, 1700, 1.03f, 8, 2);
        bend_tones(ctrl, 1699, 500, 1.04f, 8, 3);
        bend_tones(ctrl, 1000, 1700, 1.05f, 9, 10);
        break;

    case SND_FART1:
        bend_tones(ctrl, 1600, 3000, 1.02f, 2, 15);
        break;

    case SND_FART2:
        bend_tones(ctrl, 2000, 6000, 1.02f, 2, 20);
        break;

    case SND_FART3:
        bend_tones(ctrl, 1600, 4000, 1.02f, 2, 20);
        bend_tones(ctrl, 4000, 3000, 1.02f, 2, 20);
        break;

    case SND_PIRATES:
        play_music(ctrl, &music_pirates);
        break;

    case SND_STAR:
        play_music(ctrl, &music_star);
        break;

    case SND_ODE:
        play_music(ctrl, &music_ode);
        break;

    case SND_BIRTHDAY:
        play_music(ctrl, &music_birthday);
        break;

    case SND_CANON:
        play_music(ctrl, &music_canon);
        break;

    case SND_JINGLE:
        play_music(ctrl, &music_jingle);
        break;

    case SND_LAMB:
        play_music(ctrl, &music_lamb);
        break;

    case SND_AULD:
        play_music(ctrl, &music_auld);
        break;

    case SND_WEDDING:
        play_music(ctrl, &music_wedding);
        break;

    case SND_DANUBE:
        play_music(ctrl, &music_danube);
        break;

    case SND_ELISE:
        play_music(ctrl, &music_elise);
        break;

    case SND_BEEP:
        play_tone(ctrl, 800, 200);
        break;

    case SND_SIREN:
        play_tone(ctrl, 400, 200);
        play_tone(ctrl, 800, 200);
        osDelay(50);
        break;

    default:
        break;
    }

    buzzer_stop(ctrl);
}
