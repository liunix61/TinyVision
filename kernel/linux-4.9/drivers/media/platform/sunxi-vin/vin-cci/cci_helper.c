/*
 * linux-4.9/drivers/media/platform/sunxi-vin/vin-cci/cci_helper.c
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
#include "cci_helper.h"
#include "../platform/platform_cfg.h"
#include "../modules/sensor/sensor_helper.h"
#include "../modules/sensor/camera.h"

#if defined(CONFIG_CCI_MODULE) || defined(CONFIG_CCI)
#include "bsp_cci.h"
#define USE_SPECIFIC_CCI
#endif

#ifdef USE_SPECIFIC_CCI
#define cci_print(x, arg...) pr_info("[VIN_DEV_CCI]"x, ##arg)
#define cci_err(x, arg...) pr_err("[VIN_DEV_CCI_ERR]"x, ##arg)
#else
#define cci_print(x, arg...) pr_info("[VIN_DEV_I2C]"x, ##arg)
#define cci_err(x, arg...) pr_err("[VIN_DEV_I2C_ERR]"x, ##arg)
#endif

static void cci_device_release(struct device *dev)
{

}
struct device cci_device_def = {
	.release = cci_device_release,
};

#define SHOW_CCI_DEVICE_ATTR(name) \
static ssize_t cci_device_##name##_show(struct device *dev, \
			       struct device_attribute *attr, \
			       char *buf) \
{\
	struct cci_driver *cci_drv = dev_get_drvdata(dev); \
	cci_print("Get val = 0x%x !\n", cci_drv->name);\
	return sprintf(buf, "0x%x\n", cci_drv->name); \
}

#define STORE_CCI_DEVICE_ATTR(name) \
static ssize_t cci_device_##name##_store(struct device *dev, \
			       struct device_attribute *attr, \
			       const char *buf,\
			       size_t count) \
{\
	struct cci_driver *cci_drv = dev_get_drvdata(dev); \
	unsigned int val;\
	val = simple_strtoul(buf, NULL, 16);\
	cci_drv->name = val;\
	cci_print("Set val = 0x%x !\n", val);\
	return count;\
}

SHOW_CCI_DEVICE_ATTR(addr_width)
SHOW_CCI_DEVICE_ATTR(data_width)
SHOW_CCI_DEVICE_ATTR(read_value)
SHOW_CCI_DEVICE_ATTR(read_flag)
STORE_CCI_DEVICE_ATTR(addr_width)
STORE_CCI_DEVICE_ATTR(data_width)
STORE_CCI_DEVICE_ATTR(read_flag)

static ssize_t cci_sys_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct cci_driver *cci_drv = dev_get_drvdata(dev);

	cci_print("echo reg(16)|val(16) > cci_client,eg: echo 30350011 > cci_client!\n");
	cci_print("Note: current  read_flag = %d\n", cci_drv->read_flag);
	return sprintf(buf, "0x%x\n", cci_drv->read_flag);
}

static ssize_t cci_sys_store(struct device *dev,
			     struct device_attribute *attr, const char *buf,
			     size_t count)
{
	static int val;
	unsigned short reg, value;
	struct cci_driver *cci_drv = dev_get_drvdata(dev);
	struct v4l2_subdev *sd = cci_drv->sd;

	val = simple_strtoul(buf, NULL, 16);
	reg = (val >> 16) & 0xFFFF;
	value = val & 0xFFFF;
	if (cci_drv->read_flag == 0) {
		cci_write(sd, (addr_type) reg, (addr_type) value);
		cci_print("Write reg = 0x%x, write value = 0x%x\n", reg, value);
	} else {
		cci_read(sd, (addr_type) reg,
			 (addr_type *)&cci_drv->read_value);
		cci_print("Read reg = 0x%x, read value = 0x%x\n", reg,
			  cci_drv->read_value);
	}
	return count;
}

static struct device_attribute cci_device_attrs[] = {
	__ATTR(addr_width, S_IWUSR | S_IRUGO, cci_device_addr_width_show,
	       cci_device_addr_width_store),
	__ATTR(data_width, S_IWUSR | S_IRUGO, cci_device_data_width_show,
	       cci_device_data_width_store),
	__ATTR(read_value, S_IRUGO, cci_device_read_value_show, NULL),
	__ATTR(read_flag, S_IWUSR | S_IRUGO, cci_device_read_flag_show,
	       cci_device_read_flag_store),
	__ATTR(cci_client, S_IWUSR | S_IRUGO, cci_sys_show, cci_sys_store),
};
static int cci_sys_register(struct cci_driver *drv_data)
{
	int i, ret;

	drv_data->cci_device = cci_device_def;
	dev_set_name(&drv_data->cci_device, drv_data->name);
	if (device_register(&drv_data->cci_device)) {
		cci_err("error device_register()\n");
		return -1;
	}
	dev_set_drvdata(&drv_data->cci_device, drv_data);
	/* sysfs entries */
	for (i = 0; i < ARRAY_SIZE(cci_device_attrs); i++) {
		ret =
		    device_create_file(&drv_data->cci_device,
				       &cci_device_attrs[i]);
		if (ret) {
			cci_err("device_create_file error\n");
			device_remove_file(&drv_data->cci_device,
					   &drv_data->dev_attr_cci);
		}
	}
	return 0;
}
static int cci_sys_unregister(struct cci_driver *drv_data)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cci_device_attrs); i++)
		device_remove_file(&drv_data->cci_device, &cci_device_attrs[i]);
	device_unregister(&drv_data->cci_device);
	return 0;
}

static int sensor_registered(struct v4l2_subdev *sd)
{
#if !defined CONFIG_VIN_INIT_MELIS
	int ret;
	struct sensor_info *info = to_state(sd);

	mutex_lock(&info->lock);

#if !defined SENSOR_POER_BEFORE_VIN
	v4l2_subdev_call(sd, core, s_power, PWR_ON);
#endif
	sd->entity.use_count++;
	ret = v4l2_subdev_call(sd, core, init, 0);
	sd->entity.use_count--;
#if !defined SENSOR_POER_BEFORE_VIN
	v4l2_subdev_call(sd, core, s_power, PWR_OFF);
#endif

	mutex_unlock(&info->lock);

	return ret;
#else
	return 0;
#endif
}

static const struct v4l2_subdev_internal_ops sensor_internal_ops = {
	.registered = sensor_registered,
};

static LIST_HEAD(cci_drv_list);
static int cci_dev_num_id;

static void cci_subdev_init(struct v4l2_subdev *sd, struct cci_driver *drv_data,
		     const struct v4l2_subdev_ops *ops)
{
	v4l2_subdev_init(sd, ops);
	/* initialize name */
	snprintf(sd->name, sizeof(sd->name), "%s", drv_data->name);
	drv_data->sd = sd;
	drv_data->id = cci_dev_num_id;
	drv_data->is_registerd = 1;
	v4l2_set_subdevdata(sd, drv_data);

	list_add(&drv_data->cci_list, &cci_drv_list);
	cci_dev_num_id++;
}

static void cci_subdev_remove(struct cci_driver *cci_drv_p)
{
	if (cci_drv_p->is_registerd == 0) {
		cci_err("CCI subdev exit: this device is not registerd!\n");
		return;
	} else {
		cci_drv_p->is_matched = 0;
		cci_drv_p->is_registerd = 0;
		if (cci_drv_p->sd == NULL)
			cci_err("CCI subdev exit: cci_drv_p->sd is NULL!\n");
		else
			v4l2_set_subdevdata(cci_drv_p->sd, NULL);

		list_del(&cci_drv_p->cci_list);
		cci_dev_num_id--;
	}
}

struct v4l2_subdev *cci_bus_match(char *name, unsigned short cci_id,
				  unsigned short cci_saddr)
{
	struct cci_driver *cci_drv;

	list_for_each_entry(cci_drv, &cci_drv_list, cci_list) {
		if (cci_drv->is_registerd == 1 && cci_drv->is_matched == 0) {
			if (!strcmp(cci_drv->name, name)) {
				cci_drv->cci_id = cci_id;
				cci_drv->cci_saddr = cci_saddr;
				cci_drv->is_matched = 1;
				return cci_drv->sd;
			}
		}
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(cci_bus_match);

void cci_bus_match_cancel(struct cci_driver *cci_drv_p)
{
	if (cci_drv_p->is_registerd == 0) {
		cci_err("This device is not registerd!\n");
		return;
	} else {
		if (cci_drv_p->is_matched == 0) {
			cci_err("This device has not been matched!\n");
			return;
		} else {
			cci_drv_p->is_matched = 0;
			cci_drv_p->cci_id = 0;
			cci_drv_p->cci_saddr = 0;
		}
	}
}
EXPORT_SYMBOL_GPL(cci_bus_match_cancel);

void csi_cci_init_helper(unsigned int sel)
{
#ifdef USE_SPECIFIC_CCI
	cci_s_power(sel, 1);
#endif
}
EXPORT_SYMBOL_GPL(csi_cci_init_helper);

void csi_cci_exit_helper(unsigned int sel)
{
#ifdef USE_SPECIFIC_CCI
	cci_s_power(sel, 0);
#endif
}
EXPORT_SYMBOL_GPL(csi_cci_exit_helper);

void cci_lock(struct v4l2_subdev *sd)
{
#ifdef USE_SPECIFIC_CCI

#else
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	i2c_lock_adapter(client->adapter);
#endif
}
EXPORT_SYMBOL_GPL(cci_lock);

void cci_unlock(struct v4l2_subdev *sd)
{
#ifdef USE_SPECIFIC_CCI

#else
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	i2c_unlock_adapter(client->adapter);
#endif
}
EXPORT_SYMBOL_GPL(cci_unlock);

int cci_dev_init_helper(struct i2c_driver *sensor_driver)
{
#ifdef USE_SPECIFIC_CCI
	return sensor_driver->probe(NULL, NULL);
#else
	return i2c_add_driver(sensor_driver);
#endif

}
EXPORT_SYMBOL_GPL(cci_dev_init_helper);

void cci_dev_exit_helper(struct i2c_driver *sensor_driver)
{
#ifdef USE_SPECIFIC_CCI
	sensor_driver->remove(NULL);
#else
	i2c_del_driver(sensor_driver);
#endif
}
EXPORT_SYMBOL_GPL(cci_dev_exit_helper);

static int cci_media_entity_init_helper(struct v4l2_subdev *sd,
					struct cci_driver *cci_drv)
{
	struct sensor_info *si;

	switch (cci_drv->type) {
	case CCI_TYPE_SENSOR:
		si = container_of(sd, struct sensor_info, sd);
		BUG_ON(si->magic_num && (si->magic_num != SENSOR_MAGIC_NUMBER));
		si->sensor_pads[SENSOR_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;
		sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
		return media_entity_pads_init(&sd->entity, SENSOR_PAD_NUM, si->sensor_pads);
	case CCI_TYPE_ACT:
		sd->entity.function = MEDIA_ENT_F_LENS;
		return media_entity_pads_init(&sd->entity, 0, NULL);
	case CCI_TYPE_FLASH:
		sd->entity.function = MEDIA_ENT_F_FLASH;
		return media_entity_pads_init(&sd->entity, 0, NULL);
	default:
		return media_entity_pads_init(&sd->entity, 0, NULL);
	}
}

int cci_dev_probe_helper(struct v4l2_subdev *sd, struct i2c_client *client,
			 const struct v4l2_subdev_ops *sensor_ops,
			 struct cci_driver *cci_drv)
{
	if (client) {
		v4l2_i2c_subdev_init(sd, client, sensor_ops);
		/*change sd name to sensor driver name*/
		snprintf(sd->name, sizeof(sd->name), "%s", cci_drv->name);
		cci_drv->sd = sd;
	} else {
		cci_subdev_init(sd, cci_drv, sensor_ops);
	}
	v4l2_set_subdev_hostdata(sd, cci_drv);
	sd->grp_id = VIN_GRP_ID_SENSOR;
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	if (cci_drv->type != CCI_TYPE_ACT)
		sd->internal_ops = &sensor_internal_ops;
	cci_sys_register(cci_drv);
	cci_media_entity_init_helper(sd, cci_drv);

	return 0;
}
EXPORT_SYMBOL_GPL(cci_dev_probe_helper);

struct v4l2_subdev *cci_dev_remove_helper(struct i2c_client *client,
					  struct cci_driver *cci_drv)
{
	struct v4l2_subdev *sd;

	if (client)
		sd = i2c_get_clientdata(client);
	else {
		sd = cci_drv->sd;
		cci_subdev_remove(cci_drv);
	}
	if (sd->entity.use_count > 0) {
		sd->entity.use_count = 0;
		sd->entity.stream_count = 0;
		vin_err("%s is in using, cannot be removed!\n", sd->name);
		v4l2_subdev_call(sd, core, s_power, PWR_OFF);
	}
	v4l2_set_subdev_hostdata(sd, NULL);
	v4l2_device_unregister_subdev(sd);
	cci_sys_unregister(cci_drv);
	return sd;
}
EXPORT_SYMBOL_GPL(cci_dev_remove_helper);

/*
 * On most platforms, we'd rather do straight i2c I/O.
 */

int cci_read_a8_d8(struct v4l2_subdev *sd, unsigned char addr,
		   unsigned char *value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_rd_8_8(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else
	unsigned char data[2];
	struct i2c_msg msg[2];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = addr;
	data[1] = 0xee;
	/*
	 * Send out the register address...
	 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &data[0];
	/*
	 * ...then read back the result.
	 */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &data[1];

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*value = data[1];
		ret = 0;
	}
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_read_a8_d8);

int cci_write_a8_d8(struct v4l2_subdev *sd, unsigned char addr,
		    unsigned char value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_wr_8_8(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else
	struct i2c_msg msg;
	unsigned char data[2];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = addr;
	data[1] = value;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0)
		ret = 0;
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_write_a8_d8);

int cci_read_a8_d16(struct v4l2_subdev *sd, unsigned char addr,
		    unsigned short *value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_rd_8_16(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else

	unsigned char data[3];
	struct i2c_msg msg[2];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = addr;
	data[1] = 0xee;
	data[2] = 0xee;
	/*
	 * Send out the register address...
	 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &data[0];
	/*
	 * ...then read back the result.
	 */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = &data[1];

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*value = data[1] * 256 + data[2];
		ret = 0;
	}
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_read_a8_d16);

int cci_write_a8_d16(struct v4l2_subdev *sd, unsigned char addr,
		     unsigned short value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_wr_8_16(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else

	struct i2c_msg msg;
	unsigned char data[3];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = addr;
	data[1] = (value & 0xff00) >> 8;
	data[2] = (value & 0x00ff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0)
		ret = 0;
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_write_a8_d16);

int cci_read_a16_d8(struct v4l2_subdev *sd, unsigned short addr,
		    unsigned char *value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_rd_16_8(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else
	int ret;
	unsigned char data[3];
	struct i2c_msg msg[2];
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = (addr & 0xff00) >> 8;
	data[1] = (addr & 0x00ff);
	data[2] = 0xee;
	/*
	 * Send out the register address...
	 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = &data[0];
	/*
	 * ...then read back the result.
	 */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &data[2];

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*value = data[2];
		ret = 0;
	}
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_read_a16_d8);

int cci_write_a16_d8(struct v4l2_subdev *sd, unsigned short addr,
		     unsigned char value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_wr_16_8(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else
	int ret = 0;
	struct i2c_msg msg;
	unsigned char data[3];
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = (addr & 0xff00) >> 8;
	data[1] = (addr & 0x00ff);
	data[2] = value;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0)
		ret = 0;
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_write_a16_d8);

int cci_read_a16_d16(struct v4l2_subdev *sd, unsigned short addr,
		     unsigned short *value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_rd_16_16(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else

	unsigned char data[4];
	struct i2c_msg msg[2];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = (addr & 0xff00) >> 8;
	data[1] = (addr & 0x00ff);
	data[2] = 0xee;
	data[3] = 0xee;
	/*
	 * Send out the register address...
	 */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = &data[0];
	/*
	 * ...then read back the result.
	 */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = &data[2];

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*value = data[2] * 256 + data[3];
		ret = 0;
	}
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_read_a16_d16);

int cci_write_a16_d16(struct v4l2_subdev *sd, unsigned short addr,
		      unsigned short value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_wr_16_16(cci_drv->cci_id, addr, value, cci_drv->cci_saddr);
#else

	struct i2c_msg msg;
	unsigned char data[4];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = (addr & 0xff00) >> 8;
	data[1] = (addr & 0x00ff);
	data[2] = (value & 0xff00) >> 8;
	data[3] = (value & 0x00ff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0)
		ret = 0;
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_write_a16_d16);

int cci_read_a0_d16(struct v4l2_subdev *sd, unsigned short *value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_rd_0_16(cci_drv->cci_id, value, cci_drv->cci_saddr);
#else

	struct i2c_msg msg;
	unsigned char data[2];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = 0xee;
	data[1] = 0xee;

	msg.addr = client->addr;
	msg.flags = 1;
	msg.len = 2;
	msg.buf = &data[0];

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0) {
		*value = data[0] * 256 + data[1];
		ret = 0;
	}
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_read_a0_d16);

int cci_write_a0_d16(struct v4l2_subdev *sd, unsigned short value)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_wr_0_16(cci_drv->cci_id, value, cci_drv->cci_saddr);
#else

	struct i2c_msg msg;
	unsigned char data[2];
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	data[0] = (value & 0xff00) >> 8;
	data[1] = (value & 0x00ff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0)
		ret = 0;
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_write_a0_d16);

int cci_write_a16_d8_continuous_helper(struct v4l2_subdev *sd,
				       unsigned short addr, unsigned char *vals,
				       uint size)
{
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);

	return cci_wr_a16_d8_continuous(cci_drv->cci_id, addr, vals, cci_drv->cci_saddr, size);
#else
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char data[2 + 32];
	unsigned char *p = vals;
	int ret, i, len;

	while (size > 0) {
		len = size > 32 ? 32 : size;
		data[0] = (addr & 0xff00) >> 8;
		data[1] = (addr & 0x00ff);

		for (i = 2; i < 2 + len; i++)
			data[i] = *p++;

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 2 + len;
		msg.buf = data;

		ret = i2c_transfer(client->adapter, &msg, 1);

		if (ret > 0) {
			ret = 0;
		} else if (ret < 0) {
			cci_err("sensor_write error!\n");
			break;
		}
		addr += len;
		size -= len;
	}
	return ret;
#endif
}
EXPORT_SYMBOL_GPL(cci_write_a16_d8_continuous_helper);

#if 0
#define MAX_TWI_DEPTH 127
int twi_write_ndata_by_dma(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg *msg = kzalloc(MAX_TWI_DEPTH * sizeof(struct i2c_msg), GFP_KERNEL);
	unsigned char data[MAX_TWI_DEPTH][4];
	int ret = 0, i, len, valid_len = 0;

	if (!regs || !array_size)
		return -EINVAL;

	while (array_size > 0) {
		len = array_size > MAX_TWI_DEPTH ? MAX_TWI_DEPTH : array_size;
		valid_len = 0;
		for (i = 0; i < len; i++) {
			if (regs->addr == REG_DLY) {
				usleep_range(regs->data * 1000, regs->data * 1000 + 100);
				regs++;
				array_size--;
				break;
			}
			if (8 == cci_drv->addr_width && 8 == cci_drv->data_width) {
				data[valid_len][0] = regs->addr;
				data[valid_len][1] = regs->data;
				msg[valid_len].len = 2;
			} else if (8 == cci_drv->addr_width && 16 == cci_drv->data_width) {
				data[valid_len][0] = regs->addr;
				data[valid_len][1] = (regs->data & 0xff00) >> 8;
				data[valid_len][2] = (regs->data & 0x00ff);
				msg[valid_len].len = 3;
			} else if (16 == cci_drv->addr_width && 8 == cci_drv->data_width) {
				data[valid_len][0] = (regs->addr & 0xff00) >> 8;
				data[valid_len][1] = (regs->addr & 0x00ff);
				data[valid_len][2] = regs->data;
				msg[valid_len].len = 3;
			} else if (16 == cci_drv->addr_width && 16 == cci_drv->data_width) {
				data[valid_len][0] = (regs->addr & 0xff00) >> 8;
				data[valid_len][1] = (regs->addr & 0x00ff);
				data[valid_len][2] = (regs->data & 0xff00) >> 8;
				data[valid_len][3] = (regs->data & 0x00ff);
				msg[valid_len].len = 4;
			} else {
				kfree(msg);
				return -1;
			}

			msg[valid_len].addr = client->addr;
			msg[valid_len].flags = 0;
			msg[valid_len].buf = &data[valid_len][0];
			valid_len++;

			regs++;
			array_size--;
		}
		if (valid_len == 0)
			continue;

		usleep_range(2000, 3000);
		cci_print("%s: write %d i2c_msg one time!\n", __func__, valid_len);

		ret = i2c_transfer(client->adapter, msg, valid_len);
		if (ret >= 0)
			ret = 0;
		else
			break;
	}

	kfree(msg);

	return ret;
}
#endif

int cci_write(struct v4l2_subdev *sd, addr_type addr, data_type value)
{
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
	int addr_width = cci_drv->addr_width;
	int data_width = cci_drv->data_width;

	if (8 == addr_width && 8 == data_width) {
		return cci_write_a8_d8(sd, (unsigned char)addr, (unsigned char)value);
	} else if (8 == addr_width && 16 == data_width) {
		return cci_write_a8_d16(sd, (unsigned char)addr, value);
	} else if (16 == addr_width && 8 == data_width) {
		return cci_write_a16_d8(sd, addr, (unsigned char)value);
	} else if (16 == addr_width && 16 == data_width) {
		return cci_write_a16_d16(sd, addr, value);
	} else if (0 == addr_width && 16 == data_width) {
		return cci_write_a0_d16(sd, value);
	} else {
		cci_err("at %s error, addr_width = %d , data_width = %d!\n ", __func__, addr_width, data_width);
		return -1;
	}
}
EXPORT_SYMBOL_GPL(cci_write);

int cci_read(struct v4l2_subdev *sd, addr_type addr, data_type *value)
{
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
	int addr_width = cci_drv->addr_width;
	int data_width = cci_drv->data_width;

	*value = 0;
	if (8 == addr_width && 8 == data_width) {
		return cci_read_a8_d8(sd, (unsigned char)addr, (unsigned char *)value);
	} else if (8 == addr_width && 16 == data_width) {
		return cci_read_a8_d16(sd, (unsigned char)addr, value);
	} else if (16 == addr_width && 8 == data_width) {
		return cci_read_a16_d8(sd, addr, (unsigned char *)value);
	} else if (16 == addr_width && 16 == data_width) {
		return cci_read_a16_d16(sd, addr, value);
	} else if (0 == addr_width && 16 == data_width) {
		return cci_read_a0_d16(sd, value);
	} else {
		cci_err("%s error! addr_width = %d , data_width = %d!\n ", __func__, addr_width, data_width);
		return -1;
	}
}
EXPORT_SYMBOL_GPL(cci_read);

/*
 * On most platforms, we'd rather do straight i2c I/O.
 */
int sensor_read(struct v4l2_subdev *sd, addr_type reg, data_type *value)
{
	int ret = 0, cnt = 0;

	if (!sd || !sd->entity.use_count) {
		cci_err("%s error! sensor is not used!\n", __func__);
		return -1;
	}

	ret = cci_read(sd, reg, value);
	while ((ret != 0) && (cnt < 2)) {
		ret = cci_read(sd, reg, value);
		cnt++;
	}
	if (cnt > 0)
		cci_print("%s read retry %d, read addr is 0x%x\n", sd->name, cnt + 1, reg);

	return ret;
}
EXPORT_SYMBOL_GPL(sensor_read);

int sensor_write(struct v4l2_subdev *sd, addr_type reg, data_type value)
{
	int ret = 0, cnt = 0;

	if (!sd || !sd->entity.use_count) {
		cci_err("%s error! sensor is not used!\n", __func__);
		return -1;
	}

	ret = cci_write(sd, reg, value);
	while ((ret != 0) && (cnt < 2)) {
		ret = cci_write(sd, reg, value);
		cnt++;
	}
	if (cnt > 0)
		cci_print("%s write retry %d, write addr/data is 0x%x/0x%x\n", sd->name, cnt + 1, reg, value);

	return ret;
}
EXPORT_SYMBOL_GPL(sensor_write);

/*
 * Write a list of register settings;
 */
int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
	int ret = 0, i = 0, len = 1;
	unsigned char data[32] = {0};

	if (!regs)
		return -EINVAL;

	while (i < array_size) {
		len = 1;
		if (regs->addr == REG_DLY) {
			usleep_range(regs->data * 1000, regs->data * 1000 + 100);
		} else {
			if (cci_drv->addr_width == CCI_BITS_16 && cci_drv->data_width == CCI_BITS_8) {
				while (regs[len-1].addr == (regs[len].addr - 1)) {
					data[len-1] = regs[len-1].data;
					len++;
					if (len-1 >= 31)
						break;
				}
				data[len-1] = regs[len-1].data;
				ret = cci_write_a16_d8_continuous_helper(sd, regs[0].addr, data, len);
			} else {
#if 0 && defined (CONFIG_ARCH_SUN8IW16P1)
				len = array_size - i;
				ret = twi_write_ndata_by_dma(sd, regs, len);
#else
				ret = sensor_write(sd, regs->addr, regs->data);
#endif
			}
			if (ret < 0) {
				cci_err("%s sensor write array error, array_size %d!\n", sd->name, array_size);
				return -1;
			}
		}
		i += len;
		regs += len;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(sensor_write_array);

/*
 * Read a list of register settings, only using for debug;
 */
int sensor_read_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int ret = 0, i = 0, len = 1;
	data_type data;

	if (!regs)
		return -EINVAL;

	while (i < array_size) {
		len = 1;
		if (regs->addr == REG_DLY) {
			continue;
		} else {
			ret = sensor_read(sd, regs->addr, &data);
			if (ret < 0) {
				cci_err("%s sensor write array error, array_size %d!\n", sd->name, array_size);
				return -1;
			} else {
				cci_print("read from %s addr is 0x%x, data is 0x%x\n", sd->name, regs->addr, data);
			}
		}
		i += len;
		regs += len;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(sensor_read_array);

