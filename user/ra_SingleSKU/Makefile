SKUDIR = $(ROOTDIR)/romfs/ra_SKU

ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN12)),y)
SKU_2G_FCC  = "20160223_FCC_2.4GHz_SingleSKU.dat"
SKU_2G_CE   = "20150313_CE_2.4GHz_SingleSKU.dat"
SKU_2G_APAC =
SKU_2G_NCC  = "20150813_NCC_2.4GHz_SingleSKU.dat"
SKU_5G_FCC  =
SKU_5G_CE   =
SKU_5G_APAC =
SKU_5G_NCC  =
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN53)),y)
SKU_2G_FCC  = "20130808_FCC_2.4GHz_SingleSKU.dat"
SKU_2G_CE   = "20140902_CE_2.4GHz_SingleSKU.dat"
SKU_2G_APAC =
SKU_5G_FCC  =
SKU_5G_CE   =
SKU_5G_APAC =
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN14)),y)
SKU_2G_FCC  = "20131003_FCC_2.4GHz_SingleSKU.dat"
SKU_2G_CE   = "20140902_CE_2.4GHz_SingleSKU.dat"
SKU_2G_APAC =
SKU_5G_FCC  =
SKU_5G_CE   =
SKU_5G_APAC =
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPAC52)),y)
SKU_2G_FCC  = "20150623_FCC_2.4GHz_SingleSKU.dat"
SKU_2G_CE   = "20140226_CE_2.4GHz_SingleSKU.dat"
SKU_2G_APAC =
SKU_2G_AU   = "20140410_AU_2.4GHz_SingleSKU.dat"
SKU_2G_JP   = "20141009_JP_2.4GHz_SingleSKU.dat"
SKU_2G_UA   = "20140226_UA_2.4GHz_SingleSKU.dat"
SKU_2G_IC   = "20151022_IC_2.4GHz_SingleSKU.dat"
SKU_5G_FCC  = "20150614_FCC_5GHz_SingleSKU.dat"
SKU_5G_CE   = "20140915_CE_5GHz_SingleSKU.dat"
SKU_5G_APAC =
SKU_5G_AU   = "20140410_AU_5GHz_SingleSKU.dat"
SKU_5G_JP   = "20141009_JP_5GHz_SingleSKU.dat"
SKU_5G_UA   = "20140915_UA_5GHz_SingleSKU.dat"
SKU_5G_IC   = "20151022_IC_5GHz_SingleSKU.dat"
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_WMPN12)),y)
SKU_2G_FCC  = "20150820_FCC_2.4GHz_SingleSKU.dat"
SKU_5G_FCC  =
endif
endif
endif
endif
endif

all clean:

romfs:
	@install -d $(SKUDIR)
ifneq ($(SKU_2G_FCC),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_FCC) $(SKUDIR)/SingleSKU_FCC.dat
endif
ifneq ($(SKU_2G_CE),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_CE) $(SKUDIR)/SingleSKU_CE.dat
endif
ifneq ($(SKU_2G_APAC),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_APAC) $(SKUDIR)/SingleSKU_APAC.dat
endif
ifneq ($(SKU_2G_AU),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_AU) $(SKUDIR)/SingleSKU_AU.dat
endif
ifneq ($(SKU_2G_JP),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_JP) $(SKUDIR)/SingleSKU_JP.dat
endif
ifneq ($(SKU_2G_NCC),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_NCC) $(SKUDIR)/SingleSKU_NCC.dat
endif
ifneq ($(SKU_2G_UA),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_UA) $(SKUDIR)/SingleSKU_UA.dat
endif
ifneq ($(SKU_2G_IC),)
	@install $(CONFIG_PRODUCT)/$(SKU_2G_IC) $(SKUDIR)/SingleSKU_IC.dat
endif
ifneq ($(SKU_5G_FCC),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_FCC) $(SKUDIR)/SingleSKU_5G_FCC.dat
endif
ifneq ($(SKU_5G_CE),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_CE) $(SKUDIR)/SingleSKU_5G_CE.dat
endif
ifneq ($(SKU_5G_APAC),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_APAC) $(SKUDIR)/SingleSKU_5G_APAC.dat
endif
ifneq ($(SKU_5G_AU),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_AU) $(SKUDIR)/SingleSKU_5G_AU.dat
endif
ifneq ($(SKU_5G_JP),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_JP) $(SKUDIR)/SingleSKU_5G_JP.dat
endif
ifneq ($(SKU_5G_NCC),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_NCC) $(SKUDIR)/SingleSKU_5G_NCC.dat
endif
ifneq ($(SKU_5G_UA),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_UA) $(SKUDIR)/SingleSKU_5G_UA.dat
endif
ifneq ($(SKU_5G_IC),)
	@install $(CONFIG_PRODUCT)/$(SKU_5G_IC) $(SKUDIR)/SingleSKU_5G_IC.dat
endif

.PHONY: all clean romfs
