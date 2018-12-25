#ifndef DIALOG_H
#define DIALOG_H

#include "pcb.h"
#include <QDialog>
namespace Ui {
class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT

public:
    explicit Dialog(QWidget* parent = nullptr);
    ~Dialog();
    task_struct* get_task_struct();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Dialog* ui;
};

#endif // DIALOG_H
