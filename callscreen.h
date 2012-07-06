#ifndef CALLSCREEN_H
#define CALLSCREEN_H

#include <QDialog>

namespace Ui {
class CallScreen;
}

class CallScreen : public QDialog
{
    Q_OBJECT
    
public:
    explicit CallScreen(QWidget *parent = 0);
    ~CallScreen();
    
private slots:
    void on_buttonEndCall_clicked();

private:
    Ui::CallScreen *ui;
};

#endif // CALLSCREEN_H
