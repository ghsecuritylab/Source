#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

static struct platform_device ralink_pcm_device = {
	.name		= "mt7620-pcm",
	.id		= -1,
};

int __init ralink_pcm_register(void)
{
	platform_device_register(&ralink_pcm_device);
	return 0;
}
arch_initcall(ralink_pcm_register);
