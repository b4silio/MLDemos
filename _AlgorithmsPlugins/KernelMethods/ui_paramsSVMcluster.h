/********************************************************************************
** Form generated from reading UI file 'paramsSVMcluster.ui'
**
** Created: Wed Mar 23 14:24:28 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMSSVMCLUSTER_H
#define UI_PARAMSSVMCLUSTER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ParametersClust
{
public:
    QLabel *label_3;
    QSpinBox *kernelDegSpin;
    QLabel *label_2;
    QDoubleSpinBox *kernelWidthSpin;
    QLabel *label_21;
    QComboBox *kernelTypeCombo;
    QComboBox *svmTypeCombo;
    QDoubleSpinBox *svmCSpin;
    QLabel *label_9;
    QLabel *label_14;
    QLabel *label_4;
    QSpinBox *kernelClusterSpin;

    void setupUi(QWidget *ParametersClust)
    {
        if (ParametersClust->objectName().isEmpty())
            ParametersClust->setObjectName(QString::fromUtf8("ParametersClust"));
        ParametersClust->resize(291, 154);
        label_3 = new QLabel(ParametersClust);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(130, 20, 50, 16));
        QFont font;
        font.setPointSize(10);
        label_3->setFont(font);
        kernelDegSpin = new QSpinBox(ParametersClust);
        kernelDegSpin->setObjectName(QString::fromUtf8("kernelDegSpin"));
        kernelDegSpin->setGeometry(QRect(220, 40, 61, 22));
        kernelDegSpin->setFont(font);
        kernelDegSpin->setMinimum(1);
        kernelDegSpin->setValue(2);
        label_2 = new QLabel(ParametersClust);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 20, 91, 16));
        label_2->setFont(font);
        kernelWidthSpin = new QDoubleSpinBox(ParametersClust);
        kernelWidthSpin->setObjectName(QString::fromUtf8("kernelWidthSpin"));
        kernelWidthSpin->setGeometry(QRect(130, 40, 71, 22));
        kernelWidthSpin->setFont(font);
        kernelWidthSpin->setDecimals(3);
        kernelWidthSpin->setMinimum(0.001);
        kernelWidthSpin->setSingleStep(0.01);
        kernelWidthSpin->setValue(0.1);
        label_21 = new QLabel(ParametersClust);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(220, 20, 50, 16));
        label_21->setFont(font);
        kernelTypeCombo = new QComboBox(ParametersClust);
        kernelTypeCombo->setObjectName(QString::fromUtf8("kernelTypeCombo"));
        kernelTypeCombo->setGeometry(QRect(20, 40, 91, 22));
        kernelTypeCombo->setFont(font);
        svmTypeCombo = new QComboBox(ParametersClust);
        svmTypeCombo->setObjectName(QString::fromUtf8("svmTypeCombo"));
        svmTypeCombo->setEnabled(true);
        svmTypeCombo->setGeometry(QRect(20, 100, 101, 22));
        QFont font1;
        font1.setPointSize(9);
        svmTypeCombo->setFont(font1);
        svmTypeCombo->setEditable(false);
        svmCSpin = new QDoubleSpinBox(ParametersClust);
        svmCSpin->setObjectName(QString::fromUtf8("svmCSpin"));
        svmCSpin->setGeometry(QRect(130, 100, 61, 22));
        svmCSpin->setFont(font1);
        svmCSpin->setDecimals(3);
        svmCSpin->setMaximum(1);
        svmCSpin->setSingleStep(0.01);
        svmCSpin->setValue(0.1);
        label_9 = new QLabel(ParametersClust);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(130, 80, 41, 16));
        label_9->setFont(font1);
        label_14 = new QLabel(ParametersClust);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(220, 80, 51, 21));
        label_14->setFont(font1);
        label_4 = new QLabel(ParametersClust);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 80, 31, 16));
        label_4->setFont(font1);
        kernelClusterSpin = new QSpinBox(ParametersClust);
        kernelClusterSpin->setObjectName(QString::fromUtf8("kernelClusterSpin"));
        kernelClusterSpin->setGeometry(QRect(220, 100, 51, 21));
        kernelClusterSpin->setFont(font1);
        kernelClusterSpin->setMinimum(1);
        kernelClusterSpin->setMaximum(20);
        kernelClusterSpin->setValue(2);

        retranslateUi(ParametersClust);

        QMetaObject::connectSlotsByName(ParametersClust);
    } // setupUi

    void retranslateUi(QWidget *ParametersClust)
    {
        ParametersClust->setWindowTitle(QApplication::translate("ParametersClust", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ParametersClust", "Width", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelDegSpin->setToolTip(QApplication::translate("ParametersClust", "Degree of the kernel polynomial\n"
"(Polynomial only)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ParametersClust", "Kernel", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelWidthSpin->setToolTip(QApplication::translate("ParametersClust", "Width of the kernel (gamma)\n"
"RBF and Polynomial only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_21->setText(QApplication::translate("ParametersClust", "Degree", 0, QApplication::UnicodeUTF8));
        kernelTypeCombo->clear();
        kernelTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("ParametersClust", "Linear", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersClust", "Polynomial", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersClust", "RBF", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        kernelTypeCombo->setToolTip(QApplication::translate("ParametersClust", "kernel function", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        svmTypeCombo->clear();
        svmTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("ParametersClust", "One-Class SVM", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersClust", "Kernel K-Means", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        svmTypeCombo->setToolTip(QApplication::translate("ParametersClust", "Clustering method\n"
"One-Class SVM: single-cluster vs outliers clustering method\n"
"Kernel K-Means: K-Means with kernel metrics", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        svmCSpin->setToolTip(QApplication::translate("ParametersClust", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Nu: expected ratio of cluster vs outliers (SVM only)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_9->setText(QApplication::translate("ParametersClust", "Nu", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("ParametersClust", "Clusters", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ParametersClust", "Type", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelClusterSpin->setToolTip(QApplication::translate("ParametersClust", "Number of clusters (Kernel K-Means only)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class ParametersClust: public Ui_ParametersClust {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMSSVMCLUSTER_H
