/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "mldemos.h"

void MLDemos::ShowAlgorithmOptions()
{
    if (actionAlgorithms->isChecked()){
        algo->algorithmWidget->show();
        algo->SetAlgorithmWidget();
    }else{ algo->algorithmWidget->hide();

    }
}

void MLDemos::RestAlgorithmOptionsButton(){
    actionAlgorithms->setChecked(false);
}

void MLDemos::ShowOptionCompare()
{
    if (actionCompare->isChecked()) compare->paramsWidget->show();
    else compare->paramsWidget->hide();
}

void MLDemos::ShowGridSearch()
{
    if (actionGridsearch->isChecked()) gridSearch->show();
    else gridSearch->hide();
}

void MLDemos::ResetGridSearchButton(){
    actionGridsearch->setChecked(false);
}

void MLDemos::ShowSampleDrawing()
{
    if (actionDrawSamples->isChecked()) drawToolbarWidget->show();
    else drawToolbarWidget->hide();
}

void MLDemos::ShowAddData()
{
    if (actionAddData->isChecked()) generator->show();
    else generator->hide();
}

void MLDemos::HideAddData()
{
    generator->hide();
    actionAddData->setChecked(false);
}

void MLDemos::ShowOptionDisplay()
{
    if (actionDisplayOptions->isChecked()) displayDialog->show();
    else displayDialog->hide();
}

void MLDemos::ShowToolbar()
{
    if (ui.actionSmall_Icons->isChecked())
    {
        toolBar->setIconSize(QSize(32,32));
        toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    else
    {
        toolBar->setIconSize(QSize(64,64));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    if (ui.actionShow_Toolbar->isChecked()) toolBar->show();
    else toolBar->hide();
}

void MLDemos::ShowStatsDialog()
{
    if (actionShowStats->isChecked()) statsDialog->show();
    else statsDialog->hide();
}

void MLDemos::ShowAbout()
{
    aboutDialog->show();
}

void MLDemos::ShowManualSelection()
{
    manualSelectDialog->show();
}

void MLDemos::ShowInputDimensions()
{
    inputDimensionsDialog->show();
}

void MLDemos::HideSampleDrawing()
{
    drawToolbarWidget->hide();
    actionDrawSamples->setChecked(false);
}

void MLDemos::HideOptionDisplay()
{
    displayDialog->hide();
    actionDisplayOptions->setChecked(false);
}

void MLDemos::HideOptionCompare()
{
    compare->paramsWidget->hide();
    actionCompare->setChecked(false);
}

void MLDemos::HideToolbar()
{
    toolBar->hide();
    ui.actionShow_Toolbar->setChecked(false);
}

void MLDemos::HideStatsDialog()
{
    statsDialog->hide();
    actionShowStats->setChecked(false);
}

void MLDemos::ShowDataEditor()
{
    dataEdit->show();
}
