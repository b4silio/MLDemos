/********************************************************************************
** Form generated from reading UI file 'paramsSVMdynamic.ui'
**
** Created: Wed Mar 23 14:24:28 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMSSVMDYNAMIC_H
#define UI_PARAMSSVMDYNAMIC_H

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

class Ui_ParametersDynamic
{
public:
    QLabel *label_21;
    QLabel *svmCLabel;
    QDoubleSpinBox *kernelWidthSpin;
    QDoubleSpinBox *svmCSpin;
    QLabel *label_3;
    QDoubleSpinBox *svmPSpin;
    QLabel *label_2;
    QSpinBox *kernelDegSpin;
    QComboBox *svmTypeCombo;
    QLabel *svmEpsLabel;
    QLabel *label_4;
    QComboBox *kernelTypeCombo;

    void setupUi(QWidget *ParametersDynamic)
    {
        if (ParametersDynamic->objectName().isEmpty())
            ParametersDynamic->setObjectName(QString::fromUtf8("ParametersDynamic"));
        ParametersDynamic->resize(328, 157);
        label_21 = new QLabel(ParametersDynamic);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(220, 30, 41, 16));
        QFont font;
        font.setPointSize(9);
        label_21->setFont(font);
        svmCLabel = new QLabel(ParametersDynamic);
        svmCLabel->setObjectName(QString::fromUtf8("svmCLabel"));
        svmCLabel->setGeometry(QRect(130, 90, 60, 16));
        svmCLabel->setFont(font);
        kernelWidthSpin = new QDoubleSpinBox(ParametersDynamic);
        kernelWidthSpin->setObjectName(QString::fromUtf8("kernelWidthSpin"));
        kernelWidthSpin->setGeometry(QRect(130, 50, 71, 22));
        kernelWidthSpin->setFont(font);
        kernelWidthSpin->setDecimals(3);
        kernelWidthSpin->setMinimum(0.001);
        kernelWidthSpin->setSingleStep(0.01);
        kernelWidthSpin->setValue(0.1);
        svmCSpin = new QDoubleSpinBox(ParametersDynamic);
        svmCSpin->setObjectName(QString::fromUtf8("svmCSpin"));
        svmCSpin->setGeometry(QRect(130, 110, 71, 22));
        svmCSpin->setFont(font);
        svmCSpin->setDecimals(1);
        svmCSpin->setMinimum(0.1);
        svmCSpin->setMaximum(9999.9);
        svmCSpin->setSingleStep(1);
        svmCSpin->setValue(1);
        label_3 = new QLabel(ParametersDynamic);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(130, 30, 31, 16));
        label_3->setFont(font);
        svmPSpin = new QDoubleSpinBox(ParametersDynamic);
        svmPSpin->setObjectName(QString::fromUtf8("svmPSpin"));
        svmPSpin->setGeometry(QRect(220, 110, 71, 22));
        svmPSpin->setFont(font);
        svmPSpin->setDecimals(3);
        svmPSpin->setMinimum(0.001);
        svmPSpin->setMaximum(1);
        svmPSpin->setSingleStep(0.01);
        svmPSpin->setValue(0.03);
        label_2 = new QLabel(ParametersDynamic);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 30, 91, 16));
        label_2->setFont(font);
        kernelDegSpin = new QSpinBox(ParametersDynamic);
        kernelDegSpin->setObjectName(QString::fromUtf8("kernelDegSpin"));
        kernelDegSpin->setGeometry(QRect(220, 50, 61, 22));
        kernelDegSpin->setFont(font);
        kernelDegSpin->setMinimum(1);
        kernelDegSpin->setValue(2);
        svmTypeCombo = new QComboBox(ParametersDynamic);
        svmTypeCombo->setObjectName(QString::fromUtf8("svmTypeCombo"));
        svmTypeCombo->setGeometry(QRect(20, 110, 91, 22));
        svmTypeCombo->setFont(font);
        svmEpsLabel = new QLabel(ParametersDynamic);
        svmEpsLabel->setObjectName(QString::fromUtf8("svmEpsLabel"));
        svmEpsLabel->setGeometry(QRect(220, 90, 70, 16));
        svmEpsLabel->setFont(font);
        label_4 = new QLabel(ParametersDynamic);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 90, 91, 16));
        label_4->setFont(font);
        kernelTypeCombo = new QComboBox(ParametersDynamic);
        kernelTypeCombo->setObjectName(QString::fromUtf8("kernelTypeCombo"));
        kernelTypeCombo->setGeometry(QRect(20, 50, 91, 22));
        kernelTypeCombo->setFont(font);

        retranslateUi(ParametersDynamic);

        svmTypeCombo->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ParametersDynamic);
    } // setupUi

    void retranslateUi(QWidget *ParametersDynamic)
    {
        ParametersDynamic->setWindowTitle(QApplication::translate("ParametersDynamic", "Form", 0, QApplication::UnicodeUTF8));
        label_21->setText(QApplication::translate("ParametersDynamic", "Degree", 0, QApplication::UnicodeUTF8));
        svmCLabel->setText(QApplication::translate("ParametersDynamic", "C", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelWidthSpin->setToolTip(QApplication::translate("ParametersDynamic", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Width of the kernel (gamma)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">RBF and Polynomial only</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        svmCSpin->setToolTip(QApplication::translate("ParametersDynamic", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">C: cost function (SVR)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Capacity: maximum bases (0=auto/unlimited) (SOGP,KRLS)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">KRLS: capacity of 1 not allowed (switches to 2)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("ParametersDynamic", "Width", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        svmPSpin->setToolTip(QApplication::translate("ParametersDynamic", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">eps-SVR: epsilon-tube width</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">nu-SVR: nu ratio on alpha</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">RVR: accuracy-generalization tradeoff</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt"
                        "-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">SOGP: noise</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">KRLS: tolerance (stopping criterion)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ParametersDynamic", "Kernel", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelDegSpin->setToolTip(QApplication::translate("ParametersDynamic", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Degree of the kernel polynomial</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">(Polynomial only)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        svmTypeCombo->clear();
        svmTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("ParametersDynamic", "eps-SVR", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersDynamic", "nu-SVR", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersDynamic", "SOGP", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        svmTypeCombo->setToolTip(QApplication::translate("ParametersDynamic", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Kernel method for regression:</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">eps-SVM: epsilon-based SVR, trained via SMO</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Nu-SVM: Nu based SVM, trained via SMO</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; m"
                        "argin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">RVR: Relevant Vector Regression</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">SOGP: Sparse Online Gaussian Processes</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">KRLS: Kernel Recursive Least Squares</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        svmEpsLabel->setText(QApplication::translate("ParametersDynamic", "eps", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ParametersDynamic", "Method", 0, QApplication::UnicodeUTF8));
        kernelTypeCombo->clear();
        kernelTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("ParametersDynamic", "Linear", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersDynamic", "Polynomial", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersDynamic", "RBF", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        kernelTypeCombo->setToolTip(QApplication::translate("ParametersDynamic", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">kernel function</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class ParametersDynamic: public Ui_ParametersDynamic {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMSSVMDYNAMIC_H
