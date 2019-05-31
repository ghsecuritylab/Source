#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <asm/mach-ralink/rt_mmap.h>

static struct resource i2s_resources[] = {
	{
		.start		= -1, /* filled at runtime */
		.end		= -1, /* filled at runtime */
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device ralink_i2s_device = {
	.name		= "mt7620-i2s",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(i2s_resources),
	.resource	= i2s_resources,
};

int __init ralink_i2s_register(void)
{
	i2s_resources[0].start = RALINK_I2S_BASE;
	i2s_resources[0].end += RALINK_I2S_BASE + 256 - 1;

	platform_device_register(&ralink_i2s_device);
	return 0;
}
arch_initcall(ralink_i2s_register);
