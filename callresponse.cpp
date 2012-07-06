#include "callresponse.h"
#include "ui_callresponse.h"

CallResponse::CallResponse(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CallResponse)
{
    ui->setupUi(this);
}

CallResponse::~CallResponse()
{
    delete ui;
}
