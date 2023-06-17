#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "serial_dev.h"
#include "minichlink.h"

void * TryInit_Ardulink(void);

static int ArdulinkWriteReg32(void * dev, uint8_t reg_7_bit, uint32_t command);
static int ArdulinkReadReg32(void * dev, uint8_t reg_7_bit, uint32_t * commandresp);
static int ArdulinkFlushLLCommands(void * dev);
static int ArdulinkExit(void * dev);
static int ArdulinkTargetReset(void * dev, int reset);

typedef struct {
    struct ProgrammerStructBase psb;
    serial_dev_t serial;
} ardulink_ctx_t;

int ArdulinkWriteReg32(void * dev, uint8_t reg_7_bit, uint32_t command)
{
    uint8_t buf[6];
    buf[0] = 'w';
    buf[1] = reg_7_bit;

    //fprintf(stderr, "WriteReg32: 0x%02x = 0x%08x\n", reg_7_bit, command);

    buf[2] = command & 0xff;
    buf[3] = (command >> 8) & 0xff;
    buf[4] = (command >> 16) & 0xff;
    buf[5] = (command >> 24) & 0xff;

    if (serial_dev_write(&((ardulink_ctx_t*)dev)->serial, buf, 6) == -1)
        return -errno;

    if (serial_dev_read(&((ardulink_ctx_t*)dev)->serial, buf, 1) == -1)
        return -errno;

    return buf[0] == '+' ? 0 : -EPROTO;
}

int ArdulinkReadReg32(void * dev, uint8_t reg_7_bit, uint32_t * commandresp)
{
    uint8_t buf[4];
    buf[0] = 'r';
    buf[1] = reg_7_bit;

    if (serial_dev_write(&((ardulink_ctx_t*)dev)->serial, buf, 2) == -1)
        return -errno;

    if (serial_dev_read(&((ardulink_ctx_t*)dev)->serial, buf, 4) == -1)
        return -errno;

    *commandresp = (uint32_t)buf[0] | (uint32_t)buf[1] << 8 | \
        (uint32_t)buf[2] << 16 | (uint32_t)buf[3] << 24;

    //fprintf(stderr, "ReadReg32: 0x%02x = 0x%08x\n", reg_7_bit, *commandresp);

    return 0;
}

int ArdulinkFlushLLCommands(void * dev)
{
    return 0;
}

int ArdulinkExit(void * dev)
{
    serial_dev_close(&((ardulink_ctx_t*)dev)->serial);
    free(dev);
    return 0;
}

int ArdulinkTargetReset(void * dev, int reset) {
    char c;

    fprintf(stderr, "Ardulink: target reset %d\n", reset);

    // Assert reset.
    c = reset ? 'a' : 'A';
    if (serial_dev_write(&((ardulink_ctx_t*)dev)->serial, &c, 1) == -1)
        return -errno;

    if (serial_dev_read(&((ardulink_ctx_t*)dev)->serial, &c, 1) == -1)
        return -errno;

    if (c != '+')
        return -EPROTO;

    usleep(20000);
    return 0;
}

void * TryInit_Ardulink(void)
{
    ardulink_ctx_t *ctx;
    char first;

    if (!(ctx = calloc(sizeof(ardulink_ctx_t), 1))) {
        perror("calloc");
        return NULL;
    }

    if (serial_dev_create(&ctx->serial, DEFAULT_SERIAL_NAME, 115200) == -1) {
        perror("create");
        return NULL;
    }

    if (serial_dev_open(&ctx->serial) == -1) {
        perror("open");
        return NULL;
    }

    // Arduino DTR reset.
    if (serial_dev_do_dtr_reset(&ctx->serial) == -1) {
        perror("dtr reset");
        return NULL;
    }

    // Flush anything that might be in the RX buffer, we need the sync char.
    if (serial_dev_flush_rx(&ctx->serial) == -1) {
        perror("flush rx");
        return NULL;
    }

    // Let the bootloader do its thing.
    usleep(3UL*1000UL*1000UL);

    if (serial_dev_read(&ctx->serial, &first, 1) == -1) {
        perror("read");
        return NULL;
    }

    if (first != '!') {
        fprintf(stderr, "Ardulink: not the sync character.\n");
        return NULL;
    }

    if (ArdulinkTargetReset(ctx, 1) != 0) {
        fprintf(stderr, "Ardulink: target reset failed.\n");
        return NULL;
    }

    fprintf(stderr, "Ardulink: synced.\n");

    MCF.WriteReg32 = ArdulinkWriteReg32;
	MCF.ReadReg32 = ArdulinkReadReg32;
    MCF.FlushLLCommands = ArdulinkFlushLLCommands;
	MCF.Exit = ArdulinkExit;
    MCF.TargetReset = ArdulinkTargetReset;

	return ctx;
}
