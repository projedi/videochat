#ifndef CALLREQUEST_H
#define CALLREQUEST_H

#include <QDialog>

namespace Ui {
class CallRequest;
}

class CallRequest : public QDialog
{
    Q_OBJECT
    
public:
    explicit CallRequest(QString contactName, QWidget *parent = 0);
    ~CallRequest();
    
private slots:
    void on_buttonAbort_clicked();

private:
    Ui::CallRequest *ui;
};

#endif // CALLREQUEST_H
