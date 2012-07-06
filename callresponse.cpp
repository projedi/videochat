#include "callresponse.h"
#include "ui_callresponse.h"

CallResponse::CallResponse(QString contactName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CallResponse)
{
    ui->setupUi(this);
    ui->labelContact->setText(contactName);
    connect(ui->buttonAccept,SIGNAL(clicked()),SLOT(accept()));
    connect(ui->buttonDecline,SIGNAL(clicked()),SLOT(reject()));
}

CallResponse::~CallResponse()
{
    delete ui;
}
