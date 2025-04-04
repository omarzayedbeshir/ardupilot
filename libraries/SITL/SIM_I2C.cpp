/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
  Simulated i2c buses and devices
*/


#include "SIM_config.h"
#include <GCS_MAVLink/GCS.h>
#include <SITL/SITL.h>

#include "SIM_I2C.h"

#include "SIM_Airspeed_DLVR.h"
#include "SIM_AS5600.h"
#include "SIM_BattMonitor_SMBus_Generic.h"
#include "SIM_BattMonitor_SMBus_Maxell.h"
#include "SIM_BattMonitor_SMBus_Rotoye.h"
#include "SIM_ICM40609.h"
#include "SIM_INA3221.h"
#include "SIM_IS31FL3195.h"
#include "SIM_LM2755.h"
#include "SIM_LP5562.h"
#include "SIM_MaxSonarI2CXL.h"
#include "SIM_MS5525.h"
#include "SIM_MS5611.h"
#include "SIM_QMC5883L.h"
#include "SIM_Temperature_MCP9600.h"
#include "SIM_Temperature_SHT3x.h"
#include "SIM_Temperature_TSYS01.h"
#include "SIM_Temperature_TSYS03.h"
#include "SIM_TeraRangerI2C.h"
#include "SIM_ToshibaLED.h"

#include <signal.h>

using namespace SITL;

enum class IOCtlType {
    RDWR = 0,
};

class IgnoredI2CDevice : public I2CDevice
{
public:
    int rdwr(I2C::i2c_rdwr_ioctl_data *&data) override {
        return -1;
    }
};
static IgnoredI2CDevice ignored;

#if AP_SIM_TOSHIBALED_ENABLED
static ToshibaLED toshibaled;
#endif
#if AP_SIM_MAXSONAR_I2C_XL_ENABLED
static MaxSonarI2CXL maxsonari2cxl;
static MaxSonarI2CXL maxsonari2cxl_2;
#endif
#if AP_SIM_BATT_MONITOR_SMBUS_MAXELL_ENABLED
static Maxell maxell;
#endif
#if AP_SIM_BATT_MONITOR_SMBUS_ROTOYE_ENABLED
static Rotoye rotoye;
#endif
static SIM_BattMonitor_SMBus_Generic smbus_generic;
#if AP_SIM_AIRSPEED_DLVR_ENABLED
static Airspeed_DLVR airspeed_dlvr;
#endif
#if AP_SIM_TEMPERATURE_SHT3X_ENABLED
static SHT3x sht3x;
#endif  // AP_SIM_TEMPERATURE_SHT3X_ENABLED
#if AP_SIM_TEMPERATURE_TSYS01_ENABLED
static TSYS01 tsys01;
#endif
#if AP_SIM_TSYS03_ENABLED
static TSYS03 tsys03;
#endif
#if AP_SIM_TEMPERATURE_MCP9600_ENABLED
static MCP9600 mcp9600;
#endif
#if AP_SIM_ICM40609_ENABLED
static ICM40609 icm40609;
#endif
#if AP_SIM_MS5525_ENABLED
static MS5525 ms5525;
#endif
#if AP_SIM_MS5611_ENABLED
static MS5611 ms5611;
#endif
#if AP_SIM_LP5562_ENABLED
static LP5562 lp5562;
#endif
#if AP_SIM_LM2755_ENABLED
static LM2755 lm2755;
#endif
#if AP_SIM_IS31FL3195_ENABLED
static IS31FL3195 is31fl3195;
#define SIM_IS31FL3195_ADDR 0x54
#endif
#if AP_SIM_COMPASS_QMC5883L_ENABLED
static QMC5883L qmc5883l;
#endif
#if AP_SIM_INA3221_ENABLED
static INA3221 ina3221;
#endif
#if AP_SIM_TERARANGERI2C_ENABLED
static TeraRangerI2C terarangeri2c;
#endif
#if AP_SIM_AS5600_ENABLED
static AS5600 as5600;  // AoA sensor
#endif  // AP_SIM_AS5600_ENABLED

struct i2c_device_at_address {
    uint8_t bus;
    uint8_t addr;
    I2CDevice &device;
} i2c_devices[] {
#if AP_SIM_TERARANGERI2C_ENABLED
    { 0, 0x31, terarangeri2c },   // RNGFNDx_TYPE = 14, RNGFNDx_ADDR = 49
#endif  // AP_SIM_TERARANGERI2C_ENABLED
#if AP_SIM_MAXSONAR_I2C_XL_ENABLED
    { 0, 0x70, maxsonari2cxl },   // RNGFNDx_TYPE = 2, RNGFNDx_ADDR = 112
#endif
#if AP_SIM_TEMPERATURE_MCP9600_ENABLED
    { 0, 0x60, mcp9600 }, // 0x60 is low address
#endif
#if AP_SIM_MAXSONAR_I2C_XL_ENABLED
    { 0, 0x71, maxsonari2cxl_2 }, // RNGFNDx_TYPE = 2, RNGFNDx_ADDR = 113
#endif
#if AP_SIM_ICM40609_ENABLED
    { 1, 0x01, icm40609 },
#endif
#if AP_SIM_TEMPERATURE_SHT3X_ENABLED
    { 1, 0x44, sht3x },
#endif
#if AP_SIM_TOSHIBALED_ENABLED
    { 1, 0x55, toshibaled },
#endif
    { 1, 0x38, ignored }, // NCP5623
    { 1, 0x39, ignored }, // NCP5623C
#if AP_SIM_AS5600_ENABLED
    { 1, 0x36, as5600 },
#endif
    { 1, 0x40, ignored }, // KellerLD
#if AP_SIM_MS5525_ENABLED
    { 1, 0x76, ms5525 },  // MS5525: ARSPD_TYPE = 4
#endif
#if AP_SIM_INA3221_ENABLED
    { 1, 0x42, ina3221 },
#endif
#if AP_SIM_TEMPERATURE_TSYS01_ENABLED
    { 1, 0x77, tsys01 },
#endif
#if AP_SIM_BATT_MONITOR_SMBUS_ROTOYE_ENABLED
    { 1, 0x0B, rotoye },        // Rotoye: BATTx_MONITOR 19, BATTx_I2C_ADDR 13
#endif
#if AP_SIM_BATT_MONITOR_SMBUS_MAXELL_ENABLED
    { 2, 0x0B, maxell },        // Maxell: BATTx_MONITOR 16, BATTx_I2C_ADDR 13
#endif
    { 3, 0x0B, smbus_generic},  // BATTx_MONITOR 7, BATTx_I2C_ADDR 13
#if AP_SIM_AIRSPEED_DLVR_ENABLED
    { 2, 0x28, airspeed_dlvr }, // ARSPD_TYPE = 7 5inch H2O sensor
#endif
#if AP_SIM_LP5562_ENABLED
    { 2, 0x30, lp5562 },        // LP5562 RGB LED driver
#endif
#if AP_SIM_LM2755_ENABLED
    { 2, 0x67, lm2755 },        // LM2755 RGB LED driver
#endif
#if AP_SIM_IS31FL3195_ENABLED
    { 2, SIM_IS31FL3195_ADDR, is31fl3195 },    // IS31FL3195 RGB LED driver; see page 9
#endif
#if AP_SIM_TSYS03_ENABLED
    { 2, 0x40, tsys03 },
#endif
#if AP_SIM_MS5611_ENABLED
    { 2, 0x77, ms5611 },        // MS5611: BARO_PROBE_EXT = 2
#endif
#if AP_SIM_COMPASS_QMC5883L_ENABLED
    { 2, 0x0D, qmc5883l },
#endif
};

void I2C::init()
{
    for (auto &i : i2c_devices) {
        i.device.init();
    }

#if AP_SIM_IS31FL3195_ENABLED
    // IS31FL3195 needs to know its own address:
    is31fl3195.set_product_id(SIM_IS31FL3195_ADDR);
#endif

    // sanity check the i2c_devices structure to ensure we don't have
    // two devices at the same address on the same bus:
    for (uint8_t i=0; i<ARRAY_SIZE(i2c_devices)-1; i++) {
        const auto &dev_i = i2c_devices[i];
        for (uint8_t j=i+1; j<ARRAY_SIZE(i2c_devices); j++) {
            const auto &dev_j = i2c_devices[j];
            if (dev_i.bus == dev_j.bus &&
                dev_i.addr == dev_j.addr) {
                AP_HAL::panic("Two devices at the same address on the same bus");
            }
        }
    }
}

void I2C::update(const class Aircraft &aircraft)
{
    for (auto daa : i2c_devices) {
        daa.device.update(aircraft);
    }
}

int I2C::ioctl_rdwr(i2c_rdwr_ioctl_data *data)
{
    const uint8_t addr = data->msgs[0].addr;
    const uint8_t bus = data->msgs[0].bus;
    for (auto &dev_at_address : i2c_devices) {
        if (dev_at_address.addr != addr) {
            continue;
        }
        if (dev_at_address.bus != bus) {
            continue;
        }
        return dev_at_address.device.rdwr(data);
    }
//            ::fprintf(stderr, "Unhandled i2c message: bus=%u addr=0x%02x flags=%u len=%u\n", msg.bus, msg.addr, msg.flags, msg.len);
    return -1;  // ?!
}

int I2C::ioctl(uint8_t ioctl_type, void *data)
{
    switch ((IOCtlType) ioctl_type) {
    case IOCtlType::RDWR:
        return ioctl_rdwr((i2c_rdwr_ioctl_data*)data);
    }
    return -1;
}
