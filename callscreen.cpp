#include "callscreen.h"
#include "ui_callscreen.h"

CallScreen::CallScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CallScreen)
{
    ui->setupUi(this);
}

CallScreen::~CallScreen()
{
    delete ui;
}
