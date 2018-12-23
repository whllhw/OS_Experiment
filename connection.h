#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/*
    This file defines a helper function to open a connection to an
    in-memory SQLITE database and to create a test table.

    If you want to use another database, simply modify the code
    below. All the examples in this directory use this function to
    connect to a database.
*/
//! [0]
static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\n\n"
                        "Click Cancel to exit."),
            QMessageBox::Cancel);
        return false;
    }

    QSqlQuery query;
    query.exec("create table task (pid int primary key, "
               "task_name varchar(20), state varchar(20), priority integer, "
               "total_runtime integer, remain_runtime integer)");
    query.exec("insert into task values(100,'test','TASK_ACCPECT',1000,1000,1000)");
    return true;
}
#define T_pid "pid"
#define T_task_name "task_name"
#define T_state "state"
#define T_priority "priority"
#define T_exec_runtime "total_runtime"
#define T_remain_runtime "remain_runtime"

//! [0]

#endif
