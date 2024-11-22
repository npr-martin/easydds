#ifndef QOSDIALOG_H
#define QOSDIALOG_H

#include <QDialog>
#include "easyddsType.hpp"

using namespace EASYDDS;

namespace Ui {
class QosDialog;
}

class QosDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QosDialog(qos_profile_s qosProfile, QWidget *parent = nullptr);
    ~QosDialog();

    qos_profile_s getQoSProfile();

private:
    Ui::QosDialog *ui;
};

#endif // QOSDIALOG_H
