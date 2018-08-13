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

void MLDemos::ShowOptionCompare()
{
    if (drawToolbar->compareButton->isChecked()) compare->paramsWidget->show();
    else compare->paramsWidget->hide();
}

void MLDemos::ShowGridSearch()
{
    if (drawToolbar->gridsearchButton->isChecked()) gridSearch->show();
    else gridSearch->hide();
}

void MLDemos::ResetGridSearchButton(){
    drawToolbar->gridsearchButton->setChecked(false);
}

void MLDemos::ToggleDataGenerator()
{
    if (drawToolbar->datasetGeneratorButton->isChecked()) generator->show();
    else generator->hide();
}

void MLDemos::ShowOptionDisplay()
{
    if (displayOptions->showOptions->isChecked()) displayDialog->show();
    else displayDialog->hide();
}

void MLDemos::ShowStatsDialog()
{
    if (displayOptions->showStats->isChecked()) statsDialog->show();
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

void MLDemos::HideOptionDisplay()
{
    displayDialog->hide();
    displayOptions->showOptions->setChecked(false);
}

void MLDemos::HideOptionCompare()
{
    compare->paramsWidget->hide();
    drawToolbar->compareButton->setChecked(false);
}

void MLDemos::HideGridSearch()
{
    gridSearch->hide();
    drawToolbar->gridsearchButton->setChecked(false);
}

void MLDemos::HideStatsDialog()
{
    statsDialog->hide();
    displayOptions->showStats->setChecked(false);
}

void MLDemos::HideDataGenerator()
{
    generator->hide();
    drawToolbar->datasetGeneratorButton->setChecked(false);
}

void MLDemos::ShowDataEditor()
{
    dataEdit->show();
}

void MLDemos::ShowEditingTools()
{
    ui.leftPaneWidget->show();
    displayOptions->showSamples->show();
    displayOptions->showOutput->show();
    displayOptions->showModel->show();
    displayOptions->showBackground->show();
    displayOptions->showGrid->show();
    displayOptions->showLegend->show();
    displayOptions->clearAll->show();
    displayOptions->clearData->show();
    displayOptions->fitToViewport->show();
}

void MLDemos::HideEditingTools()
{
    ui.leftPaneWidget->hide();
    displayOptions->showSamples->hide();
    displayOptions->showOutput->hide();
    displayOptions->showModel->hide();
    displayOptions->showBackground->hide();
    displayOptions->showGrid->hide();
    displayOptions->showLegend->hide();
    displayOptions->clearAll->hide();
    displayOptions->clearData->hide();
    displayOptions->fitToViewport->hide();
}
