#include "mainwindow.h"
#include "ui_mainwindow.h"

static myLora Lora;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Lora.LoraInit();
    delay(10);
    Lora.RxSet();
    delay(10);
    Timer1 = new QTimer(this);
    Timer1->start(100);
    QObject::connect(Timer1,SIGNAL(timeout()),this,SLOT(RxLoop()));
}
void MainWindow::RxLoop()
{
    Lora.RxProcess();
    delay(20);
    if(Lora.SX1276LoRaGetRFState() == 3)
    {
        Lora.RxProcess();
        delay(30);
        qDebug() << "Rx";
        ui->textEdit->append("DataReceive");
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QByteArray byteArray = ui->lineEdit->text().toLocal8Bit();
    char *c  = byteArray.data();
    uint8_t BUFFER[strlen(c)] = {0};
    for(int i=0 ; i < strlen(c) ; i++){
        switch (c[i]){
            case 'a':
                BUFFER[i] = 0x01;
            break;
            case 'b':
                BUFFER[i] = 0xF0;
            break;
            case 'c':
                BUFFER[i] = 0x03;
            break;
        default:
            break;
        }
    }
    Lora.TxSet((uint8_t)strlen(c),BUFFER);
    Lora.TxStart();
    ui->lineEdit->clear();
}
