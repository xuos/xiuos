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

/**
* @file:    console.c
* @brief:   console file
* @version: 1.0
* @author:  AIIT XUOS Lab
* @date:    2020/3/15
*
*/


#include <xiuos.h>
#include <device.h>

#if defined(KERNEL_CONSOLE)
static HardwareDevType _console = NONE;
#endif

/**
 * Obtain the console
 *
 */
HardwareDevType ObtainConsole(void)
{
#if defined(KERNEL_CONSOLE)
    return _console;
#endif
}

/**
 * Setup a console
 * 
 *
 * @param name console device name 
 *
 */
HardwareDevType InstallConsole(const char *bus_name, const char *drv_name, const char *dev_name)
{
#if defined(KERNEL_CONSOLE)
    BusType console_bus;
    DriverType console_drv = NONE;
    HardwareDevType console = NONE;
    struct SerialDevParam *serial_dev_param = NONE;
    struct SerialCfgParam serial_cfg;
    struct BusConfigureInfo configure_info;

    NULL_PARAM_CHECK(bus_name);
    NULL_PARAM_CHECK(drv_name);
    NULL_PARAM_CHECK(dev_name);

    console_bus = BusFind(bus_name);
    console_drv = BusFindDriver(console_bus, drv_name);
    console = BusFindDevice(console_bus, dev_name);

    if (console != NONE) {
        if (_console != NONE) {
            BusDevClose(_console);
        }

        configure_info.configure_cmd = OPE_INT;
        memset(&serial_cfg, 0, sizeof(struct SerialCfgParam));
        configure_info.private_data = &serial_cfg;
        BusDrvConfigure(console_drv, &configure_info);
        console_bus->match(console_drv, console);

        serial_dev_param = (struct SerialDevParam *)console->private_data;
        serial_dev_param->serial_set_mode = 0;
        serial_dev_param->serial_stream_mode = SIGN_OPER_STREAM;
        BusDevOpen(console);
        _console = console;
    } else {
        console = _console;
    }

    return console;
#endif
}

#if defined(KERNEL_CONSOLE)

static __inline x_bool IsDigit(char c)
{
    return ((unsigned)((c) - '0') < 10);
}

static __inline unsigned int CharToNum(const char **s)
{
    unsigned int i = 0;
    while (IsDigit(**s)) {
        i = i * 10 + **s - '0';
        ++ *s;
    }

    return i;
}

#define ZEROPAD         (1 << 0)
#define LEFT            (1 << 1)
#define SIGN            (1 << 2)
#define SPACE           (1 << 3)
#define HASH            (1 << 4)

#define PRINTLONG       (1 << 5)
#define PRINTLONGLONG   (1 << 6)
#define PRINTSHORT      (1 << 7)
#define PRINTCHAR       (1 << 8)

#define CAPITAL         (1 << 9)

static size_t LongToChar(char* str, unsigned long value, uint32 flags, int32 precision, uint8 base, int32 width, x_bool minus, int32 pointer, int32 size)
{
    char buff[KERNEL_CONSOLEBUF_SIZE/4] = {0};
    int32 length = 0;

    if(!value)
        flags &= ~HASH;

    if(!(precision > 0) || value) {
        do {
            const char number = (char)(value % base);
            if(number < 10) {
                buff[length] = number + '0';
            } else {
                if(flags & CAPITAL)
                    buff[length] = 'A' + number - 10;
                else
                    buff[length] = 'a' + number - 10;
            }
            ++ length;
            value /= base;
        } while (value && (length < KERNEL_CONSOLEBUF_SIZE/4));
    }

    if(!(flags & LEFT)) {
        if(width && (minus || (flags & (SIGN | SPACE | ZEROPAD))))
            -- width;
        while((length < precision) && (length < KERNEL_CONSOLEBUF_SIZE/4))
            buff[length++] = '0';
        while((length < width) && (flags & ZEROPAD) && (length < KERNEL_CONSOLEBUF_SIZE/4))
            buff[length++] = '0';
    }

    if((flags & HASH) && ((base == 8) || (base == 16))) {
        if(!(precision > 0) && length && ((length == precision) || (length == width))) {
            -- length;
            if(length && (base == 16))
                -- length;
        }
        switch (base) {
        case 8:
            buff[length++] = '0';
            break;
        
        case 16:
            if(flags & CAPITAL)
            {
                buff[length++] = 'X';
                buff[length++] = '0';
            }
            else
            {
                buff[length++] = 'x';
                buff[length++] = '0';
            }
            break;

        default:
            break;
        }
    }

    if(length < KERNEL_CONSOLEBUF_SIZE/4) {
        if(minus)
            buff[length++] = '-';
        else if(flags & SIGN)
            buff[length++] = '+';
        else if(flags & SPACE)
            buff[length++] = ' ';
    }

    const int32 index = pointer;

    if(!(flags & LEFT) && !(flags & ZEROPAD) && (length < width)) {
        for(int32 i = length; i < width; i++) {
            str[pointer++] = ' ';
            if(pointer >= size) {
                str[size - 1] = '\0';
                return size;
            }
        }
    }

    while (length) {
        str[pointer++] = buff[--length];
        if(pointer >= size) {
            str[size - 1] = '\0';
            return size;
        }
    }
    
    if(flags & LEFT) {
        while (pointer - index < width) {
            str[pointer++] = ' ';
            if(pointer >= size) {
                str[size - 1] = '\0';
                return size;
            }
        }
    }

    return pointer;
}

static size_t LonglongToChar(char* str, unsigned long long value, uint32 flags, int32 precision, uint8 base, int32 width, x_bool minus, int32 pointer, int32 size)
{
    char buff[KERNEL_CONSOLEBUF_SIZE/4] = {0};
    int32 length = 0;

    if(!value)
        flags &= ~HASH;

    if(!(precision > 0) || value){
        do {
            const char number = (char)(value % base);
            if(number < 10) {
                buff[length] = number + '0';
            } else {
                if(flags & CAPITAL)
                    buff[length] = 'A' + number - 10;
                else
                    buff[length] = 'a' + number - 10;
            }
            ++ length;
            value /= base;
        } while (value && (length < KERNEL_CONSOLEBUF_SIZE/4));
    }

    if(!(flags & LEFT)) {
        if(width && (minus || (flags & (SIGN | SPACE | ZEROPAD))))
            -- width;
        while((length < precision) && (length < KERNEL_CONSOLEBUF_SIZE/4))
            buff[length++] = '0';
        while((length < width) && (flags & ZEROPAD) && (length < KERNEL_CONSOLEBUF_SIZE/4))
            buff[length++] = '0';
    }

    if((flags & HASH) && ((base == 8) || (base == 16))) {
        if(!(precision > 0) && length && ((length == precision) || (length == width))) {
            -- length;
            if(length && (base == 16))
                -- length;
        }
        switch (base) {
        case 8:
            buff[length++] = '0';
            break;
        
        case 16:
            if(flags & CAPITAL) {
                buff[length++] = 'X';
                buff[length++] = '0';
            } else {
                buff[length++] = 'x';
                buff[length++] = '0';
            }
            break;

        default:
            break;
        }
    }

    if(length < KERNEL_CONSOLEBUF_SIZE/4) {
        if(minus)
            buff[length++] = '-';
        else if(flags & SIGN)
            buff[length++] = '+';
        else if(flags & SPACE)
            buff[length++] = ' ';
    }

    const int32 index = pointer;

    if(!(flags & LEFT) && !(flags & ZEROPAD) && (length < width)) {
        for(int32 i = length; i < width; i++) {
            str[pointer++] = ' ';
            if(pointer >= size) {
                str[size - 1] = '\0';
                return size;
            }
        }
    }

    while (length) {
        str[pointer++] = buff[--length];
        if(pointer >= size) {
            str[size - 1] = '\0';
            return size;
        }
    }
    
    if(flags & LEFT) {
        while (pointer - index < width) {
            str[pointer++] = ' ';
            if(pointer >= size) {
                str[size - 1] = '\0';
                return size;
            }
        }    
    }

    return pointer;
}

int VsnPrintf(char *buf, int32 size, const char *fmt, va_list args)
{
    NULL_PARAM_CHECK(buf);
    
    int32 pointer = 0;
    int32 len = 0;
    int32 i = 0;
    int32 width = 0;
    int32 precision = 0;
    uint32 flags = 0;
    uint8 base = 0;
    char c = 0;
    char *str = NONE;
    char *s = NONE;

    pointer = 0;
    str = buf;

    while(*fmt) {
        if (*fmt != '%') {
            buf[pointer++] = *fmt;
            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }
            ++ fmt;
            continue;
        }

        flags = 0;
        uint8 CheckFlags = 1;

        while (CheckFlags) {
            ++ fmt;
            switch (*fmt) {
            case '0':
                flags |= ZEROPAD;
                break;

            case '-':
                flags |= LEFT;
                break;

            case '+':
                flags |= SIGN;
                break;

            case ' ':
                flags |= SPACE;
                break;

            case '#':
                flags |= HASH;
                break;
            
            default:
                CheckFlags = 0;
                break;
            }
        }

        width = -1;
        if (IsDigit(*fmt))
            width = CharToNum(&fmt);
        else if (*fmt == '*') {
            width = va_arg(args, int);
            if (width < 0) {
                width = -width;
                flags |= LEFT;
            }
            ++ fmt;
        }

        precision = -1;
        if (*fmt == '.') {
            ++ fmt;
            if (IsDigit(*fmt))
                precision = CharToNum(&fmt);
            else if (*fmt == '*') {
                precision = va_arg(args, int);
                ++ fmt;
            }
            if (precision < 0)
                precision = 0;
        }

        if (*fmt == 'l') {
            ++ fmt;
            if (*fmt == 'l') {
                flags |= PRINTLONGLONG;
                ++ fmt;
            }
            else
                flags |= PRINTLONG;
        }
        else if(*fmt == 'h') {
            ++ fmt;
            if (*fmt == 'h') {
                flags |= PRINTCHAR;
                ++ fmt;
            }
            else
                flags |= PRINTSHORT;
        }

        base = 10;

        switch (*fmt) {
        case 'c':
            if (!(flags & LEFT)) {
                while (--width > 0) {
                    buf[pointer++] = ' ';
                    if(pointer >= size)
                    {
                        buf[size - 1] = '\0';
                        return size - 1;
                    }
                }
            }

            c = (char)va_arg(args, int);
            buf[pointer++] = c;
            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }

            while (--width > 0) {
                buf[pointer++] = ' ';
                if(pointer >= size) {
                    buf[size - 1] = '\0';
                    return size - 1;
                }
            }
            break;

        case 's':
            s = va_arg(args, char *);
            if (!s)
                s = "(NULL)";

            len = strlen(s);
            if (precision > 0 && len > precision)
                len = precision;

            if (!(flags & LEFT)) {
                while (len < width--) {
                    buf[pointer++] = ' ';
                    if(pointer >= size) {
                        buf[size - 1] = '\0';
                        return size - 1;
                    }
                }
            }

            for (i = 0; i < len; ++ i) {
                buf[pointer++] = *s;
                if(pointer >= size) {
                    buf[size - 1] = '\0';
                    return size - 1;
                }
                ++ s;
            }

            while (len < width--) {
                buf[pointer++] = ' ';
                if(pointer >= size) {
                    buf[size - 1] = '\0';
                    return size - 1;
                }
            }
            break;

        case 'p':
            width = sizeof(void *) * 2;
            flags |= ZEROPAD | CAPITAL;
            /* Determine the machine word length */
#ifdef ARCH_CPU_64BIT
            if(sizeof(long) == sizeof(long long))
                pointer = (long)LonglongToChar(buf, (unsigned long)va_arg(args, void*), flags, precision, 16, width, 0, pointer, size);
            else
#endif
                pointer = LongToChar(buf, (unsigned long)va_arg(args, void*), flags, precision, 16, width, 0, pointer, size);
            
            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }
            break;

        case '%':
            buf[pointer] = '%';
            ++ pointer;
            break;

        case 'o':
            flags &= ~SIGN;
            base = 8;
            if(flags & PRINTLONGLONG)
                pointer = LonglongToChar(buf, va_arg(args, unsigned long long), flags, precision, base, width, 0, pointer, size);
            else if(flags & PRINTLONG)
                pointer = LongToChar(buf, va_arg(args, unsigned long), flags, precision, base, width, 0, pointer, size);
            else {
                if(flags & PRINTSHORT) {
                    const unsigned int value = (unsigned short int)va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                }  else if(flags & PRINTCHAR) {
                    const unsigned int value = (unsigned char)va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                } else {
                    const unsigned int value = va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                }
            }

            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }
            break;

        case 'X':
            flags |= CAPITAL;
        case 'x':
            flags &= ~SIGN;
            base = 16;
            
            if(flags & PRINTLONGLONG)
                pointer = LonglongToChar(buf, va_arg(args, unsigned long long), flags, precision, base, width, 0, pointer, size);
            else if(flags & PRINTLONG)
                pointer = LongToChar(buf, va_arg(args, unsigned long), flags, precision, base, width, 0, pointer, size);
            else {
                if(flags & PRINTSHORT) {
                    const unsigned int value = (unsigned short int)va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                } else if(flags & PRINTCHAR) {
                    const unsigned int value = (unsigned char)va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                } else {
                    const unsigned int value = va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                }
            }

            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }
            break;

        case 'u':
            flags &= ~SIGN;
            base = 10;
            flags &= ~HASH;
            if(flags & PRINTLONGLONG)
                pointer = LonglongToChar(buf, va_arg(args, unsigned long long), flags, precision, base, width, 0, pointer, size);
            else if(flags & PRINTLONG)
                pointer = LongToChar(buf, va_arg(args, unsigned long), flags, precision, base, width, 0, pointer, size);
            else {
                if(flags & PRINTSHORT) {
                    const unsigned int value = (unsigned short int)va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                } else if(flags & PRINTCHAR) {
                    const unsigned int value = (unsigned char)va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                } else {
                    const unsigned int value = va_arg(args, unsigned int);
                    pointer = LongToChar(buf, value, flags, precision, base, width, 0, pointer, size);
                }
            }

            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }
            break;

        case 'd':
        case 'i':
            base = 10;
            flags &= ~HASH;
            if(flags & PRINTLONGLONG) {
                const long long value = va_arg(args, long long);
                pointer = LonglongToChar(buf, (unsigned long long)(value > 0 ? value : 0 - value), flags, precision, base, width, (value < 0 ? 1 : 0), pointer, size);
            } else if(flags & PRINTLONG) {
                const long value = va_arg(args, long);
                pointer = LongToChar(buf, (unsigned long)(value > 0 ? value : 0 - value), flags, precision, base, width, (value < 0 ? 1 : 0), pointer, size);
            } else {
                if(flags & PRINTSHORT) {
                    const int value = (short int)va_arg(args, int);
                    pointer = LongToChar(buf, (unsigned int)(value > 0 ? value : 0 - value), flags, precision, base, width, (value < 0 ? 1 : 0), pointer, size);
                } else if(flags & PRINTCHAR) {
                    const int value = (char)va_arg(args, int);
                    pointer = LongToChar(buf, (unsigned int)(value > 0 ? value : 0 - value), flags, precision, base, width, (value < 0 ? 1 : 0), pointer, size);
                } else {
                    const int value = va_arg(args, int);
                    pointer = LongToChar(buf, (unsigned int)(value > 0 ? value : 0 - value), flags, precision, base, width, (value < 0 ? 1 : 0), pointer, size);
                }
            }

            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }
            break;

        default:
            buf[pointer++] = '%';
            if(pointer >= size) {
                buf[size - 1] = '\0';
                return size - 1;
            }

            if (*fmt) {
                buf[pointer++] = *fmt;
                if(pointer >= size) {
                    buf[size - 1] = '\0';
                    return size - 1;
                }
            } else {
                -- fmt;
            }
            break;
        }
        ++ fmt;
    }

    if(pointer > size)
        pointer = size - 1;
    buf[pointer] = '\0';

    return pointer;
}

#endif

void KPrintf(const char *fmt, ...)
{
#ifdef KERNEL_CONSOLE
    if(_console != NONE) {
        va_list args;
        x_size_t length = 0;
        static char logbuf[KERNEL_CONSOLEBUF_SIZE] = {0};

        va_start(args, fmt);

        length = VsnPrintf(logbuf, sizeof(logbuf) - 1, fmt, args);
        if (length > KERNEL_CONSOLEBUF_SIZE - 1)
            length = KERNEL_CONSOLEBUF_SIZE - 1;

        struct BusBlockWriteParam write_param;
        write_param.pos = 0;
        write_param.buffer = (void *)logbuf;
        write_param.size = length;
        BusDevWriteData(_console, (void *)&write_param);
        
        va_end(args);
    }
#endif
}


