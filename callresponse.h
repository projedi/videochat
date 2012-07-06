#ifndef CALLRESPONSE_H
#define CALLRESPONSE_H

#include <QDialog>

namespace Ui {
class CallResponse;
}

class CallResponse : public QDialog
{
    Q_OBJECT
    
public:
    explicit CallResponse(QWidget *parent = 0);
    ~CallResponse();
    
private:
    Ui::CallResponse *ui;
};

#endif // CALLRESPONSE_H
