#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "callrequest.h"
#include "callresponse.h"
#include "callscreen.h"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
   ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_buttonCall_clicked() {
    CallResponse req(ui->contactList->selectedItems()[0]->text());
    int result = req.exec();
    if(result == (int)QDialog::Accepted) {
        CallScreen call;
        call.exec();
    }
}

void MainWindow::on_buttonExit_clicked()
{
    this->close();
}
