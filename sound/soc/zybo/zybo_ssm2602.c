/*
 *  Copyright (C) 2015
 *	Author: Ferdinand Stehle <development@kondorgulasch.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include "../codecs/ssm2602.h"

static const struct snd_soc_dapm_widget zybo_ssm2602_widgets[] = {
	SND_SOC_DAPM_SPK("Line Out", NULL),
	SND_SOC_DAPM_HP("Headphone Out", NULL),
	SND_SOC_DAPM_MIC("Mic In", NULL),
	SND_SOC_DAPM_MIC("Line In", NULL),
};

static const struct snd_soc_dapm_route zybo_ssm2602_routes[] = {
	{ "Line Out", NULL, "LOUT" },
	{ "Line Out", NULL, "ROUT" },
	{ "LLINEIN", NULL, "Line In" },
	{ "RLINEIN", NULL, "Line In" },
};

static int zybo_ssm2602_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int pll_rate;
	int ret;

	switch (params_rate(params)) {
	case 48000:
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 96000:
		pll_rate = 48000 * 1024;
		break;
	case 44100:
	case 7350:
	case 11025:
	case 14700:
	case 22050:
	case 29400:
	case 88200:
		pll_rate = 44100 * 1024;
		break;
	default:
		return -EINVAL;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, SSM2602_SYSCLK, 12000000,
		SND_SOC_CLOCK_IN);

	return ret;
}

static struct snd_soc_ops zybo_ssm2602_ops = {
	.hw_params = zybo_ssm2602_hw_params,
};

static struct snd_soc_dai_link zybo_ssm2602_dai_link = {
	.name = "ssm2602",
	.stream_name = "ssm2602",
	.codec_dai_name = "ssm2602-hifi",
	.dai_fmt = SND_SOC_DAIFMT_I2S |
			SND_SOC_DAIFMT_NB_NF |
			SND_SOC_DAIFMT_CBS_CFS,
	.ops = &zybo_ssm2602_ops,
};

static struct snd_soc_card zybo_ssm2602_card = {
	.name = "ZYBO SSM2602",
	.owner = THIS_MODULE,
	.dai_link = &zybo_ssm2602_dai_link,
	.num_links = 1,
	.dapm_widgets = zybo_ssm2602_widgets,
	.num_dapm_widgets = ARRAY_SIZE(zybo_ssm2602_widgets),
	.dapm_routes = zybo_ssm2602_routes,
	.num_dapm_routes = ARRAY_SIZE(zybo_ssm2602_routes),
	.fully_routed = true,
};

static int zybo_ssm2602_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &zybo_ssm2602_card;
	struct device_node *of_node = pdev->dev.of_node;

	if (!of_node)
		return -ENXIO;

	card->dev = &pdev->dev;

	zybo_ssm2602_dai_link.codec_of_node = of_parse_phandle(of_node, "audio-codec", 0);
	zybo_ssm2602_dai_link.cpu_of_node = of_parse_phandle(of_node, "cpu-dai", 0);
	zybo_ssm2602_dai_link.platform_of_node = zybo_ssm2602_dai_link.cpu_of_node;

	if (!zybo_ssm2602_dai_link.codec_of_node ||
		!zybo_ssm2602_dai_link.cpu_of_node)
		return -ENXIO;

	return snd_soc_register_card(card);
}

static int zybo_ssm2602_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id zybo_ssm2602_of_match[] = {
	{ .compatible = "zybo-ssm2602-snd", },
	{},
};
MODULE_DEVICE_TABLE(of, zybo_ssm2602_of_match);

static struct platform_driver zybo_ssm2602_card_driver = {
	.driver = {
		.name = "zybo-ssm2602-snd",
		.owner = THIS_MODULE,
		.of_match_table = zybo_ssm2602_of_match,
		.pm = &snd_soc_pm_ops,
	},
	.probe = zybo_ssm2602_probe,
	.remove = zybo_ssm2602_remove,
};
module_platform_driver(zybo_ssm2602_card_driver);

MODULE_DESCRIPTION("Zybo Zynq ssm2602 driver");
MODULE_AUTHOR("Ferdinand Stehle <development@kondorgulasch.de>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:zybo-ssm2602-snd");
