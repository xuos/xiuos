/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/
#include <stdio.h>
#include <string.h>
#include <user_api.h>

extern int SensorFrameworkInit(void);
extern int RegisterAdapterEthernet(void);
extern int RegisterAdapterWifi(void);
extern int RegisterAdapterZigbee(void);
extern int RegisterAdapterNBIoT(void);
extern int LoraSx12xxSpiDeviceInit();

extern int D124VoiceInit(void);
extern int Hs300xTemperatureInit(void);
extern int Hs300xHumidityInit(void);
extern int Ps5308Pm1_0Init(void);
extern int Zg09Co2Init(void);

typedef int (*InitFunc)(void);
struct InitDesc
{
	const char* fn_name;
	const InitFunc fn;
};

static int AppInitDesc(struct InitDesc sub_desc[])
{
	int i = 0;
	int ret = 0;
	for( i = 0; sub_desc[i].fn != NULL; i++ ) {
		ret = sub_desc[i].fn();
		printf("initialize %s %s\n",sub_desc[i].fn_name, ret == 0 ? "success" : "failed");
		if(0 != ret) {
			break;
		}
	}
	return ret;
}

static struct InitDesc framework[] = 
{
#ifdef PERCEPTION_SENSORDEVICE
	{ "perception_framework", SensorFrameworkInit },
#endif

	{ "NULL", NULL },
};

static struct InitDesc perception_desc[] = 
{
#ifdef PERCEPTION_D124
	{ "d124_voice", D124VoiceInit },
#endif

#ifdef PERCEPTION_HS300X
#ifdef SENSOR_QUANTITY_HS300X_TEMPERATURE
	{ "hs300x_temperature", Hs300xTemperatureInit },
#endif
#ifdef SENSOR_QUANTITY_HS300X_HUMIDITY
	{ "hs300x_humidity", Hs300xHumidityInit },
#endif
#endif

#ifdef PERCEPTION_PS5308
#ifdef SENSOR_QUANTITY_PS5308_PM1_0
	{ "ps5308_pm1_0", Ps5308Pm1_0Init },
#endif
#endif

#ifdef PERCEPTION_ZG09
	{ "zg09_co2", Zg09Co2Init },
#endif

	{ "NULL", NULL },
};

static struct InitDesc connection_desc[] = 
{
#ifdef CONNECTION_COMMUNICATION_ETHERNET
	{ "ethernet adpter", RegisterAdapterEthernet },
#endif

#ifdef CONNECTION_COMMUNICATION_WIFI
	{ "wifi adpter", RegisterAdapterWifi },
#endif

#ifdef CONNECTION_COMMUNICATION_LORA
	{ "lora adpter", LoraSx12xxSpiDeviceInit},
#endif

#ifdef CONNECTION_COMMUNICATION_ZIGBEE
	{ "zigbee adpter", RegisterAdapterZigbee},
#endif

#ifdef CONNECTION_COMMUNICATION_NB_IOT
	{ "NB-IoT adpter", RegisterAdapterNBIoT},
#endif
	{ "NULL", NULL },
};

/**
 * This function will init perception framework and all sub perception sensors
 * @param sub_desc framework
 * 
 */
static int PerceptionFrameworkInit(struct InitDesc sub_desc[])
{
	int i = 0;
	int ret = 0;
	for ( i = 0; sub_desc[i].fn != NULL; i++ ) {
		if (0 == strncmp(sub_desc[i].fn_name, "perception_framework", strlen("perception_framework"))) {
			ret = sub_desc[i].fn();
			break;
		}
	}

	if (0 == ret) {
		printf("initialize perception_framework success.\n");
		AppInitDesc(perception_desc);
	}
}

/**
 * This function will init connection framework and all sub components
 * @param sub_desc framework
 * 
 */
static int ConnectionFrameworkInit(struct InitDesc sub_desc[])
{
	return AppInitDesc(connection_desc);
}

/**
 * This function will init system framework
 * 
 */
int FrameworkInit()
{
#ifdef PERCEPTION_SENSORDEVICE
	PerceptionFrameworkInit(framework);
#endif

#ifdef CONNECTION_ADAPTER
	ConnectionFrameworkInit(framework);
#endif

    return 0;
}