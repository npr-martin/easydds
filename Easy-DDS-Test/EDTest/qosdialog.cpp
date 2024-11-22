#include "qosdialog.h"
#include "ui_qosdialog.h"

#include <QPushButton>

QosDialog::QosDialog(qos_profile_s qosProfile, QWidget *parent)
    : QDialog(parent), ui(new Ui::QosDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    ui->cmbHistory->setCurrentIndex(qosProfile.history);
    ui->leDepth->setText(QString::number(qosProfile.depth));
    ui->leSamples->setText(QString::number(qosProfile.samples));
    ui->cmbReliability->setCurrentIndex(qosProfile.reliability - 1);
    ui->cmbDurability->setCurrentIndex(qosProfile.durability);
    ui->leDdlSec->setText(QString::number(qosProfile.deadline.sec));
    ui->leDdlNsec->setText(QString::number(qosProfile.deadline.nsec));
    ui->leLifespanSec->setText(QString::number(qosProfile.lifespan.sec));
    ui->leLifespanNsec->setText(QString::number(qosProfile.lifespan.nsec));
    ui->cmbLiveliness->setCurrentIndex(qosProfile.liveliness);
    ui->leLDSec->setText(QString::number(qosProfile.liveliness_lease_duration.sec));
    ui->leLDNsec->setText(QString::number(qosProfile.liveliness_lease_duration.nsec));
}

QosDialog::~QosDialog()
{
    delete ui;
}

qos_profile_s QosDialog::getQoSProfile()
{
    qos_profile_s qosProfile;
    qosProfile.history = static_cast<HistoryQosPolicyKind>(ui->cmbHistory->currentIndex());
    qosProfile.depth = ui->leDepth->text().toULong();
    qosProfile.samples = ui->leSamples->text().toUInt();
    qosProfile.reliability = static_cast<ReliabilityQosPolicyKind>(ui->cmbReliability->currentIndex() + 1);
    qosProfile.durability = static_cast<DurabilityQosPolicyKind>(ui->cmbDurability->currentIndex());
    qosProfile.deadline = {ui->leDdlSec->text().toInt(), ui->leLDNsec->text().toUInt()};
    qosProfile.lifespan = {ui->leLifespanSec->text().toInt(), ui->leLifespanNsec->text().toUInt()};
    qosProfile.liveliness = static_cast<LivelinessQosPolicyKind>(ui->cmbLiveliness->currentIndex());
    qosProfile.liveliness_lease_duration = {ui->leLDSec->text().toInt(), ui->leLDNsec->text().toUInt()};

    return qosProfile;
}
