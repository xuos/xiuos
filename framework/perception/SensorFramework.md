# XiUOS传感器框架

多数嵌入式操作系统对传感器的抽象采用以物理传感器设备为中心的方式，在应用开发时无法绕过传感器设备的多样性，进而增加了传感器应用的开发难度和周期。这种抽象带来的缺陷在面对一些可以同时采集多种物理量的传感器（如温湿度传感器）时尤为突出，因为应用开发者不得不考虑每种传感器设备的采集数据的能力。

XiUOS的传感器框架以用户为中心，采用了以物理量为中心的抽象方式，在开发应用时只需要考虑所需感知的物理量种类，无需把重点放在实际采集物理量的传感器设备。这种抽象方式有效地隐藏了底层传感器设备的硬件细节，对外提供统一的数据采集接口，可以简化传感器应用与驱动的开发。为实现以物理量为中心的抽象，传感器框架对传感器设备进行两层抽象：
* 一个物理传感器测量一种物理量的能力（ability）被抽象为一个SensorQuantity结构
* 一个物理传感器本身被抽象为一个SensorDevice结构

其中SensorQuantity被设计为可以采用类似面向对象的方法、针对不同物理量扩展其数据成员与接口，从而为众多不同性质的物理量实现统一的管理架构。在应用开发的过程中只需要使用对应物理量的SensorQuantity实例，无需关心传感器的硬件细节，从而实现数据采样功能与底层硬件的解耦。

从关联关系上来看，一个SensorQuantity对应一个SensorDevice，一个SensorDevice对应一个或多个SensorQuantity。例如，对于一个可以测量温度与湿度的传感器设备，该设备唯一对应一个SensorDevice结构，而该设备测量温度与湿度的能力分别对应一个SensorQuantity结构。两种数据结构的具体定义如下。

## struct SensorQuantity结构
```c
struct SensorQuantity {
    const char name[NAME_NUM_MAX]; /* name of the sensor quantity instance */
    enum SensorQuantityType type; /* type of data the sensor collects, such as CO2 concentration */
    struct SensorDevice *sdev; /* corresponding sensor device */
    struct SysDoubleLinklistNode link; /* link list node */
};
```
name成员是一个可读的名字，用于唯一标识一个SensorQuantity结构。

type成员表示该SensorQuantity可测量的物理量，用一个枚举变量表示：
```c
enum SensorQuantityType {
    SENSOR_QUANTITY_CO2 = 0, /* CO2 concentration */
    SENSOR_QUANTITY_TEMP, /* temperature */
    SENSOR_QUANTITY_HUMI, /* humidity */
    /* ...... */
    SENSOR_QUANTITY_END,
};
```

sdev成员表示该SensorQuantity所属的SensorDevice结构，其具体定义在下文给出。

最后，在系统中每种物理量的SensorQuantity被分别组织成不同双链表，如二氧化碳浓度SensorQuantity链表、温度SensorQuantity链表等，使用的链表节点即为link成员。

## struct SensorDevice结构
```c
struct SensorDevice {
    const char name[NAME_NUM_MAX]; /* name of the sensor device */
    struct SensorProductInfo info; /* sensor model info, such as vendor name and model name */
    struct SensorOps ops; /* filesystem-like APIs for data transferring */
    struct SensorInterface interface; /* physical interface for transferring data */
    struct SysDoubleLinklistNode link; /* link list node */
};
```
name成员记录传感器设备在系统中的名字，用于唯一标识一个SensorDevice结构

info成员记录传感器设备的一些属性信息，包括传感器的能力ability、厂家名vendor与型号product_model，其中ability用一个位图表示该传感器设备可以测量的物理量：
```c
#define SENSOR_ABILITY_CO2 ((uint32_t)(1 << SENSOR_QUANTITY_CO2))
#define SENSOR_ABILITY_TEMP ((uint32_t)(1 << SENSOR_QUANTITY_TEMP))
#define SENSOR_ABILITY_HUMI ((uint32_t)(1 << SENSOR_QUANTITY_HUMI))
/* ...... */

struct SensorProductInfo {
    uint32_t ability; /* bitwise OR of SENSOR_ABILITY_XXX */
    const char *vendor;
    const char *product_model;
};
```

ops成员包含统一的、类似文件系统的API，用于对传感器进行实际的数据读写。在使用一个传感器前后需要打开（open）/关闭（close）该传感器，read、write分别用与从传感器接收数据与向传感器发送数据，ioctl用于配置传感器属性（如波特率）：
```c
struct SensorOps {
    int (*open)(struct SensorDevice *sdev);
    void (*close)(struct SensorDevice *sdev);
    int (*read)(struct SensorDevice *sdev, void *buf, size_t len);
    int (*write)(struct SensorDevice *sdev, const void *buf, size_t len);
    int (*ioctl)(struct SensorDevice *sdev, int cmd, void *arg);
};
```

interface成员表示用于与传感器进行通信的总线设备：
```c
struct SensorInterface {
    device_t bus_device;
};
```

最后，系统中所有注册过的传感器设备被组织成一个双链表，即link成员。

## 传感器框架驱动开发

以二氧化碳传感器为例。传感器框架针对每个具体的物理量将SensorQuantity进行扩充，采用类似面向对象的手段添加其他必要成员，如：
```c
struct SensorQuantityCo2 {
    struct SensorQuantity parent; /* inherit from SensorQuantity */

    uint32_t (*read_concentration)(struct SensorQuantityCo2 *quant);

    uint32_t last_value; /* last measured value */
    uint32_t min_value; /* minimum measured value */
    uint32_t max_value; /* maximum measured value */
    uint32_t min_std; /* national standard: minimum */
    uint32_t max_std; /* national standard: maximum */
};
```

实现SensorOps中的数据通信API，具体实现细节取决于传感器型号，无法实现的API可以置为NULL：
```c
struct SensorOps co2_example_ops = {
    .open = co2_example_open;
    .close = co2_example_close;
    .read = co2_example_read;
    .write = NULL;
    .ioctl = co2_example_ioctl;
};
```

实现SensorQuantityCo2中的read_concentration接口，该接口用于读取当前空气中的二氧化碳浓度。在实现过程中可以使用SensorOps中的接口与传感器进行通信。

最后，将传感器设备添加到传感器框架。分别填充SensorDevice与对应物理量的SensorQuantity结构（二氧化碳即为SensorQuantityCo2)，并依次使用SensorDeviceRegister和SensorQuantityRegister函数将其注册到传感器框架：
```c
int SensorDeviceRegister(struct SensorDevice *sdev);
int SensorQuantityRegister(struct SensorQuantity *quant);

extern struct SensorOps co2_example_ops;
extern uint32_t co2_example_read_concentration(struct SensorQuantityCo2 *quant);

/* declare SensorDevice and SensorQuantityCo2 objects */
struct SensorDevice co2_example_sdev;
struct SensorQuantityCo2 co2_example_quant;

void register_co2_sensor()
{
    /* initialize and register the SensorDevice object */
    memset(&co2_example_sdev, 0, sizeof(SensorDevice));

    co2_example_sdev.name = "sensor1";
    co2_example_sdev.info.ability |= SENSOR_ABILITY_CO2;
    co2_example_sdev.info.vendor = "xxx";
    co2_example_sdev.info.product_model = "yyy";
    co2_example_sdev.ops = &co2_example_ops;
    co2_example_sdev.interface.bus_device = DeviceFind("uart1");

    SensorDeviceRegister(&co2_example_sdev);


    /* initialize and register the SensorQuantity object */
    memset(&co2_example_quant, 0, sizeof(SensorQuantityCo2));

    co2_example_quant.parent.name = "co2_1";
    co2_example_quant.parent.type = SENSOR_QUANTITY_CO2;
    co2_example_quant.parent.sdev = &co2_example_sdev;
    co2_example_quant.read_concentration = co2_example_read_concentration;

    SensorQuantityRegister((struct SensorQuantity *)&co2_example_quant);
}
```

## 传感器框架的使用

传感器应用开发者使用传感器框架提供的API操作传感器，传感器API可以分为通用API与物理量特有API。通用API用于传感器的获取、打开与关闭，物理量特有API用于传感器的数据采样。以二氧化碳传感器为例：
```c
/* generic API: find a sensor quantity instance by its name */
struct SensorQuantity *SensorQuantityFind(const char *name);

/* generic API: open/close a sensor quantity instance */
int SensorQuantityOpen(struct SensorQuantity *quant);
void SensorQuantityClose(struct SensorQuantity *quant);

/* CO2 API: get current CO2 concentration reading (in ppm unit) */
uint32_t SensorCo2Read(struct SensorQuantityCo2 *quant);
```

在获取数据前，需要先获取并打开要使用的传感器；传感器打开后可以随时对传感器数据进行读取；使用完毕后，须关闭传感器。完整的使用过程示例如下：
```c
int main(int argc, char *argv[])
{
    int ret;
    struct SensorQuantity *quant;
    struct SensorQuantityCo2 *co2_quant;

    /* get the CO2 sensor quantity instance */
    quant = SensorQuantityFind("co2_1");
    CHECK(quant->type == SENSOR_QUANTITY_CO2);

    /* open the CO2 sensor quantity instance */
    co2_quant = (struct SensorQuantityCo2 *)quant;
    ret = SensorQuantityOpen(quant);
    CHECK(ret == EOK);

    /* read CO2 concentration for 5 times, just for demonstration */
    for (int i = 0; i < 5; i++)
        KPrintf("Current CO2 concentration is %u ppm\n", SensorCo2Read(co2_quant));

    SensorQuantityClose(quant);

    return 0;
}
```
