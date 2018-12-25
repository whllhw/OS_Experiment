#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>

Dialog::Dialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_buttonBox_accepted()
{
    //    QMessageBox::information(this, tr("title"), tr("ok"));
}

task_struct* Dialog::get_task_struct()
{
    task_struct* task = new task_struct(
        ui->lineEdit->text(),
        ui->spinBox->value(),
        ui->lineEdit_3->text().toUInt());
    return task;
}
