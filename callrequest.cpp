#include "callrequest.h"
#include "ui_callrequest.h"

CallRequest::CallRequest(QString contactName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CallRequest)
{
    ui->setupUi(this);
    ui->labelContact->setText(contactName);
}

CallRequest::~CallRequest()
{
    delete ui;
}

void CallRequest::on_buttonAbort_clicked()
{
    this->close();
}
