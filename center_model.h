#ifndef CENTER_MODEL_H
#define CENTER_MODEL_H
#include <QtSql>
class MySqlQueryModel : public QSqlQueryModel {
public:
    QVariant data(const QModelIndex& item, int role) const Q_DECL_OVERRIDE
    {
        QVariant value = QSqlQueryModel::data(item, role);
        if (role == Qt::TextAlignmentRole) {
            value = (Qt::AlignCenter);
            return value;
        }
        return value;
    }
};

#endif // CENTER_MODEL_H
