/********************************************************************************
** Form generated from reading UI file 'paramsSVMregr.ui'
**
** Created: Wed Mar 23 14:24:28 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMSSVMREGR_H
#define UI_PARAMSSVMREGR_H

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

class Ui_ParametersRegr
{
public:
    QLabel *label_3;
    QLabel *svmEpsLabel;
    QComboBox *svmTypeCombo;
    QDoubleSpinBox *svmCSpin;
    QLabel *label_21;
    QDoubleSpinBox *svmPSpin;
    QLabel *label_2;
    QSpinBox *kernelDegSpin;
    QLabel *label_4;
    QLabel *svmCLabel;
    QComboBox *kernelTypeCombo;
    QDoubleSpinBox *kernelWidthSpin;

    void setupUi(QWidget *ParametersRegr)
    {
        if (ParametersRegr->objectName().isEmpty())
            ParametersRegr->setObjectName(QString::fromUtf8("ParametersRegr"));
        ParametersRegr->resize(310, 158);
        label_3 = new QLabel(ParametersRegr);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(130, 20, 31, 16));
        QFont font;
        font.setPointSize(9);
        label_3->setFont(font);
        svmEpsLabel = new QLabel(ParametersRegr);
        svmEpsLabel->setObjectName(QString::fromUtf8("svmEpsLabel"));
        svmEpsLabel->setGeometry(QRect(220, 80, 70, 16));
        svmEpsLabel->setFont(font);
        svmTypeCombo = new QComboBox(ParametersRegr);
        svmTypeCombo->setObjectName(QString::fromUtf8("svmTypeCombo"));
        svmTypeCombo->setGeometry(QRect(20, 100, 91, 22));
        svmTypeCombo->setFont(font);
        svmCSpin = new QDoubleSpinBox(ParametersRegr);
        svmCSpin->setObjectName(QString::fromUtf8("svmCSpin"));
        svmCSpin->setGeometry(QRect(130, 100, 71, 22));
        svmCSpin->setFont(font);
        svmCSpin->setDecimals(1);
        svmCSpin->setMinimum(0.1);
        svmCSpin->setMaximum(9999.9);
        svmCSpin->setSingleStep(1);
        svmCSpin->setValue(1);
        label_21 = new QLabel(ParametersRegr);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(220, 20, 41, 16));
        label_21->setFont(font);
        svmPSpin = new QDoubleSpinBox(ParametersRegr);
        svmPSpin->setObjectName(QString::fromUtf8("svmPSpin"));
        svmPSpin->setGeometry(QRect(220, 100, 71, 22));
        svmPSpin->setFont(font);
        svmPSpin->setDecimals(3);
        svmPSpin->setMinimum(0.001);
        svmPSpin->setMaximum(1);
        svmPSpin->setSingleStep(0.01);
        svmPSpin->setValue(0.03);
        label_2 = new QLabel(ParametersRegr);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 20, 91, 16));
        label_2->setFont(font);
        kernelDegSpin = new QSpinBox(ParametersRegr);
        kernelDegSpin->setObjectName(QString::fromUtf8("kernelDegSpin"));
        kernelDegSpin->setGeometry(QRect(220, 40, 61, 22));
        kernelDegSpin->setFont(font);
        kernelDegSpin->setMinimum(1);
        kernelDegSpin->setValue(2);
        label_4 = new QLabel(ParametersRegr);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 80, 91, 16));
        label_4->setFont(font);
        svmCLabel = new QLabel(ParametersRegr);
        svmCLabel->setObjectName(QString::fromUtf8("svmCLabel"));
        svmCLabel->setGeometry(QRect(130, 80, 60, 16));
        svmCLabel->setFont(font);
        kernelTypeCombo = new QComboBox(ParametersRegr);
        kernelTypeCombo->setObjectName(QString::fromUtf8("kernelTypeCombo"));
        kernelTypeCombo->setGeometry(QRect(20, 40, 91, 22));
        kernelTypeCombo->setFont(font);
        kernelWidthSpin = new QDoubleSpinBox(ParametersRegr);
        kernelWidthSpin->setObjectName(QString::fromUtf8("kernelWidthSpin"));
        kernelWidthSpin->setGeometry(QRect(130, 40, 71, 22));
        kernelWidthSpin->setFont(font);
        kernelWidthSpin->setDecimals(3);
        kernelWidthSpin->setMinimum(0.001);
        kernelWidthSpin->setSingleStep(0.01);
        kernelWidthSpin->setValue(0.1);

        retranslateUi(ParametersRegr);

        svmTypeCombo->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ParametersRegr);
    } // setupUi

    void retranslateUi(QWidget *ParametersRegr)
    {
        ParametersRegr->setWindowTitle(QApplication::translate("ParametersRegr", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ParametersRegr", "Width", 0, QApplication::UnicodeUTF8));
        svmEpsLabel->setText(QApplication::translate("ParametersRegr", "eps", 0, QApplication::UnicodeUTF8));
        svmTypeCombo->clear();
        svmTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("ParametersRegr", "eps-SVR", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersRegr", "nu-SVR", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersRegr", "RVR", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersRegr", "SOGP", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersRegr", "KRLS", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        svmTypeCombo->setToolTip(QApplication::translate("ParametersRegr", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
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
#ifndef QT_NO_TOOLTIP
        svmCSpin->setToolTip(QApplication::translate("ParametersRegr", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">C: cost function (SVR)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Capacity: maximum bases (0=auto/unlimited) (SOGP,KRLS)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">KRLS: capacity of 1 not allowed (switches to 2)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_21->setText(QApplication::translate("ParametersRegr", "Degree", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        svmPSpin->setToolTip(QApplication::translate("ParametersRegr", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
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
        label_2->setText(QApplication::translate("ParametersRegr", "Kernel", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelDegSpin->setToolTip(QApplication::translate("ParametersRegr", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Degree of the kernel polynomial</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">(Polynomial only)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("ParametersRegr", "Method", 0, QApplication::UnicodeUTF8));
        svmCLabel->setText(QApplication::translate("ParametersRegr", "C", 0, QApplication::UnicodeUTF8));
        kernelTypeCombo->clear();
        kernelTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("ParametersRegr", "Linear", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersRegr", "Polynomial", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ParametersRegr", "RBF", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        kernelTypeCombo->setToolTip(QApplication::translate("ParametersRegr", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">kernel function</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        kernelWidthSpin->setToolTip(QApplication::translate("ParametersRegr", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Width of the kernel (gamma)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">RBF and Polynomial only</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class ParametersRegr: public Ui_ParametersRegr {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMSSVMREGR_H
