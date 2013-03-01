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
#include <QDebug>
#include <fstream>
#include <QPixmap>
#include <QBitmap>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include "classifier.h"
#include "regressor.h"
#include <fstream>

using namespace std;

void MLDemos::SaveLayoutOptions()
{
    QCoreApplication::setOrganizationDomain("b4silio");
    QCoreApplication::setOrganizationName("b4silio");
    QCoreApplication::setApplicationName("MLDemos");

    QSettings settings;
    settings.beginGroup("Gui");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("algoGeometry", algorithmWidget->saveGeometry());
    settings.setValue("drawGeometry", drawToolbarWidget->saveGeometry());
    settings.setValue("displayGeometry", displayDialog->saveGeometry());
    settings.setValue("statsGeometry", statsDialog->saveGeometry());
    settings.setValue("compareGeometry", compareWidget->saveGeometry());
    settings.setValue("generatorGeometry", generator->saveGeometry());

    settings.setValue("algoTab", algorithmOptions->tabWidget->currentIndex());
    settings.setValue("ShowAlgoOptions", algorithmWidget->isVisible());
    settings.setValue("ShowCompare", compareWidget->isVisible());
    settings.setValue("ShowDrawOptions", drawToolbarWidget->isVisible());
    settings.setValue("ShowDisplayOptions", displayDialog->isVisible());
    settings.setValue("ShowStatsOptions", statsDialog->isVisible());
    settings.setValue("ShowToolbar", ui.actionShow_Toolbar->isChecked());
    settings.setValue("SmallIcons", ui.actionSmall_Icons->isChecked());
    settings.setValue("ShowGenerator", generator->isVisible());
    //    settings.setValue("canvasType", ui.canvasTypeCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("displayOptions");
    settings.setValue("infoCheck", displayOptions->infoCheck->isChecked());
    settings.setValue("mapCheck", displayOptions->mapCheck->isChecked());
    settings.setValue("modelCheck", displayOptions->modelCheck->isChecked());
    settings.setValue("samplesCheck", displayOptions->samplesCheck->isChecked());
    settings.setValue("gridCheck", displayOptions->gridCheck->isChecked());
    settings.setValue("spinZoom", displayOptions->spinZoom->value());
    settings.setValue("legendCheck", displayOptions->legendCheck->isChecked());
    settings.setValue("check3DSamples", displayOptions->check3DSamples->isChecked());
    settings.setValue("check3DWireframe", displayOptions->check3DWireframe->isChecked());
    settings.setValue("check3DSurfaces", displayOptions->check3DSurfaces->isChecked());
    settings.setValue("check3DTransparency", displayOptions->check3DTransparency->isChecked());
    settings.setValue("check3DBlurry", displayOptions->check3DBlurry->isChecked());
    settings.setValue("check3DRotate", displayOptions->check3DRotate->isChecked());
    settings.endGroup();

    settings.beginGroup("drawingOptions");
    settings.setValue("infoCheck", drawToolbarContext1->randCombo->currentIndex());
    settings.setValue("spinCount", drawToolbarContext1->spinCount->value());
    settings.setValue("spinSize", drawToolbar->radiusSpin->value());
    settings.setValue("spinAngle", drawToolbarContext2->spinAngle->value());
    settings.setValue("spinSigmaX", drawToolbarContext2->spinSigmaX->value());
    settings.setValue("spinSigmaY", drawToolbarContext2->spinSigmaY->value());
    settings.setValue("spinObsAngle", drawToolbarContext3->spinAngle->value());
    settings.setValue("spinObsSigmaX", drawToolbarContext3->spinSigmaX->value());
    settings.setValue("spinObsSigmaY", drawToolbarContext3->spinSigmaY->value());
    settings.setValue("spinObsPowerX", drawToolbarContext3->spinPowerX->value());
    settings.setValue("spinObsPowerY", drawToolbarContext3->spinPowerY->value());
    settings.setValue("spinObsRepulsionX", drawToolbarContext3->spinRepulsionX->value());
    settings.setValue("spinObsRepulsionY", drawToolbarContext3->spinRepulsionY->value());
    settings.setValue("spinRadius", drawToolbarContext4->spinRadius->value());
    settings.setValue("spinAlpha", drawToolbarContext4->spinAlpha->value());
    settings.setValue("eraseCheck", drawToolbar->eraseButton->isChecked());
    settings.setValue("sprayCheck", drawToolbar->sprayButton->isChecked());
    settings.setValue("spray3DCheck", drawToolbar->spray3DButton->isChecked());
    settings.setValue("singleCheck", drawToolbar->singleButton->isChecked());
    settings.setValue("ellipseCheck", drawToolbar->ellipseButton->isChecked());
    settings.setValue("lineCheck", drawToolbar->lineButton->isChecked());
    settings.setValue("trajectoryCheck", drawToolbar->trajectoryButton->isChecked());
    settings.setValue("obstacleCheck", drawToolbar->obstacleButton->isChecked());
    settings.setValue("paintCheck", drawToolbar->paintButton->isChecked());
    settings.setValue("infoCheck", drawToolbarContext1->randCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("classificationOptions");
    settings.setValue("positiveClass", optionsClassify->positiveSpin->value());
    settings.setValue("binaryCheck", optionsClassify->binaryCheck->isChecked());
    settings.setValue("trainRatio", optionsClassify->traintestRatioCombo->currentIndex());
    settings.setValue("tab", optionsClassify->algoList->currentIndex());
    settings.endGroup();

    settings.beginGroup("regressionOptions");
    settings.setValue("trainRatio", optionsRegress->traintestRatioCombo->currentIndex());
    settings.setValue("tab", optionsRegress->algoList->currentIndex());
    settings.endGroup();

    settings.beginGroup("dynamicalOptions");
    settings.setValue("centerType", optionsDynamic->centerCombo->currentIndex());
    settings.setValue("zeroCheck", optionsDynamic->zeroCheck->isChecked());
    settings.setValue("resampleType", optionsDynamic->resampleCombo->currentIndex());
    settings.setValue("resampleCount", optionsDynamic->resampleSpin->value());
    settings.setValue("obstacleType", optionsDynamic->obstacleCombo->currentIndex());
    settings.setValue("dT", optionsDynamic->dtSpin->value());
    settings.setValue("tab", optionsDynamic->algoList->currentIndex());
    settings.setValue("colorCheck", optionsDynamic->colorCheck->isChecked());
    settings.endGroup();

    settings.beginGroup("compareOptions");
    settings.setValue("foldCount", optionsCompare->foldCountSpin->value());
    settings.setValue("trainRatio", optionsCompare->traintestRatioCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("clusterOptions");
    settings.setValue("tab", optionsCluster->algoList->currentIndex());
    settings.setValue("trainRatio", optionsCluster->trainRatioCombo->currentIndex());
    settings.setValue("trainTestCombo", optionsCluster->trainTestCombo->currentIndex());
    settings.setValue("optimizeCombo", optionsCluster->optimizeCombo->currentIndex());
    settings.setValue("rangeStart", optionsCluster->rangeStartSpin->value());
    settings.setValue("rangeStop", optionsCluster->rangeStopSpin->value());
    settings.endGroup();

    settings.beginGroup("maximizeOptions");
    settings.setValue("tab", optionsMaximize->algoList->currentIndex());
    settings.setValue("varianceSpin", optionsMaximize->varianceSpin->value());
    settings.setValue("iterationsSpin", optionsMaximize->iterationsSpin->value());
    settings.setValue("stoppingSpin", optionsMaximize->stoppingSpin->value());
    settings.setValue("benchmarkCombo", optionsMaximize->benchmarkCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("reinforceOptions");
    settings.setValue("tab", optionsReinforcement->algoList->currentIndex());
    settings.setValue("varianceSpin", optionsReinforcement->varianceSpin->value());
    settings.setValue("iterationsSpin", optionsReinforcement->iterationsSpin->value());
    settings.setValue("displayIterationSpin", optionsReinforcement->displayIterationSpin->value());
    settings.setValue("problemCombo", optionsReinforcement->problemCombo->currentIndex());
    settings.setValue("rewardCombo", optionsReinforcement->rewardCombo->currentIndex());
    settings.setValue("policyCombo", optionsReinforcement->policyCombo->currentIndex());
    settings.setValue("quantizeCombo", optionsReinforcement->quantizeCombo->currentIndex());
    settings.setValue("resolutionSpin", optionsReinforcement->resolutionSpin->value());
    settings.setValue("benchmarkCombo", optionsReinforcement->benchmarkCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("projectOptions");
    settings.setValue("tab", optionsProject->algoList->currentIndex());
    settings.setValue("fitCheck", optionsProject->fitCheck->isChecked());
    settings.endGroup();

    settings.beginGroup("statsOptions");
    settings.setValue("tab", showStats->tabWidget->currentIndex());
    settings.endGroup();

    settings.beginGroup("generatorOptions");
    settings.setValue("generatorCombo", generator->ui->generatorCombo->currentIndex());
    settings.setValue("countSpin", generator->ui->countSpin->value());
    settings.setValue("dimSpin", generator->ui->dimSpin->value());
    settings.setValue("gridCountSpin", generator->ui->gridCountSpin->value());
    settings.setValue("classesCount", generator->ui->classesCount->value());
    settings.setValue("radiusSpin", generator->ui->radiusSpin->value());
    settings.endGroup();

    FOR(i,classifiers.size())
    {
        if(!classifiers[i]) continue;
        settings.beginGroup(QString("plugins::classifiers::") + classifiers[i]->GetName());
        classifiers[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,clusterers.size())
    {
        if(!clusterers[i]) continue;
        settings.beginGroup(QString("plugins::clusterers::") + clusterers[i]->GetName());
        clusterers[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,regressors.size())
    {
        if(!regressors[i]) continue;
        settings.beginGroup(QString("plugins::regressors::") + regressors[i]->GetName());
        regressors[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,dynamicals.size())
    {
        if(!dynamicals[i]) continue;
        settings.beginGroup(QString("plugins::dynamicals::") + dynamicals[i]->GetName());
        dynamicals[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,maximizers.size())
    {
        if(!maximizers[i]) continue;
        settings.beginGroup(QString("plugins::maximizers::") + maximizers[i]->GetName());
        maximizers[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,reinforcements.size())
    {
        if(!reinforcements[i]) continue;
        settings.beginGroup(QString("plugins::reinforcements::") + reinforcements[i]->GetName());
        reinforcements[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,projectors.size())
    {
        if(!projectors[i]) continue;
        settings.beginGroup(QString("plugins::projectors::") + projectors[i]->GetName());
        projectors[i]->SaveOptions(settings);
        settings.endGroup();
    }
}

void MLDemos::LoadLayoutOptions()
{
    QCoreApplication::setOrganizationDomain("b4silio");
    QCoreApplication::setOrganizationName("b4silio");
    QCoreApplication::setApplicationName("MLDemos");

    QSettings settings;
    settings.beginGroup("Gui");
    if(settings.contains("geometry")) restoreGeometry(settings.value("geometry").toByteArray());
    if(settings.contains("windowState")) restoreState(settings.value("windowState").toByteArray());
    if(settings.contains("algoGeometry")) algorithmWidget->restoreGeometry(settings.value("algoGeometry").toByteArray());
    if(settings.contains("drawGeometry")) drawToolbarWidget->restoreGeometry(settings.value("drawGeometry").toByteArray());
    if(settings.contains("displayGeometry")) displayDialog->restoreGeometry(settings.value("displayGeometry").toByteArray());
    if(settings.contains("statsGeometry")) statsDialog->restoreGeometry(settings.value("statsGeometry").toByteArray());
    if(settings.contains("compareGeometry")) compareWidget->restoreGeometry(settings.value("compareGeometry").toByteArray());
    if(settings.contains("generatorGeometry")) generator->restoreGeometry(settings.value("generatorGeometry").toByteArray());
#ifdef MACX // ugly hack to avoid resizing problems on the mac
    if(height() < 400) resize(width(),400);
    if(algorithmWidget->height() < 220) algorithmWidget->resize(636,220);
#endif // MACX

    if(settings.contains("algoTab")) algorithmOptions->tabWidget->setCurrentIndex(settings.value("algoTab").toInt());
    if(settings.contains("ShowAlgoOptions")) algorithmWidget->setVisible(settings.value("ShowAlgoOptions").toBool());
    if(settings.contains("ShowCompare")) compareWidget->setVisible(settings.value("ShowCompare").toBool());
    if(settings.contains("ShowDrawOptions")) drawToolbarWidget->setVisible(settings.value("ShowDrawOptions").toBool());
    if(settings.contains("ShowDisplayOptions")) displayDialog->setVisible(settings.value("ShowDisplayOptions").toBool());
    if(settings.contains("ShowStatsOptions")) statsDialog->setVisible(settings.value("ShowStatsOptions").toBool());
    if(settings.contains("ShowToolbar")) ui.actionShow_Toolbar->setChecked(settings.value("ShowToolbar").toBool());
    if(settings.contains("SmallIcons")) ui.actionSmall_Icons->setChecked(settings.value("SmallIcons").toBool());
    if(settings.contains("ShowGenerator")) generator->setVisible(settings.value("ShowGenerator").toBool());
    //    if(settings.contains("canvasType")) ui.canvasTypeCombo->setCurrentIndex(settings.value("canvasType").toInt());
    settings.endGroup();

    actionAlgorithms->setChecked(algorithmWidget->isVisible());
    actionCompare->setChecked(compareWidget->isVisible());
    actionDrawSamples->setChecked(drawToolbarWidget->isVisible());
    actionDisplayOptions->setChecked(displayDialog->isVisible());
    actionShowStats->setChecked(statsDialog->isVisible());
    actionAddData->setChecked(generator->isVisible());

    settings.beginGroup("displayOptions");
    if(settings.contains("infoCheck")) displayOptions->infoCheck->setChecked(settings.value("infoCheck").toBool());
    if(settings.contains("mapCheck")) displayOptions->mapCheck->setChecked(settings.value("mapCheck").toBool());
    if(settings.contains("modelCheck")) displayOptions->modelCheck->setChecked(settings.value("modelCheck").toBool());
    if(settings.contains("samplesCheck")) displayOptions->samplesCheck->setChecked(settings.value("samplesCheck").toBool());
    if(settings.contains("gridCheck")) displayOptions->gridCheck->setChecked(settings.value("gridCheck").toBool());
    if(settings.contains("spinZoom")) displayOptions->spinZoom->setValue(settings.value("spinZoom").toFloat());
    if(settings.contains("legendCheck")) displayOptions->legendCheck->setChecked(settings.value("legendCheck").toBool());
    if(settings.contains("check3DSamples")) displayOptions->check3DSamples->setChecked(settings.value("check3DSamples").toBool());
    if(settings.contains("check3DWireframe")) displayOptions->check3DWireframe->setChecked(settings.value("check3DWireframe").toBool());
    if(settings.contains("check3DSurfaces")) displayOptions->check3DSurfaces->setChecked(settings.value("check3DSurfaces").toBool());
    if(settings.contains("check3DTransparency")) displayOptions->check3DTransparency->setChecked(settings.value("check3DTransparency").toBool());
    if(settings.contains("check3DBlurry")) displayOptions->check3DBlurry->setChecked(settings.value("check3DBlurry").toBool());
    if(settings.contains("check3DRotate")) displayOptions->check3DRotate->setChecked(settings.value("check3DRotate").toBool());

    //if(settings.contains("xDimIndex")) displayOptions->xDimIndex->setValue(settings.value("xDimIndex").toInt());
    //if(settings.contains("yDimIndex")) displayOptions->yDimIndex->setValue(settings.value("yDimIndex").toInt());
    settings.endGroup();

    settings.beginGroup("drawingOptions");
    if(settings.contains("infoCheck")) drawToolbarContext1->randCombo->setCurrentIndex(settings.value("infoCheck").toInt());
    if(settings.contains("spinAngle")) drawToolbarContext2->spinAngle->setValue(settings.value("spinAngle").toFloat());
    if(settings.contains("spinCount")) drawToolbarContext1->spinCount->setValue(settings.value("spinCount").toFloat());
    if(settings.contains("spinSigmaX")) drawToolbarContext2->spinSigmaX->setValue(settings.value("spinSigmaX").toFloat());
    if(settings.contains("spinSigmaY")) drawToolbarContext2->spinSigmaY->setValue(settings.value("spinSigmaY").toFloat());
    if(settings.contains("spinObsAngle")) drawToolbarContext3->spinAngle->setValue(settings.value("spinObsAngle").toFloat());
    if(settings.contains("spinObsSigmaX")) drawToolbarContext3->spinSigmaX->setValue(settings.value("spinObsSigmaX").toFloat());
    if(settings.contains("spinObsSigmaY")) drawToolbarContext3->spinSigmaY->setValue(settings.value("spinObsSigmaY").toFloat());
    if(settings.contains("spinObsPowerX")) drawToolbarContext3->spinPowerX->setValue(settings.value("spinObsPowerX").toInt());
    if(settings.contains("spinObsPowerY")) drawToolbarContext3->spinPowerY->setValue(settings.value("spinObsPowerY").toInt());
    if(settings.contains("spinObsRepulsionX")) drawToolbarContext3->spinRepulsionX->setValue(settings.value("spinObsRepulsionX").toFloat());
    if(settings.contains("spinObsRepulsionY")) drawToolbarContext3->spinRepulsionY->setValue(settings.value("spinObsRepulsionY").toFloat());
    if(settings.contains("spinRadius")) drawToolbarContext4->spinRadius->setValue(settings.value("spinRadius").toFloat());
    if(settings.contains("spinAlpha")) drawToolbarContext4->spinAlpha->setValue(settings.value("spinAlpha").toFloat());
    if(settings.contains("spinSize")) drawToolbar->radiusSpin->setValue(settings.value("spinSize").toFloat());
    if(settings.contains("eraseCheck")) drawToolbar->eraseButton->setChecked(settings.value("eraseCheck").toBool());
    if(settings.contains("sprayCheck")) drawToolbar->sprayButton->setChecked(settings.value("sprayCheck").toBool());
    if(settings.contains("spray3DCheck")) drawToolbar->spray3DButton->setChecked(settings.value("spray3DCheck").toBool());
    if(settings.contains("singleCheck")) drawToolbar->singleButton->setChecked(settings.value("singleCheck").toBool());
    if(settings.contains("ellipseCheck")) drawToolbar->ellipseButton->setChecked(settings.value("ellipseCheck").toBool());
    if(settings.contains("lineCheck")) drawToolbar->lineButton->setChecked(settings.value("lineCheck").toBool());
    if(settings.contains("trajectoryCheck")) drawToolbar->trajectoryButton->setChecked(settings.value("trajectoryCheck").toBool());
    if(settings.contains("obstacleCheck")) drawToolbar->obstacleButton->setChecked(settings.value("obstacleCheck").toBool());
    if(settings.contains("paintCheck")) drawToolbar->paintButton->setChecked(settings.value("paintCheck").toBool());
    settings.endGroup();

    settings.beginGroup("classificationOptions");
    if(settings.contains("positiveClass")) optionsClassify->positiveSpin->setValue(settings.value("positiveClass").toFloat());
    if(settings.contains("binaryCheck")) optionsClassify->binaryCheck->setChecked(settings.value("binaryCheck").toBool());
    if(settings.contains("trainRatio")) optionsClassify->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    if(settings.contains("tab")) optionsClassify->algoList->setCurrentIndex(settings.value("tab").toInt());
    settings.endGroup();

    settings.beginGroup("regressionOptions");
    if(settings.contains("trainRatio")) optionsRegress->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    if(settings.contains("tab")) optionsRegress->algoList->setCurrentIndex(settings.value("tab").toInt());
    settings.endGroup();

    settings.beginGroup("dynamicalOptions");
    if(settings.contains("centerType")) optionsDynamic->centerCombo->setCurrentIndex(settings.value("centerType").toInt());
    if(settings.contains("zeroCheck")) optionsDynamic->zeroCheck->setChecked(settings.value("zeroCheck").toBool());
    if(settings.contains("resampleType")) optionsDynamic->resampleCombo->setCurrentIndex(settings.value("resampleType").toInt());
    if(settings.contains("resampleCount")) optionsDynamic->resampleSpin->setValue(settings.value("resampleCount").toFloat());
    if(settings.contains("obstacleType")) optionsDynamic->obstacleCombo->setCurrentIndex(settings.value("obstacleType").toInt());
    if(settings.contains("dT")) optionsDynamic->dtSpin->setValue(settings.value("dT").toFloat());
    if(settings.contains("tab")) optionsDynamic->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("colorCheck")) optionsDynamic->colorCheck->setChecked(settings.value("colorCheck").toBool());
    settings.endGroup();

    settings.beginGroup("compareOptions");
    if(settings.contains("foldCount")) optionsCompare->foldCountSpin->setValue(settings.value("foldCount").toFloat());
    if(settings.contains("trainRatio")) optionsCompare->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    settings.endGroup();

    settings.beginGroup("clusterOptions");
    if(settings.contains("tab")) optionsCluster->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("trainRatio")) optionsCluster->trainRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    if(settings.contains("trainTestCombo")) optionsCluster->trainTestCombo->setCurrentIndex(settings.value("trainTestCombo").toInt());
    if(settings.contains("optimizeCombo")) optionsCluster->optimizeCombo->setCurrentIndex(settings.value("optimizeCombo").toInt());
    if(settings.contains("rangeStart")) optionsCluster->rangeStartSpin->setValue(settings.value("rangeStart").toInt());
    if(settings.contains("rangeStop")) optionsCluster->rangeStopSpin->setValue(settings.value("rangeStop").toInt());
    settings.endGroup();

    settings.beginGroup("maximizeOptions");
    if(settings.contains("tab")) optionsMaximize->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("varianceSpin")) optionsMaximize->varianceSpin->setValue(settings.value("varianceSpin").toDouble());
    if(settings.contains("iterationsSpin")) optionsMaximize->iterationsSpin->setValue(settings.value("iterationsSpin").toInt());
    if(settings.contains("stoppingSpin")) optionsMaximize->stoppingSpin->setValue(settings.value("stoppingSpin").toDouble());
    if(settings.contains("benchmarkCombo")) optionsMaximize->benchmarkCombo->setCurrentIndex(settings.value("benchmarkCombo").toInt());
    settings.endGroup();

    settings.beginGroup("reinforceOptions");
    if(settings.contains("tab")) optionsReinforcement->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("varianceSpin")) optionsReinforcement->varianceSpin->setValue(settings.value("varianceSpin").toDouble());
    if(settings.contains("iterationsSpin")) optionsReinforcement->iterationsSpin->setValue(settings.value("iterationsSpin").toInt());
    if(settings.contains("displayIterationSpin")) optionsReinforcement->displayIterationSpin->setValue(settings.value("displayIterationSpin").toInt());
    if(settings.contains("problemCombo")) optionsReinforcement->problemCombo->setCurrentIndex(settings.value("problemCombo").toInt());
    if(settings.contains("rewardCombo")) optionsReinforcement->rewardCombo->setCurrentIndex(settings.value("rewardCombo").toInt());
    if(settings.contains("policyCombo")) optionsReinforcement->policyCombo->setCurrentIndex(settings.value("policyCombo").toInt());
    if(settings.contains("quantizeCombo")) optionsReinforcement->quantizeCombo->setCurrentIndex(settings.value("quantizeCombo").toInt());
    if(settings.contains("resolutionSpin")) optionsReinforcement->resolutionSpin->setValue(settings.value("resolutionSpin").toInt());
    if(settings.contains("benchmarkCombo")) optionsReinforcement->benchmarkCombo->setCurrentIndex(settings.value("benchmarkCombo").toInt());
    settings.endGroup();

    settings.beginGroup("projectOptions");
    if(settings.contains("tab")) optionsProject->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("fitCheck")) optionsProject->fitCheck->setChecked(settings.value("fitCheck").toBool());
    settings.endGroup();

    settings.beginGroup("statsOptions");
    if(settings.contains("tab")) showStats->tabWidget->setCurrentIndex(settings.value("tab").toInt());
    settings.endGroup();

    settings.beginGroup("generatorOptions");
    if(settings.contains("generatorCombo")) generator->ui->generatorCombo->setCurrentIndex(settings.value("generatorCombo").toInt());
    if(settings.contains("countSpin")) generator->ui->countSpin->setValue(settings.value("countSpin").toInt());
    if(settings.contains("dimSpin")) generator->ui->dimSpin->setValue(settings.value("dimSpin").toInt());
    if(settings.contains("gridCountSpin")) generator->ui->gridCountSpin->setValue(settings.value("gridCountSpin").toInt());
    if(settings.contains("classesCount")) generator->ui->classesCount->setValue(settings.value("classesCount").toInt());
    if(settings.contains("radiusSpin")) generator->ui->radiusSpin->setValue(settings.value("radiusSpin").toFloat());
    settings.endGroup();

    if(optionsClassify->algoList->currentIndex() < 0 || optionsClassify->algoList->currentIndex() > optionsClassify->algoList->count()) optionsClassify->algoList->setCurrentIndex(0);
    if(optionsCluster->algoList->currentIndex() < 0 || optionsCluster->algoList->currentIndex() > optionsCluster->algoList->count()) optionsCluster->algoList->setCurrentIndex(0);
    if(optionsDynamic->algoList->currentIndex() < 0 || optionsDynamic->algoList->currentIndex() > optionsDynamic->algoList->count()) optionsDynamic->algoList->setCurrentIndex(0);
    if(optionsMaximize->algoList->currentIndex() < 0 || optionsMaximize->algoList->currentIndex() > optionsMaximize->algoList->count()) optionsMaximize->algoList->setCurrentIndex(0);
    if(optionsProject->algoList->currentIndex() < 0 || optionsProject->algoList->currentIndex() > optionsProject->algoList->count()) optionsProject->algoList->setCurrentIndex(0);
    if(optionsRegress->algoList->currentIndex() < 0 || optionsRegress->algoList->currentIndex() > optionsRegress->algoList->count()) optionsRegress->algoList->setCurrentIndex(0);
    if(optionsReinforcement->algoList->currentIndex() < 0 || optionsReinforcement->algoList->currentIndex() > optionsReinforcement->algoList->count()) optionsReinforcement->algoList->setCurrentIndex(0);

    FOR(i,classifiers.size())
    {
        if(!classifiers[i]) continue;
        settings.beginGroup(QString("plugins::classifiers::") + classifiers[i]->GetName());
        classifiers[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,clusterers.size())
    {
        if(!clusterers[i]) continue;
        settings.beginGroup(QString("plugins::clusterers::") + clusterers[i]->GetName());
        clusterers[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,regressors.size())
    {
        if(!regressors[i]) continue;
        settings.beginGroup(QString("plugins::regressors::") + regressors[i]->GetName());
        regressors[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,dynamicals.size())
    {
        if(!dynamicals[i]) continue;
        settings.beginGroup(QString("plugins::dynamicals::") + dynamicals[i]->GetName());
        dynamicals[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,maximizers.size())
    {
        if(!maximizers[i]) continue;
        settings.beginGroup(QString("plugins::maximizers::") + maximizers[i]->GetName());
        maximizers[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,reinforcements.size())
    {
        if(!reinforcements[i]) continue;
        settings.beginGroup(QString("plugins::reinforcements::") + reinforcements[i]->GetName());
        reinforcements[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,projectors.size())
    {
        if(!projectors[i]) continue;
        settings.beginGroup(QString("plugins::projectors::") + projectors[i]->GetName());
        projectors[i]->LoadOptions(settings);
        settings.endGroup();
    }
    canvas->repaint();
}


void MLDemos::SaveParams( QString filename )
{
    if(!classifier && !regressor && !clusterer && !dynamical && !maximizer) return;
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Append);
    QTextStream out(&file);
    if(!file.isOpen()) return;

    if(!canvas->data->GetCount()) out << "0 2\n";
    char groupName[255];

    if(canvas->dimNames.size())
    {
        out << "headers" << " " << canvas->dimNames.size();
        FOR(i, canvas->dimNames.size())
        {
            QString header = canvas->dimNames.at(i);
            // we take out all spaces as they're really not nice for parsing the headers afterwards
            header.replace("\n", "_");
            header.replace(" ", "_");
            header.replace("\t", "_");
            out << " " << header;
        }
        out << "\n";
    }
    if(canvas->classNames.size())
    {
        out << "classNames" << " " << canvas->classNames.size();
        FORIT(canvas->classNames, int, QString)
        {
            int index = it->first;
            QString name = it->second;
            name.replace("\n", "_");
            name.replace(" ", "_");
            name.replace("\t", "_");
            out << " " << index << " " << name;
        }
        out << "\n";
    }
    if(canvas->data->categorical.size())
    {
        out << "categorical" << " " << canvas->data->categorical.size();
        for(map<int,vector<string> >::iterator it=canvas->data->categorical.begin(); it != canvas->data->categorical.end(); it++)
        {
            out << " " << it->first << " " << it->second.size();
            FOR(i, it->second.size())
            {
                QString name(it->second[i].c_str());
                name.replace("\n", "_");
                name.replace(" ", "_");
                name.replace("\t", "_");
                out << " " << name;
            }
        }
        out << "\n";
    }
    if(classifier)
    {
        int tab = optionsClassify->algoList->currentIndex();
        sprintf(groupName,"classificationOptions");
        out << groupName << ":" << "tab" << " " << optionsClassify->algoList->currentIndex() << "\n";
        out << groupName << ":" << "positiveClass" << " " << optionsClassify->positiveSpin->value() << "\n";
        out << groupName << ":" << "binaryCheck" << " " << optionsClassify->binaryCheck->isChecked() << "\n";
        if(tab < classifiers.size() && classifiers[tab])
        {
            classifiers[tab]->SaveParams(out);
        }
    }
    if(regressor)
    {
        int tab = optionsRegress->algoList->currentIndex();
        sprintf(groupName,"regressionOptions");
        out << groupName << ":" << "tab" << " " << optionsRegress->algoList->currentIndex() << "\n";
        out << groupName << ":" << "outputDimCombo" << " " << optionsRegress->outputDimCombo->currentIndex() << "\n";
        if(tab < regressors.size() && regressors[tab])
        {
            regressors[tab]->SaveParams(out);
        }
    }
    if(dynamical)
    {
        int tab = optionsDynamic->algoList->currentIndex();
        sprintf(groupName,"dynamicalOptions");
        out << groupName << ":" << "centerType" << " " << optionsDynamic->centerCombo->currentIndex() << "\n";
        out << groupName << ":" << "zeroCheck" << " " << optionsDynamic->zeroCheck->isChecked() << "\n";
        out << groupName << ":" << "resampleType" << " " << optionsDynamic->resampleCombo->currentIndex() << "\n";
        out << groupName << ":" << "resampleCount" << " " << optionsDynamic->resampleSpin->value() << "\n";
        out << groupName << ":" << "obstacleType" << " " << optionsDynamic->obstacleCombo->currentIndex() << "\n";
        out << groupName << ":" << "dT" << " " << optionsDynamic->dtSpin->value() << "\n";
        out << groupName << ":" << "colorCheck" << " " << optionsDynamic->colorCheck->isChecked() << "\n";
        out << groupName << ":" << "tab" << " " << optionsDynamic->algoList->currentIndex() << "\n";
        if(tab < dynamicals.size() && dynamicals[tab])
        {
            dynamicals[tab]->SaveParams(out);
        }
    }
    if(clusterer)
    {
        int tab = optionsCluster->algoList->currentIndex();
        sprintf(groupName,"clusterOptions");
        out << groupName << ":" << "tab" << " " << optionsCluster->algoList->currentIndex() << "\n";
        out << groupName << ":" << "trainRatio" << " " << optionsCluster->trainRatioCombo->currentIndex() << "\n";
        out << groupName << ":" << "trainTestCombo" << " " << optionsCluster->trainTestCombo->currentIndex() << "\n";
        out << groupName << ":" << "optimizeCombo" << " " << optionsCluster->optimizeCombo->currentIndex() << "\n";
        out << groupName << ":" << "rangeStart" << " " << optionsCluster->rangeStartSpin->value() << "\n";
        out << groupName << ":" << "rangeStop" << " " << optionsCluster->rangeStopSpin->value() << "\n";
        if(tab < clusterers.size() && clusterers[tab])
        {
            clusterers[tab]->SaveParams(out);
        }
    }
    if(maximizer)
    {
        int tab = optionsMaximize->algoList->currentIndex();
        double variance = optionsMaximize->varianceSpin->value();
        sprintf(groupName,"maximizationOptions");
        out << groupName << ":" << "tab" << " " << optionsMaximize->algoList->currentIndex() << "\n";
        out << groupName << ":" << "gaussVarianceSpin" << " " << optionsMaximize->varianceSpin->value() << "\n";
        out << groupName << ":" << "iterationsSpin" << " " << optionsMaximize->iterationsSpin->value() << "\n";
        out << groupName << ":" << "stoppingSpin" << " " << optionsMaximize->stoppingSpin->value() << "\n";
        out << groupName << ":" << "benchmarkCombo" << " " << optionsMaximize->benchmarkCombo->currentIndex() << "\n";
        if(tab < maximizers.size() && maximizers[tab])
        {
            maximizers[tab]->SaveParams(out);
        }
    }
    if(reinforcement)
    {
        int tab = optionsReinforcement->algoList->currentIndex();
        double variance = optionsReinforcement->varianceSpin->value();
        sprintf(groupName,"reinforcementOptions");
        out << groupName << ":" << "tab" << " " << optionsReinforcement->algoList->currentIndex() << "\n";
        out << groupName << ":" << "gaussVarianceSpin" << " " << optionsReinforcement->varianceSpin->value() << "\n";
        out << groupName << ":" << "iterationsSpin" << " " << optionsReinforcement->iterationsSpin->value() << "\n";
        out << groupName << ":" << "displayIterationSpin" << " " << optionsReinforcement->displayIterationSpin->value() << "\n";
        out << groupName << ":" << "problemCombo" << " " << optionsReinforcement->problemCombo->currentIndex() << "\n";
        out << groupName << ":" << "rewardCombo" << " " << optionsReinforcement->rewardCombo->currentIndex() << "\n";
        out << groupName << ":" << "policyCombo" << " " << optionsReinforcement->policyCombo->currentIndex() << "\n";
        out << groupName << ":" << "quantizeCombo" << " " << optionsReinforcement->quantizeCombo->currentIndex() << "\n";
        out << groupName << ":" << "resolutionSpin" << " " << optionsReinforcement->resolutionSpin->value() << "\n";
        out << groupName << ":" << "benchmarkCombo" << " " << optionsReinforcement->benchmarkCombo->currentIndex() << "\n";

        if(tab < reinforcements.size() && reinforcements[tab])
        {
            reinforcements[tab]->SaveParams(out);
        }
    }
    if(projector)
    {
        int tab = optionsProject->algoList->currentIndex();
        sprintf(groupName,"projectOptions");
        out << groupName << ":" << "tab" << " " << optionsProject->algoList->currentIndex() << "\n";
        out << groupName << ":" << "fitCheck" << " " << optionsProject->fitCheck->isChecked() << "\n";
        if(tab < projectors.size() && projectors[tab])
        {
            projectors[tab]->SaveParams(out);
        }
    }
}

bool startsWith(char *a, char *b)
{
    bool yeah = true;
    for (int i=0; i<strlen(b); i++)
    {
        yeah &= a[i] == b[i];
    }
    return yeah;
}

void MLDemos::LoadParams( QString filename )
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    if(!file.isOpen()) return;

    int sampleCnt, size;
    in >> sampleCnt;
    in >> size;
    QString line;
    //char line[255];
    float value;

    char classGroup[255];
    char regrGroup[255];
    char dynGroup[255];
    char clustGroup[255];
    char maximGroup[255];
    char reinfGroup[255];
    char projGroup[255];
    sprintf(classGroup,"classificationOptions");
    sprintf(regrGroup,"regressionOptions");
    sprintf(dynGroup,"dynamicalOptions");
    sprintf(clustGroup,"clusteringOptions");
    sprintf(maximGroup,"maximizationOptions");
    sprintf(reinfGroup,"reinforcementOptions");
    sprintf(projGroup,"projectOptions");

    // we skip the samples themselves
    //qDebug() << "Skipping "<< sampleCnt <<" samples" << endl;
    FOR(i, sampleCnt) line = in.readLine();
    bool bClass = false, bRegr = false, bDyn = false, bClust = false, bMaxim = false, bReinf = false, bProj = false;
    //qDebug() << "Loading parameter list" << endl;
    int tab = 0;
    while(!in.atEnd())
    {
        in >> line;
        in >> value;
        //		qDebug() << line << " " << value << endl;
        if(line.startsWith("headers"))
        {
            int headerCount = value;
            canvas->dimNames.clear();
            FOR(i, headerCount)
            {
                QString header;
                in >> header;
                canvas->dimNames << header;
            }
            //qDebug() << "dimensions: " << dimensionNames;
        }
        if(line.startsWith("classNames"))
        {
            int namesCount = value;
            canvas->classNames.clear();
            FOR(i, namesCount)
            {
                int index;
                QString name;
                in >> index;
                in >> name;
                canvas->classNames[index] = name;
            }
        }
        if(line.startsWith("categorical"))
        {
            int namesCount = value;
            canvas->data->categorical.clear();
            FOR(i, namesCount)
            {
                int index;
                int dimCount;
                QString name;
                in >> index;
                in >> dimCount;
                vector<string> cats;
                FOR(j, dimCount)
                {
                    in >> name;
                    cats.push_back(name.toStdString());
                }
                canvas->data->categorical[index] = cats;
            }
        }
        if(line.startsWith(classGroup))
        {
            bClass = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabClass);
            if(line.endsWith("tab")) optionsClassify->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("positiveClass")) optionsClassify->positiveSpin->setValue((int)value);
            if(line.endsWith("binaryCheck")) optionsClassify->binaryCheck->setChecked((int)value);
            if(tab < classifiers.size() && classifiers[tab]) classifiers[tab]->LoadParams(line,value);
        }
        if(line.startsWith(regrGroup))
        {
            bRegr = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabRegr);
            if(line.endsWith("tab")) optionsRegress->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("outputDimCombo")) optionsRegress->outputDimCombo->setCurrentIndex((int)value);
            if(tab < regressors.size() && regressors[tab]) regressors[tab]->LoadParams(line,value);
        }
        if(line.startsWith(dynGroup))
        {
            bDyn = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabDyn);
            if(line.endsWith("centerType")) optionsDynamic->centerCombo->setCurrentIndex((int)value);
            if(line.endsWith("zeroCheck")) optionsDynamic->zeroCheck->setChecked((int)value);
            if(line.endsWith("resampleType")) optionsDynamic->resampleCombo->setCurrentIndex((int)value);
            if(line.endsWith("resampleCount")) optionsDynamic->resampleSpin->setValue((int)value);
            if(line.endsWith("obstacleType")) optionsDynamic->obstacleCombo->setCurrentIndex((int)value);
            if(line.endsWith("dT")) optionsDynamic->dtSpin->setValue((float)value);
            if(line.endsWith("colorCheck")) optionsDynamic->colorCheck->setChecked((int)value);
            if(line.endsWith("tab")) optionsDynamic->algoList->setCurrentIndex(tab = (int)value);
            if(tab < dynamicals.size() && dynamicals[tab]) dynamicals[tab]->LoadParams(line,value);
        }
        if(line.startsWith(clustGroup))
        {
            bClust = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabClust);
            if(line.endsWith("tab")) optionsCluster->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("trainRatio")) optionsCluster->trainRatioCombo->setCurrentIndex((int)value);
            if(line.endsWith("trainTestCombo")) optionsCluster->trainTestCombo->setCurrentIndex((int)value);
            if(line.endsWith("optimizeCombo")) optionsCluster->optimizeCombo->setCurrentIndex((int)value);
            if(line.endsWith("rangeStart")) optionsCluster->rangeStartSpin->setValue((int)value);
            if(line.endsWith("rangeStop")) optionsCluster->rangeStopSpin->setValue((int)value);
            if(tab < clusterers.size() && clusterers[tab]) clusterers[tab]->LoadParams(line,value);
        }
        if(line.startsWith(maximGroup))
        {
            bMaxim = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabMax);
            if(line.endsWith("tab")) optionsMaximize->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("gaussVarianceSpin")) optionsMaximize->varianceSpin->setValue((double)value);
            if(line.endsWith("iterationsSpin")) optionsMaximize->iterationsSpin->setValue((int)value);
            if(line.endsWith("stoppingSpin")) optionsMaximize->stoppingSpin->setValue((double)value);
            if(line.endsWith("benchmarkCombo")) optionsMaximize->benchmarkCombo->setCurrentIndex((int)value);
            if(tab < maximizers.size() && maximizers[tab]) maximizers[tab]->LoadParams(line,value);
        }
        if(line.startsWith(reinfGroup))
        {
            bReinf = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabReinf);
            if(line.endsWith("tab")) optionsReinforcement->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("gaussVarianceSpin")) optionsReinforcement->varianceSpin->setValue((double)value);
            if(line.endsWith("iterationsSpin")) optionsReinforcement->iterationsSpin->setValue((int)value);
            if(line.endsWith("displayIterationSpin")) optionsReinforcement->displayIterationSpin->setValue((int)value);
            if(line.endsWith("problemCombo")) optionsReinforcement->problemCombo->setCurrentIndex((int)value);
            if(line.endsWith("rewardCombo")) optionsReinforcement->rewardCombo->setCurrentIndex((int)value);
            if(line.endsWith("policyCombo")) optionsReinforcement->policyCombo->setCurrentIndex((int)value);
            if(line.endsWith("quantizeCombo")) optionsReinforcement->quantizeCombo->setCurrentIndex((int)value);
            if(line.endsWith("resolutionSpin")) optionsReinforcement->resolutionSpin->setValue((int)value);
            if(line.endsWith("benchmarkCombo")) optionsReinforcement->benchmarkCombo->setCurrentIndex((int)value);
            if(tab < reinforcements.size() && reinforcements[tab]) reinforcements[tab]->LoadParams(line,value);

        }
        if(line.startsWith(projGroup))
        {
            bProj = true;
            algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabProj);
            if(line.endsWith("tab")) optionsProject->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("fitCheck")) optionsProject->fitCheck->setChecked((int)value);
            if(tab < projectors.size() && projectors[tab]) projectors[tab]->LoadParams(line,value);
        }
    }
    ResetPositiveClass();
    ManualSelectionUpdated();
    InputDimensionsUpdated();
    if(bClass) Classify();
    if(bRegr) Regression();
    if(bDyn) Dynamize();
    if(bClust) Cluster();
    if(bProj) Project();
    actionAlgorithms->setChecked(algorithmWidget->isVisible());
}

void MLDemos::ExportOutput()
{
    if(!classifier && !regressor && !clusterer && !projector) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Output Data"), "", tr("Data (*.txt *.csv)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".txt") && !filename.endsWith(".csv")) filename += ".txt";

    QFile file(filename);
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    if(!file.isOpen()) return;

    if(classifier || clusterer || regressor)
    {
        out << "#Sample(n-dims) TrueClass ComputedValue(s)\n";
        vector<fvec> samples = sourceDims.size() ? canvas->data->GetSampleDims(sourceDims) : canvas->data->GetSamples();
        ivec labels = canvas->data->GetLabels();
        FOR(i, samples.size())
        {
            fvec &sample = samples[i];
            fvec res;
            if(classifier) res = classifier->TestMulti(sample);
            else if (clusterer) res = clusterer->Test(sample);
            else if (regressor) res = regressor->Test(sample);
            FOR(d, sample.size()) out << QString("%1\t").arg(sample[d]);
            out << QString("%1\t").arg(labels[i]);
            FOR(d, res.size()) out << QString("%1\t").arg(res[d], 0, 'f', 3);
            out << "\n";
        }
    }
    else if(projector)
    {
        out << "#Sample(n-dims) TrueClass Projected(m-dims)\n";
        vector<fvec> samples = canvas->data->GetSamples();
        ivec labels = canvas->data->GetLabels();
        FOR(i, samples.size())
        {
            fvec &sample = samples[i];
            fvec projected;
            projected = projector->Project(sample);
            FOR(d, sample.size()) out << QString("%1\t").arg(sample[d]);
            out << QString("%1\t").arg(labels[i]);
            FOR(d, projected.size()) out << QString("%1\t").arg(projected[d], 0, 'f', 3);
            out << "\n";
        }
    }
    file.close();
}

void MLDemos::LoadClassifier()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    int tab = optionsClassify->algoList->currentIndex();
    if(tab >= classifiers.size() || !classifiers[tab]) return;
    Classifier *classifier = classifiers[tab]->GetClassifier();
    bool ok = classifier->LoadModel(filename.toStdString());
    if(ok)
    {
        if(!classifierMulti.size()) DEL(this->classifier);
        this->classifier = 0;
        FOR(i,classifierMulti.size()) DEL(classifierMulti[i]); classifierMulti.clear();
        this->classifier = classifier;
        tabUsedForTraining = tab;
        classifiers[tab]->Draw(canvas, classifier);
        DrawClassifiedSamples(canvas, classifier, classifierMulti);
        if(drawTimer->isRunning()) drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->start(QThread::NormalPriority);
    }
    else DEL(classifier);
}

void MLDemos::SaveClassifier()
{
    if(!classifier) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".model")) filename += ".model";
    classifier->SaveModel(filename.toStdString());
}

void MLDemos::LoadRegressor()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    int tab = optionsRegress->algoList->currentIndex();
    if(tab >= regressors.size() || !regressors[tab]) return;
    Regressor *regressor = regressors[tab]->GetRegressor();
    bool ok = regressor->LoadModel(filename.toStdString());
    if(ok)
    {
        DEL(this->regressor);
        this->regressor = regressor;
        tabUsedForTraining = tab;
        regressors[tab]->Draw(canvas, regressor);
        if(drawTimer->isRunning()) drawTimer->Stop();
        drawTimer->Clear();
        drawTimer->start(QThread::NormalPriority);
    }
    else DEL(regressor);
}

void MLDemos::SaveRegressor()
{
    if(!regressor) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".model")) filename += ".model";
    regressor->SaveModel(filename.toStdString());
}

void MLDemos::LoadDynamical()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    int tab = optionsDynamic->algoList->currentIndex();
    if(tab >= dynamicals.size() || !dynamicals[tab]) return;
    Dynamical *dynamical = dynamicals[tab]->GetDynamical();
    bool ok = dynamical->LoadModel(filename.toStdString());
    if(ok)
    {
        DEL(this->dynamical);
        this->dynamical = dynamical;
        tabUsedForTraining = tab;
        dynamicals[tab]->Draw(canvas, dynamical);
        if(dynamicals[tab]->UsesDrawTimer())
        {
            if(drawTimer->isRunning()) drawTimer->Stop();
            drawTimer->Clear();
            drawTimer->bColorMap = optionsDynamic->colorCheck->isChecked();
            drawTimer->start(QThread::NormalPriority);
        }
    }
    else DEL(dynamical);
}

void MLDemos::SaveDynamical()
{
    if(!dynamical) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Model"), "", tr("Model (*.model)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".model")) filename += ".model";
    dynamical->SaveModel(filename.toStdString());
}

void MLDemos::MapFromReward()
{
    RewardMap *reward = canvas->data->GetReward();
    if(!reward || reward->Empty() || reward->dim < 2) return;
    int w = reward->size[0];
    int h = reward->size[1];
    QImage image(w,h,QImage::Format_RGB32);
    FOR(y, h)
    {
        FOR(x, w)
        {
            double value = reward->rewards[y*w + x];
            value = min(max(value*255.,0.),255.);
            image.setPixel(x,y,qRgb(255,255-value,255-value));
        }
    }
    int W = canvas->width();
    int H = canvas->height();
    canvas->maps.reward = QPixmap::fromImage(image).scaled(W, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    canvas->repaint();
}

void MLDemos::RewardFromMap(QImage rewardMap)
{
    QRgb *pixels = (QRgb*) rewardMap.bits();
    int w = rewardMap.width();
    int h = rewardMap.height();

    double *data = new double[w*h];
    double maxData = 0;
    FOR(i, w*h)
    {
        data[i] = 1. - qBlue(pixels[i])/255.; // all data is in a 0-1 range
        maxData = max(maxData, data[i]);
    }
    if(maxData > 0)
    {
        FOR(i, w*h) data[i] /= maxData; // we ensure that the data is normalized
    }
    ivec size;
    size.push_back(w);
    size.push_back(h);
    fvec low(2,0.f);
    fvec high(2,1.f);
    canvas->data->GetReward()->SetReward(data, size, low, high);
    delete [] data;
}
