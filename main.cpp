#include <QtWidgets>
#include <QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include "widget.h"
#include <QDebug>
#include <QFile>

bool arduino_is_available = false;
QString arduino_port_name = "";

QSerialPort *arduino = new QSerialPort;
QString serialBuffer = "";

static const quint16 arduino_uno_vendor_id = 10755;
static const quint16 arduino_uno_product_id = 67;

class PeriodicReader : public QObject {

       Q_OBJECT


   QTimer m_timer{this};
   QFile m_file{this};
   QFile startfile;




   void readLine() {
      if (m_file.atEnd()) {
         m_timer.stop();
         return;
      }

      QByteArray lineBaru(m_file.readLine());

      if(!startfile.exists())
      {
      emit newLine(lineBaru);
      qDebug()<<lineBaru;
      QString lineString(lineBaru);
      qDebug()<<lineString << " converted to QString";
      lineString.remove("\n");
      qDebug()<<lineString;
      lineString.remove(" ");
      qDebug()<<lineString;

      updateSpeedometer(lineString);
      }

  }

   void updateSpeedometer(QString command)
   {
       if(arduino->isWritable())

       {
           command.remove(" ");

           arduino->write(command.toStdString().c_str());

          //QThread::msleep(5000);

           qDebug() << command.toStdString().c_str() << " is uploaded to Arduino";

       createStartFile();


       }else{
           qDebug()<<"Couldn't write to Serial !" ;
       }
   }

   void createStartFile()

   {
       qDebug() <<"In Start File";
       startfile.setFileName("G:/CREST/GUI 03112016/Qt 20122016/PeriodicReader/start.txt");

        startfile.open(QIODevice::ReadWrite | QIODevice::Text);
        startfile.close();

        qDebug() << "Start file is created";

    qDebug() <<"Out Start File";
   }



public:
   explicit PeriodicReader(QObject * parent = {}) : QObject(parent) {
      connect(&m_timer, &QTimer::timeout, this, &PeriodicReader::readLine);
   }
   void load(const QString & fileName) {
      m_file.close(); // allow re-opening of the file
      m_file.setFileName(fileName);
      if (m_file.open(QFile::ReadOnly | QFile::Text)) {
         readLine();
         m_timer.start(300); // 0.3s interval
      }
   }
   Q_SIGNAL void newLine(const QByteArray &);
};

QString lineToString(QByteArray line)
   {
   while (line.endsWith('\n') || line.endsWith('\r'))
      line.chop(1);
   return QString::fromUtf8(line);
}

int main(int argc, char ** argv) {

    qDebug()<<"Number of available ports :" <<QSerialPortInfo::availablePorts().length();

      foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
                  qDebug()<<"Has Vendor ID:" << serialPortInfo.hasVendorIdentifier();
                  if(serialPortInfo.hasVendorIdentifier()){
                      qDebug()<<"Vendor ID:"<< serialPortInfo.vendorIdentifier();
                  }
                      qDebug()<<"Has Product ID:" << serialPortInfo.hasProductIdentifier();
                      if(serialPortInfo.hasProductIdentifier()){
                          qDebug()<<"Product ID:"<< serialPortInfo.productIdentifier();
                      }
              }

              foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
                  if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
                      if(serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id){
                          if(serialPortInfo.productIdentifier() == arduino_uno_product_id){
                              arduino_port_name = serialPortInfo.portName();
                              arduino_is_available =  true;
                          }
                      }
                  }
              }

                  if(arduino_is_available){
                      //open and configure the serialport

                      arduino->setPortName(arduino_port_name);
                      arduino->open(QSerialPort::ReadWrite);
                      arduino->setBaudRate(QSerialPort::Baud9600);
                      arduino->setDataBits(QSerialPort::Data8);
                      arduino->setParity(QSerialPort::NoParity);
                      arduino->setStopBits(QSerialPort::OneStop);
                      arduino->setFlowControl(QSerialPort::NoFlowControl);
                      //QObject::connect(arduino,SIGNAL(readyRead()),this,SLOT(readSerial()));

                  }else{
                      //show error message
                      qDebug()<<" Port Error, Couldn't find the Arduino' !";

                   //   QMessageBox::warning(this, "Port Error, Couldn't find the Arduino !");
                  }

   QApplication app{argc, argv};

   QWidget window;
   QVBoxLayout layout{&window};
   QPushButton load{"Load"};
   QPlainTextEdit edit;
   layout.addWidget(&load);
   layout.addWidget(&edit);
   window.show();

   PeriodicReader reader;
   QObject::connect(&load, &QPushButton::clicked, [&]{
      auto name = QFileDialog::getOpenFileName(&window);
      if (!name.isEmpty()) {
         edit.clear(); // allow re-opening of the file
         reader.load(name);
      }
   });
   QObject::connect(&reader, &PeriodicReader::newLine, &edit,
                    [&](const QByteArray & line){ edit.appendPlainText(lineToString(line)); });

   return app.exec();
}
#include "main.moc"
