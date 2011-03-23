/********************************************************************************
** Form generated from reading UI file 'paramsSVM.ui'
**
** Created: Wed Mar 23 14:24:28 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMSSVM_H
#define UI_PARAMSSVM_H

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

class Ui_Parameters
{
public:
    QLabel *label_3;
    QSpinBox *kernelDegSpin;
    QLabel *label_22;
    QLabel *label_2;
    QLabel *svmTypeLabel;
    QComboBox *svmTypeCombo;
    QLabel *label_4;
    QDoubleSpinBox *kernelWidthSpin;
    QLabel *label_21;
    QSpinBox *maxSVSpin;
    QComboBox *kernelTypeCombo;
    QDoubleSpinBox *svmCSpin;

    void setupUi(QWidget *Parameters)
    {
        if (Parameters->objectName().isEmpty())
            Parameters->setObjectName(QString::fromUtf8("Parameters"));
        Parameters->resize(310, 158);
        label_3 = new QLabel(Parameters);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(130, 20, 50, 16));
        QFont font;
        font.setPointSize(10);
        label_3->setFont(font);
        kernelDegSpin = new QSpinBox(Parameters);
        kernelDegSpin->setObjectName(QString::fromUtf8("kernelDegSpin"));
        kernelDegSpin->setGeometry(QRect(220, 40, 61, 22));
        kernelDegSpin->setFont(font);
        kernelDegSpin->setMinimum(1);
        kernelDegSpin->setValue(2);
        label_22 = new QLabel(Parameters);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setGeometry(QRect(230, 80, 50, 16));
        label_22->setFont(font);
        label_2 = new QLabel(Parameters);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 20, 91, 16));
        label_2->setFont(font);
        svmTypeLabel = new QLabel(Parameters);
        svmTypeLabel->setObjectName(QString::fromUtf8("svmTypeLabel"));
        svmTypeLabel->setGeometry(QRect(150, 80, 51, 16));
        svmTypeLabel->setFont(font);
        svmTypeCombo = new QComboBox(Parameters);
        svmTypeCombo->setObjectName(QString::fromUtf8("svmTypeCombo"));
        svmTypeCombo->setGeometry(QRect(50, 100, 71, 22));
        svmTypeCombo->setFont(font);
        label_4 = new QLabel(Parameters);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(50, 80, 61, 16));
        label_4->setFont(font);
        kernelWidthSpin = new QDoubleSpinBox(Parameters);
        kernelWidthSpin->setObjectName(QString::fromUtf8("kernelWidthSpin"));
        kernelWidthSpin->setGeometry(QRect(130, 40, 71, 22));
        kernelWidthSpin->setFont(font);
        kernelWidthSpin->setDecimals(3);
        kernelWidthSpin->setMinimum(0.001);
        kernelWidthSpin->setSingleStep(0.01);
        kernelWidthSpin->setValue(0.1);
        label_21 = new QLabel(Parameters);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(220, 20, 50, 16));
        label_21->setFont(font);
        maxSVSpin = new QSpinBox(Parameters);
        maxSVSpin->setObjectName(QString::fromUtf8("maxSVSpin"));
        maxSVSpin->setEnabled(false);
        maxSVSpin->setGeometry(QRect(230, 100, 41, 22));
        maxSVSpin->setFont(font);
        maxSVSpin->setMinimum(2);
        maxSVSpin->setMaximum(999);
        maxSVSpin->setValue(2);
        kernelTypeCombo = new QComboBox(Parameters);
        kernelTypeCombo->setObjectName(QString::fromUtf8("kernelTypeCombo"));
        kernelTypeCombo->setGeometry(QRect(20, 40, 91, 22));
        kernelTypeCombo->setFont(font);
        svmCSpin = new QDoubleSpinBox(Parameters);
        svmCSpin->setObjectName(QString::fromUtf8("svmCSpin"));
        svmCSpin->setGeometry(QRect(140, 100, 71, 22));
        svmCSpin->setFont(font);
        svmCSpin->setDecimals(3);
        svmCSpin->setMinimum(0.001);
        svmCSpin->setMaximum(999.99);
        svmCSpin->setSingleStep(0.01);
        svmCSpin->setValue(1);

        retranslateUi(Parameters);

        QMetaObject::connectSlotsByName(Parameters);
    } // setupUi

    void retranslateUi(QWidget *Parameters)
    {
        Parameters->setWindowTitle(QApplication::translate("Parameters", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Parameters", "Width", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelDegSpin->setToolTip(QApplication::translate("Parameters", "Degree of the kernel polynomial\n"
"(Polynomial only)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_22->setText(QApplication::translate("Parameters", "max SV", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Parameters", "Kernel", 0, QApplication::UnicodeUTF8));
        svmTypeLabel->setText(QApplication::translate("Parameters", "C", 0, QApplication::UnicodeUTF8));
        svmTypeCombo->clear();
        svmTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("Parameters", "C-SVM", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Parameters", "nu-SVM", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Parameters", "RVM", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Parameters", "Pegasos", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        svmTypeCombo->setToolTip(QApplication::translate("Parameters", "Type of SVM training:\n"
"C-SVM (or eps-SVM): C cost based SVM, trained via SMO\n"
"Nu-SVM: Nu based SVM, trained via SMO\n"
"RVM: Relevant Vector Machine\n"
"Pegasos: Online trainer for SVM", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("Parameters", "Type", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        kernelWidthSpin->setToolTip(QApplication::translate("Parameters", "Width of the kernel (gamma)\n"
"RBF and Polynomial only", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_21->setText(QApplication::translate("Parameters", "Degree", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        maxSVSpin->setToolTip(QApplication::translate("Parameters", "Maximal number of support vectors\n"
"(Pegasos only)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        kernelTypeCombo->clear();
        kernelTypeCombo->insertItems(0, QStringList()
         << QApplication::translate("Parameters", "Linear", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Parameters", "Polynomial", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Parameters", "RBF", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        kernelTypeCombo->setToolTip(QApplication::translate("Parameters", "kernel function", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        svmCSpin->setToolTip(QApplication::translate("Parameters", "Training Parameter\n"
"C-SVM: C cost function, (alpha_i <= C)\n"
"Nu-SVM: nu, ratio on the amount of alphas that can be selected\n"
"RVM: eps, stopping criterion (the smaller, the more accurate)\n"
"Pegasos: lambda, accuracy-generalization tradeoff parameter ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class Parameters: public Ui_Parameters {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMSSVM_H
