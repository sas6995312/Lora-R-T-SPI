#ifndef MYLORA_H
#define MYLORA_H

#include <QObject>
#include <crypt.h>
#include <QTextEdit>
#include <QByteArray>
#include <QDebug>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <libconfig.h>
#include <libconfig.h++>
#include <stdint.h>
#include <ctime>

class myLora
{
public:
    myLora();

    void LoraInit();

    void SX1276Reset();

    uint8_t SX1276Read(uint8_t addr,uint8_t data);

    void SX1276Write(uint8_t addr,uint data);

    uint8_t SX1276ReadBuffer(uint8_t addr,uint8_t *buffer,uint8_t size);

    void SX1276WriteBuffer(uint8_t addr,uint8_t *buffer,uint8_t size);

    void SX1276ReadFifo(uint8_t *buffer,uint8_t size);

    void SX1276WriteFifo(uint8_t *buffer,uint8_t size);

    void SX1276LoRaSetOpMode(uint8_t opMode);

    void SX1276SetSPIState(int state);

    void TxSet(uint8_t TxPacketSize,uint8_t *buffer);

    void RxSet();

    void TxStart();

    void RxProcess();

    uint8_t SX1276LoRaGetRFState();
private:
    void LoRaOn();
};

#endif // MYLORA_H
