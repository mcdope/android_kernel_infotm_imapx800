#include <linux/module.h>   
#include <linux/ioport.h>   
#include <mach/imap-iomap.h>
#include <mach/imap-mac.h>  
#include <asm/io.h>         
#include <linux/gpio.h>     
#include <mach/power-gate.h>
#include <mach/pad.h>       
#include <linux/init.h>     
#include <asm/delay.h>      
#include <linux/delay.h>    
#include <mach/items.h>    
#include <mach/imap-iis.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/cache.h>
#include <linux/io.h>
#include <linux/poll.h>

struct reg{
	char regindex;
	short int val;
}reg;
struct reg record_reg[]={
	{0x00, 0x0000},
	{0x02, 0x4040},
	{0x04, 0x4040},
	{0x06, 0xa080},
	{0x0a, 0x0808},
	{0x0c, 0x0000},
	{0x0e, 0x8080},
	{0x10, 0x2020},
	{0x12, 0x000b},
	{0x14, 0xb0f0},
	{0x16, 0x0000},
	{0x18, 0x0010},
	{0x1a, 0xdfc0},
	{0x1c, 0xdfc0},
	{0x1e, 0x88c0},
	{0x20, 0x88c0},
	{0x22, 0x3000},
	{0x24, 0x3000},
	{0x26, 0x8808},
	{0x28, 0xd8d8},
	{0x2a, 0x6c00},
	{0x2c, 0x0000},
	{0x34, 0x8000},
	{0x36, 0x8000},
	{0x38, 0x1010},
	{0x3a, 0x9fe0},
	{0x3b, 0xf828},
	{0x3c, 0xe01e},
	{0x3e, 0xcc00},
	{0x40, 0x2e00},
	{0x42, 0x0000},
	{0x44, 0x0000},
	{0x48, 0x2000},
	{0x4a, 0x4f10},
	{0x4c, 0x0000},
	{0x52, 0x2040},
	{0x54, 0xc003},
	{0x56, 0x8000},
	{0x5a, 0x4e80},
	{0x5c, 0x07e0},
	{0x64, 0x0206},
	{0x65, 0x0000},
	{0x66, 0x2000},
	{0x68, 0x0553},
	{0x6a, 0x0056},
	{0x6c, 0x302f},
	{0x6e, 0x0000},
	{0x7a, 0x0003},
	{0x7c, 0x10ec},
	{0x7e, 0x6132},
};


struct pcbtest_audio {
	char *name;
	int (*init)(void);
	int (*record_init)(void);
};

struct imapx800_i2s_info {
	void __iomem    *regs;
	void __iomem    *mclk;
	void __iomem    *bclk;
	void __iomem    *interface;
	struct clk  *iis_clk;
	unsigned int    iis_clock_rate;
} imapx800_i2s;
int pcbtest_audio_total = 0;
EXPORT_SYMBOL(pcbtest_audio_total);
char *audio_buf = NULL;
#define AUDIO_BUFFER_LEN 	(1920000 + 4096)
unsigned int    iis_clock_rate;
void msleep(unsigned int msecs);
unsigned int    iis_clock_rate;
extern int alc5631_play(void);
extern void imapx800_snd_rxctrl(int on);
extern  int snd_soc_update_bits(struct snd_soc_codec *codec, unsigned short reg,
		                  unsigned int mask, unsigned int value);
extern unsigned int snd_soc_write(struct snd_soc_codec *codec,
		               unsigned int reg, unsigned int val);
extern int imapx800_switch_spk(int dir);
extern int imapx_set_AudioRunStatus1(void);
extern int imapx_set_audioRunStatus(void); 
extern void imapx800_snd_txctrl(int on);
static int has_init=0;      //used to count the init times,init once is enougn for most of the reg
static int enc_or_dec = 0;  //used to judge if encoding or decoding init is going to be done.
extern void imapx800_snd_txctrl(int on);

struct rt5621_reg{
	u8  reg_index;
	u16     reg_value;
	u16     mask;
};
static struct rt5621_reg recording_data[] = {
	{0x34   ,0x8000,-1},
	{0x34   ,0x800c,-1},
	{0x36   ,0x1a2d,-1},
	{0x3a   ,0x8930,-1},
	{0x3c   ,0xe7f3,-1},
	{0x3e   ,0x9603,-1},
	{0x14   ,0x3f3f,-1},
};
extern struct snd_soc_codec *rt5631_codec;
int alc5631_record_init(void)
{
	int i,  ret=0;

	printk(KERN_ERR"Ready for ALC5621 initiation.\r\n");
	// init for recording
	for (i=0; i<ARRAY_SIZE(record_reg); i++){
	//	ret = HW_WriteRegisters(recording_data[i].reg_index, &recording_data[i].reg_value, &recording_data[i].mask);
		snd_soc_write(rt5631_codec,imapx800_i2s.regs + record_reg[i].regindex , record_reg[i].val);
	}
}
int es8328_play(void)
{
	return 0;
}

int virtual_play(void)
{
	return 0;
}

struct pcbtest_audio audios[] = {
    {
	.name = "rt5631",
	.init = alc5631_play,
	.record_init = alc5631_record_init,
    },
    {
	.name = "es8328",
	.init = es8328_play,
	.record_init = es8328_play,
    },
    {
	.name = "virtual",
	.init = virtual_play,
	.record_init = virtual_play, 
    },
};

void pcbtest_set_clock(void)
{
    writel(2, imapx800_i2s.mclk);
    writel(23, imapx800_i2s.bclk);
    writel(0x6, imapx800_i2s.interface);
}

int pcbtest_init_iis(int encode)
{
    module_power_on(SYSMGR_IIS_BASE);
    imapx_pad_cfg(IMAPX_IIS0,1);

    imapx800_i2s.regs = ioremap(IMAP_IIS0_BASE, SZ_4K);
    imapx800_i2s.mclk = ioremap(rI2S_MCLKDIV, 4);
    imapx800_i2s.bclk = ioremap(rI2S_BCLKDIV, 4);
    imapx800_i2s.interface = ioremap(rI2S_INTERFACE, 4);

    if (imapx800_i2s.regs == NULL || imapx800_i2s.mclk == NULL
	    || imapx800_i2s.bclk == NULL || imapx800_i2s.interface == NULL)
    {
	printk(KERN_ERR "regs is null, exit!\n");
	return -ENXIO;
    }
    imapx800_i2s.iis_clk = clk_get(NULL, "audio-clk");
    if (imapx800_i2s.iis_clk == NULL) {
	printk(KERN_ERR "failed to get iis_clock\n");
	return -ENODEV;
    }
    clk_enable(imapx800_i2s.iis_clk);
    imapx800_i2s.iis_clock_rate = clk_get_rate(imapx800_i2s.iis_clk);
    printk("iis clock is %d\n", imapx800_i2s.iis_clock_rate);
    pcbtest_set_clock();
	
	if(0==encode){
		writel(0, imapx800_i2s.regs + rRER0);
		writel(0, imapx800_i2s.regs + rTER0);
		writel(0, imapx800_i2s.regs + rRER1);
		writel(0, imapx800_i2s.regs + rTER1);
		writel(0, imapx800_i2s.regs + rRER2);
		writel(0, imapx800_i2s.regs + rTER2);
		writel(0, imapx800_i2s.regs + rRER3);
		writel(0, imapx800_i2s.regs + rTER3);
		writel(0x0, imapx800_i2s.regs + rCCR);//16 clock ,no gate
		writel(0x2, imapx800_i2s.regs + rTCR0); //16 bit resolution
		writel(0x8, imapx800_i2s.regs + rTFCR0);//when 8 enters in tx fifo ,a interrupt occur
		writel(((0x1<<5) | (0x1<<4) | (0x1<<1) | 0x1),imapx800_i2s.regs + rIMR0);
		//imapx800_snd_txctrl(1);
	}else{
//		imapx800_snd_rxctrl(1);
	}
    return 0;
}

void pcbtest_start_play(bool auto_playback)
{
	int len, count = 0;
	int tmp;
	unsigned short *start_audio = (unsigned short *)audio_buf;

	//enable the speak
	//imapx_set_AudioRunStatus1();
	imapx_set_audioRunStatus();
        if (auto_playback)
	        imapx800_switch_spk(1);
	writel(0x1, imapx800_i2s.regs + rIER);
	writel(0x1, imapx800_i2s.regs + rITER);//I2S Transmitter Block Enable
	writel(0x1, imapx800_i2s.regs + rTER0);
	writel(0x1, imapx800_i2s.regs + rCER);//clock enable

	len = pcbtest_audio_total >>2;
    unsigned long to;
    if (auto_playback) 
	    to = jiffies + 3*HZ;
    else
        to = jiffies + 10;

	printk("Hz:%d\n",HZ);
	for(;count < len && jiffies < to ;count++)
	{
		tmp = readl(imapx800_i2s.regs + rISR0);
		if(tmp & (0x1<<4))
		{
			writel(*start_audio,imapx800_i2s.regs + rLTHR0);
			writel(*start_audio,imapx800_i2s.regs + rRTHR0);
			start_audio++;
			start_audio++;
			//	    printk("audio is %x\n", *start_audio);
		}
		else
		{
			count -= 1;
		}
	}
	kfree (audio_buf);
	audio_buf = NULL;

	writel(0x1, imapx800_i2s.regs + rRFF0);
	writel(0x0, imapx800_i2s.regs + rIRER);
	writel(0x0, imapx800_i2s.regs + rRER0);
}

int audio_play (bool auto_playback) 
{
	char codec_name[ITEM_MAX_LEN];
	int i, ret;

	printk("audio play enter\n");

	if (!item_exist("codec.model")) {
		printk("No codec machine\n");
		return -ENOMEM;
	}
	item_string(codec_name, "codec.model", 0);
	for (i = 0;i < ARRAY_SIZE(audios);i++) {
		printk("codec %s ========== %s\n", audios[i].name, codec_name);
		if (!strcmp(audios[i].name, codec_name)) {
			ret = pcbtest_init_iis(0);
			if (ret < 0) {
				printk("Init iis err\n");
				return -1;
			}
                        printk("finish init i2s\n");
                        if (auto_playback) {
                            if (audios[i].init() < 0) {
                                printk("Init codec err\n");
                                return -1;
                            }
                        }
                        printk("finish init codec\n");
			pcbtest_start_play(auto_playback);
                        printk("finish playback\n");
			return 0;
		}
	}
	printk("No match codec %s\n", codec_name);
	return -1;
}
EXPORT_SYMBOL(audio_play);

int audio_auto_playback_init(void)
{
        if (audio_buf == NULL) {
                audio_buf = (char *)kzalloc(AUDIO_BUFFER_LEN, GFP_KERNEL);
                if (audio_buf == NULL) {
                        printk("can't malloc audio buffer\n");
                        return -1;
                }
                pcbtest_audio_total = AUDIO_BUFFER_LEN;       
        }

        return 0;
}
EXPORT_SYMBOL(audio_auto_playback_init);

int audio_send_frame(char *buf, int frames)
{
	if (audio_buf == NULL) {
		audio_buf = (char *)kzalloc(AUDIO_BUFFER_LEN, GFP_KERNEL);
		if (audio_buf == NULL) {
			printk("can't malloc audio buffer\n");
			return -1;
		}
	}

	if (copy_from_user(audio_buf + pcbtest_audio_total, buf, frames))
		return -EFAULT;

	pcbtest_audio_total += frames;
	if (pcbtest_audio_total > AUDIO_BUFFER_LEN) {
		printk("pcbtest send frames err\n");
		return -1;
	}

	return 0;
}
int audio_record( void)
{
	int time = 0x4000;

	if (audio_buf == NULL) {                                 
		audio_buf = (char *)kzalloc(AUDIO_BUFFER_LEN, GFP_KERNEL); 
		if (audio_buf == NULL) {                                 
			printk("can't malloc audio buffer\n");               
			return -ENOMEM;                                           
		}                                                        
	}                                                        
	char *buffer = (char*)audio_buf;
	char codec_name[ITEM_MAX_LEN];
	if (!item_exist("codec.model")) {           
		printk("No codec machine\n");           
		return -1;                         
	}                                           
	//	pcbtest_audio_total = (1920000 + 4096)>>3;
	item_string(codec_name, "codec.model", 0);  
	 
	imapx800_i2s.regs = ioremap(IMAP_IIS0_BASE, SZ_4K);

	writel(0x0,imapx800_i2s.regs + rIER);  
	writel(0x0,imapx800_i2s.regs + rITER); 
	writel(0x0,imapx800_i2s.regs + rTER0); 
	writel(0x0,imapx800_i2s.regs + rIRER); 
	writel(0x0,imapx800_i2s.regs + rRER0); 
	writel(0x0,imapx800_i2s.regs + rCER);  

	int i=0;
	for (i = 0;i < ARRAY_SIZE(audios);i++) {                          
		//printk("codec %s ========== %s\n", audios[i].name, codec_name);                                   

		if (!strcmp(audios[i].name, codec_name)) {                    
			int ret = pcbtest_init_iis(1);                                 
			if (ret < 0) {                                            
				printk("Init iis err\n");                             
				return -1;                                            
			}                                                         
			if (audios[i].record_init() < 0) {                               
				printk("Init codec err\n");                           
				return -1;                                            
			}                                                         
			break;                                                 
		}                                                             
	}                                                                 

	/* TODO, 0x*/
	volatile unsigned int tmp = 0;	
	unsigned short * pBuffer;
	int fileLen, len;

	i = 0;
	pBuffer = (unsigned short *)buffer;


	writel(((0x3<<4)|(0x3)),imapx800_i2s.regs +rIMR0);
	writel(0x0,imapx800_i2s.regs +rCCR);
	writel(0x2,imapx800_i2s.regs +rRCR0);
	writel(0x8,imapx800_i2s.regs +rRFCR0);

	writel(0x1,imapx800_i2s.regs +rIER);
	writel(0x1,imapx800_i2s.regs +rIRER);
	writel(0x1,imapx800_i2s.regs +rRER0);
	writel(0x1,imapx800_i2s.regs +rCER);
	
	unsigned long to = jiffies + 2*HZ;
	for(i=0;i < AUDIO_BUFFER_LEN &&  jiffies < to;)
	{

		tmp = readl(imapx800_i2s.regs +rISR0);
		if(tmp & (0x1<<1))
		{
			readl(imapx800_i2s.regs +rROR0); //rx fifo overrun
			readl(imapx800_i2s.regs +rISR0);
		}
		if(tmp & 0x1)
		{

			*pBuffer= readl(imapx800_i2s.regs + rLRBR0);
			//printk("0x%x ",*pBuffer);
			//	if(*pBuffer==0) *pBuffer =(pBuffer&0x11)<<8; 
			pBuffer++;
			*pBuffer = readl(imapx800_i2s.regs + rRRBR0);
			//	if(*pBuffer==0) *pBuffer =(pBuffer&0x11)<<8;
			pBuffer++;
			i++;
		}    
	}
	len = i;
	fileLen = len<<2;
	pcbtest_audio_total = fileLen;

	printk(KERN_ERR"record ok\n");
	return audio_buf;

}

