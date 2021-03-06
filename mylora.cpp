#include "mylora.h"
#include <lora_pindef.h>
#include <lora_regdef.h>
static uint8_t RFState = RFLR_STATE_IDLE;
static uint8_t RFBuffer[255] = {0};
myLora::myLora()
{
    LoraInit();
}
void initPin(){
    wiringPiSetupGpio();
    pinMode(TXEN, OUTPUT);
    pinMode(RXEN, OUTPUT);
    pinMode(RST, OUTPUT);
    pinMode(NSS, OUTPUT);
    pinMode(DIO0, INPUT);
    pinMode(DIO1, INPUT);
    pinMode(DIO2, INPUT);
    pinMode(DIO3, INPUT);
    pinMode(DIO4, INPUT);
    pinMode(DIO5, INPUT);
}
void initPinData(){

    digitalWrite(NSS, HIGH);
    digitalWrite(RST, HIGH);
}

void myLora::LoraInit(){
    initPin();
    initPinData();
    if(wiringPiSPISetupMode(SPI_CHANNEL, SPI_FREQUENCY, SPI_MODE) < 0){
        qDebug() << "wiringPiSPISetip() is failed!\n";
    }

    if(wiringPiSetupGpio() == -1){
        qDebug() << "wiringPiSetupGpio Error!\n";
    }
    LoRaOn();
    SX1276Write(REG_LR_PADAC,0x87);delay(10);
    SX1276Write(REG_LR_PACONFIG,0xCF);delay(10);
    SX1276Write(REG_LR_FRFMSB,0xE6);delay(10);
    SX1276Write(REG_LR_FRFMID,0x00);delay(10);
    SX1276Write(REG_LR_FRFLSB,0x26);delay(10);
    SX1276Write(REG_LR_MODEMCONFIG1,0x73);delay(10);
    SX1276Write(REG_LR_MODEMCONFIG2,0x74);delay(10);
    SX1276Write(REG_LR_PAYLOADLENGTH,0xF0);delay(10);
    SX1276Write(REG_LR_MODEMCONFIG3,0x04);delay(10);

    SX1276Write(REG_LR_PARAMP,0x08);delay(10);

}
void myLora::LoRaOn()
{
    SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);
    RegOpMode = (RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_ON;
    SX1276Write(REG_LR_OPMODE,RegOpMode);
    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
    delay(20);
    RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
    RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
    SX1276WriteBuffer(REG_LR_DIOMAPPING1 , &RegDioMapping1,2);
}

void myLora::SX1276WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    digitalWrite(NSS,LOW);
    buffer[0] = addr | 0x80;
    wiringPiSPIDataRW(SPI_CHANNEL,buffer,size);
    digitalWrite(NSS,HIGH);
}
uint8_t myLora::SX1276ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    digitalWrite(NSS,LOW);
    buffer[0] = addr & 0x7F;
    wiringPiSPIDataRW(SPI_CHANNEL,buffer,2);
    if(addr == 0x00){qDebug() << "FifoRead = " <<buffer[1];}
    digitalWrite(NSS,HIGH);
    return buffer[1];
}
void myLora::SX1276Write(uint8_t addr, uint data)
{
    uint8_t buffer[2];
    buffer[1] = data;
    SX1276WriteBuffer(addr,buffer,2);
}
uint8_t myLora::SX1276Read(uint8_t addr, uint8_t data)
{
    uint8_t buffer[2];
    buffer[1] = data;
    SX1276ReadBuffer(addr,buffer,2);
    return buffer[1];
}
void myLora::SX1276WriteFifo(uint8_t *buffer, uint8_t size)
{
    uint8_t data[size+1];
    for(int i =1;i<size;i++){
        data[i] = buffer[i-1];
    }
    SX1276WriteBuffer(REG_LR_FIFO,data,size);
}
void myLora::SX1276ReadFifo(uint8_t *buffer, uint8_t size)
{
    RFBuffer[1] = SX1276ReadBuffer(REG_LR_FIFO,buffer,size);
}
void myLora::SX1276LoRaSetOpMode(uint8_t opMode)
{
    RegOpMode = (RegOpMode & RFLR_OPMODE_MASK) | opMode;
    SX1276Write(REG_LR_OPMODE,RegOpMode);
}
void myLora::SX1276SetSPIState(int state){
    switch(state)
    {
        case 0:
            digitalWrite(TXEN,HIGH);
            digitalWrite(RXEN,LOW);
            break;
        case 1:
            digitalWrite(TXEN,LOW);
            digitalWrite(RXEN,HIGH);
            break;
        case 2:
            digitalWrite(TXEN,LOW);
            digitalWrite(RXEN,LOW);
            break;
    }
}
void myLora::TxSet(uint8_t TxPacketSize,uint8_t *buffer){
    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
    delay(10);
    SX1276Write(REG_LR_IRQFLAGSMASK,0xF7);
    delay(10);
    SX1276Write(REG_LR_HOPPERIOD,0x00);
    delay(10);
    SX1276Write(REG_LR_PAYLOADLENGTH,TxPacketSize);
    delay(10);
    SX1276Write(REG_LR_FIFOTXBASEADDR,0x00);
    delay(10);
    SX1276Write(REG_LR_FIFOADDRPTR,0x00);
    delay(10);
    SX1276WriteFifo(buffer,TxPacketSize);
    delay(10);
    SX1276Write(REG_LR_DIOMAPPING1,0x41);
    delay(10);
}
void myLora::TxStart(){
    uint8_t IRQState;
    SX1276SetSPIState(0);
    delayMicroseconds(10);
    SX1276LoRaSetOpMode(RFLR_OPMODE_TRANSMITTER);
    delay(50);
    while(!(SX1276Read(REG_LR_IRQFLAGS,IRQState) && 0x08));
    SX1276Write(REG_LR_IRQFLAGS,RFLR_IRQFLAGS_TXDONE);
    delay(50);
    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
    qDebug() << "TXdone";
}
void myLora::RxSet(){
    SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
    delay(10);
    SX1276Write(REG_LR_IRQFLAGSMASK,0x9D);
    delay(10);
    SX1276Write(REG_LR_FIFORXBASEADDR,0x00);
    delay(10);
    SX1276Write(REG_LR_FIFOADDRPTR,0x00);
    delay(10);
    SX1276Write(REG_LR_DIOMAPPING1,0x00);
    delay(10);
    SX1276SetSPIState(1);
    delay(10);
    SX1276LoRaSetOpMode(RFLR_OPMODE_RECEIVER);
    RFState = RFLR_STATE_RX_RUNNING;
}
uint8_t myLora::SX1276LoRaGetRFState(){
    return RFState;
}

void myLora::RxProcess(){
    switch(RFState){
        case RFLR_STATE_RX_RUNNING:
            if( SX1276Read(REG_LR_IRQFLAGS,RegIrqFlags) && 0x40)  // RxDone
            {
                SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE  );
                delay(10);
                RFState = RFLR_STATE_RX_DONE;
            }
            break;
        case RFLR_STATE_RX_DONE:
            /*qDebug() << "DoneIRQ" <<SX1276Read(REG_LR_IRQFLAGS,RegIrqFlags);
            RegIrqFlags = SX1276Read(REG_LR_IRQFLAGS,RegIrqFlags);
            delay(10);
            if((RegIrqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR) == RFLR_IRQFLAGS_PAYLOADCRCERROR){

                RFState = RFLR_STATE_RX_RUNNING;
                qDebug() << "CRC ERROR !! ->RX CONTINUE";
                break;
            }*/
            SX1276Write(REG_LR_IRQFLAGS , RFLR_IRQFLAGS_PAYLOADCRCERROR);
            delay(10);
            RegFifoRxCurrentAddr = SX1276Read(REG_LR_FIFORXCURRENTADDR , RegFifoRxCurrentAddr);
            delay(10);
            SX1276Write(REG_LR_FIFOADDRPTR,RegFifoRxCurrentAddr);
            delay(10);
            qDebug()<< "ADDR OK";
            SX1276Read(REG_LR_FIFO,*RFBuffer);
            delay(10);
            qDebug()<< "FiFOGET";
            qDebug()<<"Real Fifo : " << RFBuffer[1];
            RFState = RFLR_STATE_RX_RUNNING;
            qDebug() << "RxDone";
            break;

    }


}
