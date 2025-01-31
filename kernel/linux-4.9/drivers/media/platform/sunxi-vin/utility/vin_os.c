/*
 * linux-4.9/drivers/media/platform/sunxi-vin/utility/vin_os.c
 *
 * Copyright (c) 2007-2017 Allwinnertech Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include "vin_os.h"

unsigned int vin_log_mask;
EXPORT_SYMBOL_GPL(vin_log_mask);

int os_gpio_set(struct gpio_config *gc)
{
#ifndef FPGA_VER
	char pin_name[32];
	__u32 config;

	if (gc == NULL)
		return -1;
	if (gc->gpio == GPIO_INDEX_INVALID)
		return -1;

	if (!IS_AXP_PIN(gc->gpio)) {
		/* valid pin of sunxi-pinctrl,
		 * config pin attributes individually.
		 */
		sunxi_gpio_to_name(gc->gpio, pin_name);
		config =
		    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, gc->mul_sel);
		pin_config_set(SUNXI_PINCTRL, pin_name, config);
		if (gc->pull != GPIO_PULL_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, gc->pull);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
		if (gc->drv_level != GPIO_DRVLVL_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,
					      gc->drv_level);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
		if (gc->data != GPIO_DATA_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, gc->data);
			pin_config_set(SUNXI_PINCTRL, pin_name, config);
		}
	} else {
		/* valid pin of axp-pinctrl,
		 * config pin attributes individually.
		 */
		sunxi_gpio_to_name(gc->gpio, pin_name);
		config =
		    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC, gc->mul_sel);
		pin_config_set(AXP_PINCTRL, pin_name, config);
		if (gc->data != GPIO_DATA_DEFAULT) {
			config =
			    SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT, gc->data);
			pin_config_set(AXP_PINCTRL, pin_name, config);
		}
	}
#endif
	return 0;
}
EXPORT_SYMBOL_GPL(os_gpio_set);

int os_gpio_write(u32 gpio, __u32 out_value, int force_value_flag)
{
#ifndef FPGA_VER
	if (gpio == GPIO_INDEX_INVALID)
		return 0;

	if (force_value_flag == 1) {
		gpio_direction_output(gpio, 0);
		__gpio_set_value(gpio, out_value);
	} else {
		if (out_value == 0) {
			gpio_direction_output(gpio, 0);
			__gpio_set_value(gpio, out_value);
		} else {
			gpio_direction_input(gpio);
		}
	}
#endif
	return 0;
}
EXPORT_SYMBOL_GPL(os_gpio_write);

int os_mem_alloc(struct device *dev, struct vin_mm *mem_man)
{
#ifdef SUNXI_MEM
	int ret = -1;
	char *ion_name = "ion_vin";
	mem_man->client = sunxi_ion_client_create(ion_name);

	if (IS_ERR_OR_NULL(mem_man->client)) {
		vin_err("sunxi_ion_client_create failed!!");
		goto err_client;
	}
#if defined CONFIG_SUNXI_IOMMU && defined CONFIG_VIN_IOMMU
	/* IOMMU */
	mem_man->handle = ion_alloc(mem_man->client, mem_man->size, PAGE_SIZE,
					ION_HEAP_SYSTEM_MASK, 0);
#else
	/* CMA or CARVEOUT */
	mem_man->handle = ion_alloc(mem_man->client, mem_man->size, PAGE_SIZE,
				ION_HEAP_TYPE_DMA_MASK |
				ION_HEAP_CARVEOUT_MASK, 0);

#endif
	if (IS_ERR_OR_NULL(mem_man->handle)) {
		vin_err("ion_alloc failed!!");
		goto err_alloc;
	}
	mem_man->vir_addr = ion_map_kernel(mem_man->client, mem_man->handle);
	if (IS_ERR_OR_NULL(mem_man->vir_addr)) {
		vin_err("ion_map_kernel failed!!");
		goto err_map_kernel;
	}

#if defined CONFIG_SUNXI_IOMMU && defined CONFIG_VIN_IOMMU
	/* IOMMU */
	ret = dma_map_sg_attrs(get_device(dev), mem_man->handle->buffer->sg_table->sgl,
				mem_man->handle->buffer->sg_table->nents, DMA_BIDIRECTIONAL,
				DMA_ATTR_SKIP_CPU_SYNC);
	if (ret != 1) {
		vin_err("dma map sg fail!!\n");
		goto err_phys;
	}
	mem_man->dma_addr = (void *)sg_dma_address(mem_man->handle->buffer->sg_table->sgl);
#else
	/* CMA or CARVEOUT */
	ret = ion_phys(mem_man->client, mem_man->handle,
			 (ion_phys_addr_t *)&mem_man->phy_addr, &mem_man->size);
	if (ret) {
		vin_err("ion_phys failed!!");
		goto err_phys;
	}
	mem_man->dma_addr = mem_man->phy_addr;
#endif
	return 0;
err_phys:
#if defined CONFIG_SUNXI_IOMMU && defined CONFIG_VIN_IOMMU
	dma_unmap_sg_attrs(get_device(dev), mem_man->handle->buffer->sg_table->sgl, mem_man->handle->buffer->sg_table->nents,
						DMA_FROM_DEVICE, DMA_ATTR_SKIP_CPU_SYNC);
#endif
	ion_unmap_kernel(mem_man->client, mem_man->handle);
err_map_kernel:
	ion_free(mem_man->client, mem_man->handle);
err_alloc:
	ion_client_destroy(mem_man->client);
err_client:
	return -1;
#else
	mem_man->vir_addr = dma_alloc_coherent(dev, (size_t) mem_man->size,
					(dma_addr_t *)&mem_man->phy_addr,
					GFP_KERNEL);
	if (!mem_man->vir_addr) {
		vin_err("dma_alloc_coherent memory alloc failed\n");
		return -ENOMEM;
	}
	mem_man->dma_addr = mem_man->phy_addr;
	return 0;
#endif
}
EXPORT_SYMBOL_GPL(os_mem_alloc);

void os_mem_free(struct device *dev, struct vin_mm *mem_man)
{
#ifdef SUNXI_MEM
	if (IS_ERR_OR_NULL(mem_man->client) || IS_ERR_OR_NULL(mem_man->handle)
		|| IS_ERR_OR_NULL(mem_man->vir_addr))
		return;
#if defined CONFIG_SUNXI_IOMMU && defined CONFIG_VIN_IOMMU
	dma_unmap_sg_attrs(get_device(dev), mem_man->handle->buffer->sg_table->sgl, mem_man->handle->buffer->sg_table->nents,
					DMA_FROM_DEVICE, DMA_ATTR_SKIP_CPU_SYNC);
#endif
	ion_unmap_kernel(mem_man->client, mem_man->handle);
	ion_free(mem_man->client, mem_man->handle);
	ion_client_destroy(mem_man->client);
#else
	if (mem_man->vir_addr)
		dma_free_coherent(dev, mem_man->size, mem_man->vir_addr,
				  (dma_addr_t) mem_man->phy_addr);
#endif
	mem_man->phy_addr = NULL;
	mem_man->dma_addr = NULL;
	mem_man->vir_addr = NULL;
}
EXPORT_SYMBOL_GPL(os_mem_free);

extern void sunxi_enable_device_iommu(unsigned int master_id, bool flag);
void vin_iommu_en(unsigned int mester_id, bool en)
{
#if defined CONFIG_SUNXI_IOMMU && defined CONFIG_VIN_IOMMU
	sunxi_enable_device_iommu(mester_id, en);
#endif
}
EXPORT_SYMBOL_GPL(vin_iommu_en);

MODULE_AUTHOR("raymonxiu");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Video front end OSAL for sunxi");
