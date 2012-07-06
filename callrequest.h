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
    explicit CallRequest(QWidget *parent = 0);
    ~CallRequest();
    
private:
    Ui::CallRequest *ui;
};

#endif // CALLREQUEST_H
