#include "basewidget.h"

BaseWidget::BaseWidget(QWidget *parent): QWidget(parent)
{
}

BaseWidget::~BaseWidget(){}

void BaseWidget::closeEvent(QCloseEvent * event){
    emit closed();
}
