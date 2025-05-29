/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @brief set IO function.
 *
 * @param id -- IO number, reference {@ HalWifiIotIoName}.
 * @param val -- the io function value which defined in {@ hi_io.h}.
 *
 * @return Returns {@link WIFI_IOT_SUCCESS} if the operation is successful;
 *         returns an error code defined in {@link wifiiot_errno.h} otherwise.
 * @since 1.0
 * @version 1.0
 *
unsigned int HalIoSetFunc(HalWifiIotIoName id, const char *val);
*/
/*
 * @brief Initializes an I2C device with a specified baud rate.
 *
 *
 *
 * @param id Indicates the I2C device ID.
 * @param baudrate Indicates the I2C baud rate.
 * @return Returns {@link IOT_SUCCESS} if the I2C device is initialized;
 * returns {@link IOT_FAILURE} otherwise. For details about other return values, see the chip description.
 * @since 2.2
 * @version 2.2
 * 
unsigned int IoTI2cInit(unsigned int id, unsigned int baudrate);
*/

#include "button.h"
#include "common_init.h"
#include "oled.h"
#include "common_log.h"
#include "peripheral_hal.h"

#define I2C_IDX_BAUDRATE                  (400000)
#define WIFI_IOT_OLED_I2C_IDX_0           0
#define WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA 6
#define WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL 6

static void I2cBusInit(void)
{
    IoTGpioInit(HAL_WIFI_IOT_IO_NAME_GPIO_13);
    
    HalIoSetFunc(HAL_WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA); // Set up the gpio funcion as i2c

    IoTGpioInit(HAL_WIFI_IOT_IO_NAME_GPIO_14);
    HalIoSetFunc(HAL_WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoTI2cInit(WIFI_IOT_OLED_I2C_IDX_0, I2C_IDX_BAUDRATE); // Rate: 400kbps
    LOG_I("I2C0 bus init success !\n");
}

void DeviceInit(void)
{
    I2cBusInit();
    OledInit();
    LOG_I("Device init success !\n");
}