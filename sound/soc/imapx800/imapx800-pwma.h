#ifndef __IMAPX800_PWMA_H__
#define __IMAPX800_PWMA_H__

#define PWM_AUDIO_CFG0          0x0
#define PWM_AUDIO_CFG1          0x4
#define PWM_AUDIO_TCON          0x8 
#define PWM_AUDIO_TCNTB0        0xC
#define PWM_AUDIO_TCNTO0        0x14
#define PWM_AUDIO_TCNTB1        0x18
#define PWM_AUDIO_TCNTO1        0x20
#define PWM_AUDIO_TSTSEN        0x44
#define PWM_AUDIO_TINTEN        0x48
#define PWM_AUDIO_TINT          0x4C
#define PWM_AUDIO_TCFG2         0x50
#define PWM_AUDIO_TSTS          0x54
#define PWM_AUDIO_FIFO          0x60

#define PWM_AUDIO_START         0x1
#define PWM_AUDIO_STOP          0x0

#define PWM_AUDIO_DOUBLE        0x1
#define PWM_AUDIO_SINGLE        0x0

/* only support in imapx800 pwma */
#define SNDRV_PCM_RATE_128000   128000
#define SNDRV_PCM_RATE_132300   132300

struct imapx800_pwma_info {
    struct device   *dev;
    void __iomem    *regs;
    struct clk  *extclk;
    unsigned int    iis_clock_rate;
    int chan_num;
};
#endif
