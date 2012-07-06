#include "callrequest.h"
#include "ui_callrequest.h"

CallRequest::CallRequest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CallRequest)
{
    ui->setupUi(this);
}

CallRequest::~CallRequest()
{
    delete ui;
}
