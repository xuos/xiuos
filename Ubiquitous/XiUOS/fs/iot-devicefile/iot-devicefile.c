#include <xiuos.h>
#include <device.h>
#include <iot-vfs.h>
#include <iot-vfs_posix.h>

static void GetBusName(const char *path, char *bus_name)
{
    uint8 i = 0;
    for (; i < strlen(path); i++) {
        if (path[i] == '_') {
            bus_name[i] = '\0';
            break;
        }
        
        bus_name[i] = path[i];
    }
}

static void GetDrvName(char *bus_name, char *drv_name)
{
    snprintf(drv_name, strlen(bus_name) + 5, "%s_drv", bus_name);
}

static int DevicefileOpen(struct FileDescriptor *fdp, const char *path)
{
    int ret;
    BusType bus_id;
    char bus_name[NAME_NUM_MAX], drv_name[NAME_NUM_MAX];

    GetBusName(path, bus_name);
    GetDrvName(bus_name, drv_name);

    bus_id = BusFind(bus_name);
    if (bus_id == NULL)
        return -ENODEV;
    bus_id->owner_driver = BusFindDriver(bus_id, drv_name);
    bus_id->owner_haldev = BusFindDevice(bus_id, path);

    bus_id->match(bus_id->owner_driver, bus_id->owner_haldev);

    fdp->data = bus_id;

    ret = BusDevOpen(bus_id->owner_haldev);

    if (ret < 0) {
        fdp->data = NULL;
        return -EIO;
    }

    return 0;
}

static int DevicefileClose(struct FileDescriptor *fdp)
{
    int ret;
    BusType bus_id;

    NULL_PARAM_CHECK(fdp);
    NULL_PARAM_CHECK(fdp->data);
    bus_id = (BusType)fdp->data;

    ret = BusDevClose(bus_id->owner_haldev);

    if (ret == 0) {
        fdp->data = NULL;
        return 0;
    }

    return -EIO;
}

static ssize_t DevicefileRead(struct FileDescriptor *fdp, void *dst, size_t len)
{
    int ret;
    BusType bus_id;

    NULL_PARAM_CHECK(fdp);
    NULL_PARAM_CHECK(fdp->data);
    bus_id = (BusType)fdp->data;
    struct BusBlockReadParam param;
    memset(&param, 0, sizeof(struct BusBlockReadParam));
    param.size = len;
    param.buffer = dst;
    param.pos = fdp->pos;

    ret = BusDevReadData(bus_id->owner_haldev, &param);
    fdp->pos += param.read_length;

    return param.read_length;
}

static ssize_t DevicefileWrite(struct FileDescriptor *fdp, const void *src, size_t len)
{
    int ret;
    BusType bus_id;

    NULL_PARAM_CHECK(fdp);
    NULL_PARAM_CHECK(fdp->data);
    bus_id = (BusType)fdp->data;
    struct BusBlockWriteParam param;
    memset(&param, 0, sizeof(struct BusBlockWriteParam));
    param.size = len;
    param.buffer = src;
    param.pos = fdp->pos;

    ret = BusDevWriteData(bus_id->owner_haldev, &param);
    fdp->pos += ret;

    return ret;
}

static int DevicefileIoctl(struct FileDescriptor *fdp, int cmd, void *args)
{
    int ret;
    BusType bus_id;

    NULL_PARAM_CHECK(fdp);
    NULL_PARAM_CHECK(fdp->data);
    bus_id = (BusType)fdp->data;
    struct BusConfigureInfo param;
    param.configure_cmd = cmd;
    param.private_data = args;

    ret = BusDrvConfigure(bus_id->owner_driver, &param);

    return ret;
}

static int DevicefileMount(struct MountPoint *mp)
{
    return 0;
}

static struct Filesystem devicefile_fs = {
    .open = DevicefileOpen,
    .close = DevicefileClose,
    .read = DevicefileRead,
    .write = DevicefileWrite,
    .ioctl = DevicefileIoctl,
    .mount = DevicefileMount,
};

int DevicefileInit()
{
    RegisterFilesystem(FSTYPE_IOTDEVICEFILE, &devicefile_fs);

    return 0;
}
