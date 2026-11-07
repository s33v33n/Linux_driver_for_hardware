/*
 * crcmodule - QEMU device with the Verilog CRC16-CDMA2000 module (crcmodule.v)
 *
 * The Verilog module is simulated by its Verilator model. The device maps the module registers into the QEMU machine..
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/module.h"
#include "crcmodule_model.h"

#define TYPE_CRCMODULE          "crcmodule"
OBJECT_DECLARE_SIMPLE_TYPE(CrcModuleState, CRCMODULE)

/* --- CRCMODULE REGISTERS (addresses on the Verilog bus) --- */
#define CRCMODULE_REG_BASE      0x200       /* first register, start of the MMIO window */
#define CRCMODULE_REG_CTRL      0x218       /* control, 0x00 = calculate CRC */
#define CRCMODULE_MMIO_SIZE     0x20        /* IN, STATE, RESULT, CTRL data window size 32 bytes*/

/* clock cycles after CTRL write: 250 bytes need 252 cycles (init + bytes + finish) */
#define CRCMODULE_CALC_CYCLES   260

struct CrcModuleState {
    SysBusDevice parent_obj;
    MemoryRegion mmio;
};


/* START OF MMIO */

/* CPU read: one bus read cycle on the Verilog model */
static uint64_t crcmodule_read(void *opaque, hwaddr addr, unsigned int size)
{
    return crcmodule_model_read(CRCMODULE_REG_BASE + addr);
}

/* CPU write: one bus write cycle on the Verilog model */
static void crcmodule_write(void *opaque, hwaddr addr, uint64_t val,
                            unsigned int size)
{
    uint16_t reg = CRCMODULE_REG_BASE + addr;

    crcmodule_model_write(reg, (uint32_t)val);

    /* let the module finish the calculation before the CPU reads STATE */
    if (reg == CRCMODULE_REG_CTRL) {
        crcmodule_model_tick(CRCMODULE_CALC_CYCLES);
    }
}

static const MemoryRegionOps crcmodule_ops = {
    .read = crcmodule_read,
    .write = crcmodule_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 1,           /* driver uses ioread8 / iowrite8 */
        .max_access_size = 4,
    },
};

/* END OF MMIO */


/* START OF DEVICE DESCRIPTION */

static void crcmodule_init(Object *obj)
{
    CrcModuleState *s = CRCMODULE(obj);

    memory_region_init_io(&s->mmio, obj, &crcmodule_ops, s,
                          TYPE_CRCMODULE, CRCMODULE_MMIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

/* create the Verilog model when the device is realized */
static void crcmodule_realize(DeviceState *dev, Error **errp)
{
    crcmodule_model_init();
}

/* machine reset -> n_reset pulse */
static void crcmodule_reset(DeviceState *dev)
{
    crcmodule_model_reset();
}

static void crcmodule_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = crcmodule_realize;
    device_class_set_legacy_reset(dc, crcmodule_reset);
}

static const TypeInfo crcmodule_info = {
    .name          = TYPE_CRCMODULE,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(CrcModuleState),
    .instance_init = crcmodule_init,
    .class_init    = crcmodule_class_init,
};

static void crcmodule_register_types(void)
{
    type_register_static(&crcmodule_info);
}

type_init(crcmodule_register_types)

/* END OF DEVICE DESCRIPTION */
