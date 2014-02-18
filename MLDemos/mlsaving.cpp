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
    settings.setValue("algoGeometry", algo->algorithmWidget->saveGeometry());
    settings.setValue("drawGeometry", drawToolbarWidget->saveGeometry());
    settings.setValue("displayGeometry", displayDialog->saveGeometry());
    settings.setValue("statsGeometry", statsDialog->saveGeometry());
    settings.setValue("compareGeometry", compare->paramsWidget->saveGeometry());
    settings.setValue("generatorGeometry", generator->saveGeometry());

    settings.setValue("algoTab", algo->options->tabWidget->currentIndex());
    settings.setValue("ShowAlgoOptions", algo->algorithmWidget->isVisible());
    settings.setValue("ShowCompare", compare->paramsWidget->isVisible());
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
    settings.setValue("positiveClass", algo->optionsClassify->positiveSpin->value());
    settings.setValue("binaryCheck", algo->optionsClassify->binaryCheck->isChecked());
    settings.setValue("trainRatio", algo->optionsClassify->traintestRatioCombo->currentIndex());
    settings.setValue("tab", algo->optionsClassify->algoList->currentIndex());
    settings.endGroup();

    settings.beginGroup("regressionOptions");
    settings.setValue("trainRatio", algo->optionsRegress->traintestRatioCombo->currentIndex());
    settings.setValue("tab", algo->optionsRegress->algoList->currentIndex());
    settings.endGroup();

    settings.beginGroup("dynamicalOptions");
    settings.setValue("centerType", algo->optionsDynamic->centerCombo->currentIndex());
    settings.setValue("zeroCheck", algo->optionsDynamic->zeroCheck->isChecked());
    settings.setValue("resampleType", algo->optionsDynamic->resampleCombo->currentIndex());
    settings.setValue("resampleCount", algo->optionsDynamic->resampleSpin->value());
    settings.setValue("obstacleType", algo->optionsDynamic->obstacleCombo->currentIndex());
    settings.setValue("dT", algo->optionsDynamic->dtSpin->value());
    settings.setValue("tab", algo->optionsDynamic->algoList->currentIndex());
    settings.setValue("colorCheck", algo->optionsDynamic->colorCheck->isChecked());
    settings.endGroup();

    settings.beginGroup("clusterOptions");
    settings.setValue("tab", algo->optionsCluster->algoList->currentIndex());
    settings.setValue("trainRatio", algo->optionsCluster->trainRatioCombo->currentIndex());
    settings.setValue("trainTestCombo", algo->optionsCluster->trainTestCombo->currentIndex());
    settings.setValue("optimizeCombo", algo->optionsCluster->optimizeCombo->currentIndex());
    settings.setValue("rangeStart", algo->optionsCluster->rangeStartSpin->value());
    settings.setValue("rangeStop", algo->optionsCluster->rangeStopSpin->value());
    settings.endGroup();

    settings.beginGroup("maximizeOptions");
    settings.setValue("tab", algo->optionsMaximize->algoList->currentIndex());
    settings.setValue("varianceSpin", algo->optionsMaximize->varianceSpin->value());
    settings.setValue("iterationsSpin", algo->optionsMaximize->iterationsSpin->value());
    settings.setValue("stoppingSpin", algo->optionsMaximize->stoppingSpin->value());
    settings.setValue("benchmarkCombo", algo->optionsMaximize->benchmarkCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("reinforceOptions");
    settings.setValue("tab", algo->optionsReinforcement->algoList->currentIndex());
    settings.setValue("varianceSpin", algo->optionsReinforcement->varianceSpin->value());
    settings.setValue("iterationsSpin", algo->optionsReinforcement->iterationsSpin->value());
    settings.setValue("displayIterationSpin", algo->optionsReinforcement->displayIterationSpin->value());
    settings.setValue("problemCombo", algo->optionsReinforcement->problemCombo->currentIndex());
    settings.setValue("rewardCombo", algo->optionsReinforcement->rewardCombo->currentIndex());
    settings.setValue("policyCombo", algo->optionsReinforcement->policyCombo->currentIndex());
    settings.setValue("quantizeCombo", algo->optionsReinforcement->quantizeCombo->currentIndex());
    settings.setValue("resolutionSpin", algo->optionsReinforcement->resolutionSpin->value());
    settings.setValue("benchmarkCombo", algo->optionsReinforcement->benchmarkCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("projectOptions");
    settings.setValue("tab", algo->optionsProject->algoList->currentIndex());
    settings.setValue("fitCheck", algo->optionsProject->fitCheck->isChecked());
    settings.endGroup();

    settings.beginGroup("statsOptions");
    settings.setValue("tab", showStats->tabWidget->currentIndex());
    settings.endGroup();

    settings.beginGroup("compareOptions");
    settings.setValue("foldCount", compare->params->foldCountSpin->value());
    settings.setValue("trainRatio", compare->params->traintestRatioCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("generatorOptions");
    settings.setValue("generatorCombo", generator->ui->generatorCombo->currentIndex());
    settings.setValue("countSpin", generator->ui->countSpin->value());
    settings.setValue("dimSpin", generator->ui->dimSpin->value());
    settings.setValue("gridCountSpin", generator->ui->gridCountSpin->value());
    settings.setValue("classesCount", generator->ui->classesCount->value());
    settings.setValue("radiusSpin", generator->ui->radiusSpin->value());
    settings.endGroup();

    FOR(i,algo->classifiers.size())
    {
        if(!algo->classifiers[i]) continue;
        settings.beginGroup(QString("plugins::classifiers::") + algo->classifiers[i]->GetName());
        algo->classifiers[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->clusterers.size())
    {
        if(!algo->clusterers[i]) continue;
        settings.beginGroup(QString("plugins::clusterers::") + algo->clusterers[i]->GetName());
        algo->clusterers[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->regressors.size())
    {
        if(!algo->regressors[i]) continue;
        settings.beginGroup(QString("plugins::regressors::") + algo->regressors[i]->GetName());
        algo->regressors[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->dynamicals.size())
    {
        if(!algo->dynamicals[i]) continue;
        settings.beginGroup(QString("plugins::dynamicals::") + algo->dynamicals[i]->GetName());
        algo->dynamicals[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->maximizers.size())
    {
        if(!algo->maximizers[i]) continue;
        settings.beginGroup(QString("plugins::maximizers::") + algo->maximizers[i]->GetName());
        algo->maximizers[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->reinforcements.size())
    {
        if(!algo->reinforcements[i]) continue;
        settings.beginGroup(QString("plugins::reinforcements::") + algo->reinforcements[i]->GetName());
        algo->reinforcements[i]->SaveOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->projectors.size())
    {
        if(!algo->projectors[i]) continue;
        settings.beginGroup(QString("plugins::projectors::") + algo->projectors[i]->GetName());
        algo->projectors[i]->SaveOptions(settings);
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
    if(settings.contains("algoGeometry")) algo->algorithmWidget->restoreGeometry(settings.value("algoGeometry").toByteArray());
    if(settings.contains("drawGeometry")) drawToolbarWidget->restoreGeometry(settings.value("drawGeometry").toByteArray());
    if(settings.contains("displayGeometry")) displayDialog->restoreGeometry(settings.value("displayGeometry").toByteArray());
    if(settings.contains("statsGeometry")) statsDialog->restoreGeometry(settings.value("statsGeometry").toByteArray());
    if(settings.contains("compareGeometry")) compare->paramsWidget->restoreGeometry(settings.value("compareGeometry").toByteArray());
    if(settings.contains("generatorGeometry")) generator->restoreGeometry(settings.value("generatorGeometry").toByteArray());
#ifdef MACX // ugly hack to avoid resizing problems on the mac
    if(height() < 400) resize(width(),400);
    if(algo->algorithmWidget->height() < 220) algo->algorithmWidget->resize(636,220);
#endif // MACX

    if(settings.contains("algoTab")) algo->options->tabWidget->setCurrentIndex(settings.value("algoTab").toInt());
    if(settings.contains("ShowAlgoOptions")) algo->algorithmWidget->setVisible(settings.value("ShowAlgoOptions").toBool());
    if(settings.contains("ShowCompare")) compare->paramsWidget->setVisible(settings.value("ShowCompare").toBool());
    if(settings.contains("ShowDrawOptions")) drawToolbarWidget->setVisible(settings.value("ShowDrawOptions").toBool());
    if(settings.contains("ShowDisplayOptions")) displayDialog->setVisible(settings.value("ShowDisplayOptions").toBool());
    if(settings.contains("ShowStatsOptions")) statsDialog->setVisible(settings.value("ShowStatsOptions").toBool());
    if(settings.contains("ShowToolbar")) ui.actionShow_Toolbar->setChecked(settings.value("ShowToolbar").toBool());
    if(settings.contains("SmallIcons")) ui.actionSmall_Icons->setChecked(settings.value("SmallIcons").toBool());
    if(settings.contains("ShowGenerator")) generator->setVisible(settings.value("ShowGenerator").toBool());
    //    if(settings.contains("canvasType")) ui.canvasTypeCombo->setCurrentIndex(settings.value("canvasType").toInt());
    settings.endGroup();

    actionAlgorithms->setChecked(algo->algorithmWidget->isVisible());
    actionCompare->setChecked(compare->paramsWidget->isVisible());
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
    if(settings.contains("positiveClass")) algo->optionsClassify->positiveSpin->setValue(settings.value("positiveClass").toFloat());
    if(settings.contains("binaryCheck")) algo->optionsClassify->binaryCheck->setChecked(settings.value("binaryCheck").toBool());
    if(settings.contains("trainRatio")) algo->optionsClassify->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    if(settings.contains("tab")) algo->optionsClassify->algoList->setCurrentIndex(settings.value("tab").toInt());
    settings.endGroup();

    settings.beginGroup("regressionOptions");
    if(settings.contains("trainRatio")) algo->optionsRegress->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    if(settings.contains("tab")) algo->optionsRegress->algoList->setCurrentIndex(settings.value("tab").toInt());
    settings.endGroup();

    settings.beginGroup("dynamicalOptions");
    if(settings.contains("centerType")) algo->optionsDynamic->centerCombo->setCurrentIndex(settings.value("centerType").toInt());
    if(settings.contains("zeroCheck")) algo->optionsDynamic->zeroCheck->setChecked(settings.value("zeroCheck").toBool());
    if(settings.contains("resampleType")) algo->optionsDynamic->resampleCombo->setCurrentIndex(settings.value("resampleType").toInt());
    if(settings.contains("resampleCount")) algo->optionsDynamic->resampleSpin->setValue(settings.value("resampleCount").toFloat());
    if(settings.contains("obstacleType")) algo->optionsDynamic->obstacleCombo->setCurrentIndex(settings.value("obstacleType").toInt());
    if(settings.contains("dT")) algo->optionsDynamic->dtSpin->setValue(settings.value("dT").toFloat());
    if(settings.contains("tab")) algo->optionsDynamic->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("colorCheck")) algo->optionsDynamic->colorCheck->setChecked(settings.value("colorCheck").toBool());
    settings.endGroup();

    settings.beginGroup("clusterOptions");
    if(settings.contains("tab")) algo->optionsCluster->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("trainRatio")) algo->optionsCluster->trainRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    if(settings.contains("trainTestCombo")) algo->optionsCluster->trainTestCombo->setCurrentIndex(settings.value("trainTestCombo").toInt());
    if(settings.contains("optimizeCombo")) algo->optionsCluster->optimizeCombo->setCurrentIndex(settings.value("optimizeCombo").toInt());
    if(settings.contains("rangeStart")) algo->optionsCluster->rangeStartSpin->setValue(settings.value("rangeStart").toInt());
    if(settings.contains("rangeStop")) algo->optionsCluster->rangeStopSpin->setValue(settings.value("rangeStop").toInt());
    settings.endGroup();

    settings.beginGroup("maximizeOptions");
    if(settings.contains("tab")) algo->optionsMaximize->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("varianceSpin")) algo->optionsMaximize->varianceSpin->setValue(settings.value("varianceSpin").toDouble());
    if(settings.contains("iterationsSpin")) algo->optionsMaximize->iterationsSpin->setValue(settings.value("iterationsSpin").toInt());
    if(settings.contains("stoppingSpin")) algo->optionsMaximize->stoppingSpin->setValue(settings.value("stoppingSpin").toDouble());
    if(settings.contains("benchmarkCombo")) algo->optionsMaximize->benchmarkCombo->setCurrentIndex(settings.value("benchmarkCombo").toInt());
    settings.endGroup();

    settings.beginGroup("reinforceOptions");
    if(settings.contains("tab")) algo->optionsReinforcement->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("varianceSpin")) algo->optionsReinforcement->varianceSpin->setValue(settings.value("varianceSpin").toDouble());
    if(settings.contains("iterationsSpin")) algo->optionsReinforcement->iterationsSpin->setValue(settings.value("iterationsSpin").toInt());
    if(settings.contains("displayIterationSpin")) algo->optionsReinforcement->displayIterationSpin->setValue(settings.value("displayIterationSpin").toInt());
    if(settings.contains("problemCombo")) algo->optionsReinforcement->problemCombo->setCurrentIndex(settings.value("problemCombo").toInt());
    if(settings.contains("rewardCombo")) algo->optionsReinforcement->rewardCombo->setCurrentIndex(settings.value("rewardCombo").toInt());
    if(settings.contains("policyCombo")) algo->optionsReinforcement->policyCombo->setCurrentIndex(settings.value("policyCombo").toInt());
    if(settings.contains("quantizeCombo")) algo->optionsReinforcement->quantizeCombo->setCurrentIndex(settings.value("quantizeCombo").toInt());
    if(settings.contains("resolutionSpin")) algo->optionsReinforcement->resolutionSpin->setValue(settings.value("resolutionSpin").toInt());
    if(settings.contains("benchmarkCombo")) algo->optionsReinforcement->benchmarkCombo->setCurrentIndex(settings.value("benchmarkCombo").toInt());
    settings.endGroup();

    settings.beginGroup("projectOptions");
    if(settings.contains("tab")) algo->optionsProject->algoList->setCurrentIndex(settings.value("tab").toInt());
    if(settings.contains("fitCheck")) algo->optionsProject->fitCheck->setChecked(settings.value("fitCheck").toBool());
    settings.endGroup();

    settings.beginGroup("statsOptions");
    if(settings.contains("tab")) showStats->tabWidget->setCurrentIndex(settings.value("tab").toInt());
    settings.endGroup();

    settings.beginGroup("compareOptions");
    if(settings.contains("foldCount")) compare->params->foldCountSpin->setValue(settings.value("foldCount").toFloat());
    if(settings.contains("trainRatio")) compare->params->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
    settings.endGroup();

    settings.beginGroup("generatorOptions");
    if(settings.contains("generatorCombo")) generator->ui->generatorCombo->setCurrentIndex(settings.value("generatorCombo").toInt());
    if(settings.contains("countSpin")) generator->ui->countSpin->setValue(settings.value("countSpin").toInt());
    if(settings.contains("dimSpin")) generator->ui->dimSpin->setValue(settings.value("dimSpin").toInt());
    if(settings.contains("gridCountSpin")) generator->ui->gridCountSpin->setValue(settings.value("gridCountSpin").toInt());
    if(settings.contains("classesCount")) generator->ui->classesCount->setValue(settings.value("classesCount").toInt());
    if(settings.contains("radiusSpin")) generator->ui->radiusSpin->setValue(settings.value("radiusSpin").toFloat());
    settings.endGroup();

    if(algo->optionsClassify->algoList->currentIndex() < 0 ||algo->optionsClassify->algoList->currentIndex() > algo->optionsClassify->algoList->count()) algo->optionsClassify->algoList->setCurrentIndex(0);
    if(algo->optionsCluster->algoList->currentIndex() < 0 || algo->optionsCluster->algoList->currentIndex() > algo->optionsCluster->algoList->count()) algo->optionsCluster->algoList->setCurrentIndex(0);
    if(algo->optionsDynamic->algoList->currentIndex() < 0 || algo->optionsDynamic->algoList->currentIndex() > algo->optionsDynamic->algoList->count()) algo->optionsDynamic->algoList->setCurrentIndex(0);
    if(algo->optionsMaximize->algoList->currentIndex() < 0 || algo->optionsMaximize->algoList->currentIndex() > algo->optionsMaximize->algoList->count()) algo->optionsMaximize->algoList->setCurrentIndex(0);
    if(algo->optionsProject->algoList->currentIndex() < 0 || algo->optionsProject->algoList->currentIndex() > algo->optionsProject->algoList->count()) algo->optionsProject->algoList->setCurrentIndex(0);
    if(algo->optionsRegress->algoList->currentIndex() < 0 || algo->optionsRegress->algoList->currentIndex() > algo->optionsRegress->algoList->count()) algo->optionsRegress->algoList->setCurrentIndex(0);
    if(algo->optionsReinforcement->algoList->currentIndex() < 0 || algo->optionsReinforcement->algoList->currentIndex() > algo->optionsReinforcement->algoList->count()) algo->optionsReinforcement->algoList->setCurrentIndex(0);

    FOR(i,algo->classifiers.size())
    {
        if(!algo->classifiers[i]) continue;
        settings.beginGroup(QString("plugins::classifiers::") + algo->classifiers[i]->GetName());
        algo->classifiers[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->clusterers.size())
    {
        if(!algo->clusterers[i]) continue;
        settings.beginGroup(QString("plugins::clusterers::") + algo->clusterers[i]->GetName());
        algo->clusterers[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->regressors.size())
    {
        if(!algo->regressors[i]) continue;
        settings.beginGroup(QString("plugins::regressors::") + algo->regressors[i]->GetName());
        algo->regressors[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->dynamicals.size())
    {
        if(!algo->dynamicals[i]) continue;
        settings.beginGroup(QString("plugins::dynamicals::") + algo->dynamicals[i]->GetName());
        algo->dynamicals[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->maximizers.size())
    {
        if(!algo->maximizers[i]) continue;
        settings.beginGroup(QString("plugins::maximizers::") + algo->maximizers[i]->GetName());
        algo->maximizers[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->reinforcements.size())
    {
        if(!algo->reinforcements[i]) continue;
        settings.beginGroup(QString("plugins::reinforcements::") + algo->reinforcements[i]->GetName());
        algo->reinforcements[i]->LoadOptions(settings);
        settings.endGroup();
    }
    FOR(i,algo->projectors.size())
    {
        if(!algo->projectors[i]) continue;
        settings.beginGroup(QString("plugins::projectors::") + algo->projectors[i]->GetName());
        algo->projectors[i]->LoadOptions(settings);
        settings.endGroup();
    }
    canvas->repaint();
}


void MLDemos::SaveParams( QString filename )
{
    if(!algo->classifier && !algo->regressor && !algo->clusterer && !algo->dynamical && !algo->maximizer) return;
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
    if(algo->classifier)
    {
        int tab = algo->optionsClassify->algoList->currentIndex();
        sprintf(groupName,"classificationOptions");
        out << groupName << ":" << "tab" << " " << algo->optionsClassify->algoList->currentIndex() << "\n";
        out << groupName << ":" << "positiveClass" << " " << algo->optionsClassify->positiveSpin->value() << "\n";
        out << groupName << ":" << "binaryCheck" << " " << algo->optionsClassify->binaryCheck->isChecked() << "\n";
        if(tab < algo->classifiers.size() && algo->classifiers[tab])
        {
            algo->classifiers[tab]->SaveParams(out);
        }
    }
    if(algo->regressor)
    {
        int tab = algo->optionsRegress->algoList->currentIndex();
        sprintf(groupName,"regressionOptions");
        out << groupName << ":" << "tab" << " " << algo->optionsRegress->algoList->currentIndex() << "\n";
        out << groupName << ":" << "outputDimCombo" << " " << algo->optionsRegress->outputDimCombo->currentIndex() << "\n";
        if(tab < algo->regressors.size() && algo->regressors[tab])
        {
            algo->regressors[tab]->SaveParams(out);
        }
    }
    if(algo->dynamical)
    {
        int tab = algo->optionsDynamic->algoList->currentIndex();
        sprintf(groupName,"dynamicalOptions");
        out << groupName << ":" << "centerType" << " " << algo->optionsDynamic->centerCombo->currentIndex() << "\n";
        out << groupName << ":" << "zeroCheck" << " " << algo->optionsDynamic->zeroCheck->isChecked() << "\n";
        out << groupName << ":" << "resampleType" << " " << algo->optionsDynamic->resampleCombo->currentIndex() << "\n";
        out << groupName << ":" << "resampleCount" << " " << algo->optionsDynamic->resampleSpin->value() << "\n";
        out << groupName << ":" << "obstacleType" << " " << algo->optionsDynamic->obstacleCombo->currentIndex() << "\n";
        out << groupName << ":" << "dT" << " " << algo->optionsDynamic->dtSpin->value() << "\n";
        out << groupName << ":" << "colorCheck" << " " << algo->optionsDynamic->colorCheck->isChecked() << "\n";
        out << groupName << ":" << "tab" << " " << algo->optionsDynamic->algoList->currentIndex() << "\n";
        if(tab < algo->dynamicals.size() && algo->dynamicals[tab])
        {
            algo->dynamicals[tab]->SaveParams(out);
        }
    }
    if(algo->clusterer)
    {
        int tab = algo->optionsCluster->algoList->currentIndex();
        sprintf(groupName,"clusterOptions");
        out << groupName << ":" << "tab" << " " << algo->optionsCluster->algoList->currentIndex() << "\n";
        out << groupName << ":" << "trainRatio" << " " << algo->optionsCluster->trainRatioCombo->currentIndex() << "\n";
        out << groupName << ":" << "trainTestCombo" << " " << algo->optionsCluster->trainTestCombo->currentIndex() << "\n";
        out << groupName << ":" << "optimizeCombo" << " " << algo->optionsCluster->optimizeCombo->currentIndex() << "\n";
        out << groupName << ":" << "rangeStart" << " " << algo->optionsCluster->rangeStartSpin->value() << "\n";
        out << groupName << ":" << "rangeStop" << " " << algo->optionsCluster->rangeStopSpin->value() << "\n";
        if(tab < algo->clusterers.size() && algo->clusterers[tab])
        {
            algo->clusterers[tab]->SaveParams(out);
        }
    }
    if(algo->maximizer)
    {
        int tab = algo->optionsMaximize->algoList->currentIndex();
        double variance = algo->optionsMaximize->varianceSpin->value();
        sprintf(groupName,"maximizationOptions");
        out << groupName << ":" << "tab" << " " << algo->optionsMaximize->algoList->currentIndex() << "\n";
        out << groupName << ":" << "gaussVarianceSpin" << " " << algo->optionsMaximize->varianceSpin->value() << "\n";
        out << groupName << ":" << "iterationsSpin" << " " << algo->optionsMaximize->iterationsSpin->value() << "\n";
        out << groupName << ":" << "stoppingSpin" << " " << algo->optionsMaximize->stoppingSpin->value() << "\n";
        out << groupName << ":" << "benchmarkCombo" << " " << algo->optionsMaximize->benchmarkCombo->currentIndex() << "\n";
        if(tab < algo->maximizers.size() && algo->maximizers[tab])
        {
            algo->maximizers[tab]->SaveParams(out);
        }
    }
    if(algo->reinforcement)
    {
        int tab = algo->optionsReinforcement->algoList->currentIndex();
        double variance = algo->optionsReinforcement->varianceSpin->value();
        sprintf(groupName,"reinforcementOptions");
        out << groupName << ":" << "tab" << " " << algo->optionsReinforcement->algoList->currentIndex() << "\n";
        out << groupName << ":" << "gaussVarianceSpin" << " " << algo->optionsReinforcement->varianceSpin->value() << "\n";
        out << groupName << ":" << "iterationsSpin" << " " << algo->optionsReinforcement->iterationsSpin->value() << "\n";
        out << groupName << ":" << "displayIterationSpin" << " " << algo->optionsReinforcement->displayIterationSpin->value() << "\n";
        out << groupName << ":" << "problemCombo" << " " << algo->optionsReinforcement->problemCombo->currentIndex() << "\n";
        out << groupName << ":" << "rewardCombo" << " " << algo->optionsReinforcement->rewardCombo->currentIndex() << "\n";
        out << groupName << ":" << "policyCombo" << " " << algo->optionsReinforcement->policyCombo->currentIndex() << "\n";
        out << groupName << ":" << "quantizeCombo" << " " << algo->optionsReinforcement->quantizeCombo->currentIndex() << "\n";
        out << groupName << ":" << "resolutionSpin" << " " << algo->optionsReinforcement->resolutionSpin->value() << "\n";
        out << groupName << ":" << "benchmarkCombo" << " " << algo->optionsReinforcement->benchmarkCombo->currentIndex() << "\n";

        if(tab < algo->reinforcements.size() && algo->reinforcements[tab])
        {
            algo->reinforcements[tab]->SaveParams(out);
        }
    }
    if(algo->projector)
    {
        int tab = algo->optionsProject->algoList->currentIndex();
        sprintf(groupName,"projectOptions");
        out << groupName << ":" << "tab" << " " << algo->optionsProject->algoList->currentIndex() << "\n";
        out << groupName << ":" << "fitCheck" << " " << algo->optionsProject->fitCheck->isChecked() << "\n";
        if(tab < algo->projectors.size() && algo->projectors[tab])
        {
            algo->projectors[tab]->SaveParams(out);
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
            algo->options->tabWidget->setCurrentWidget(algo->options->tabClass);
            if(line.endsWith("tab")) algo->optionsClassify->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("positiveClass")) algo->optionsClassify->positiveSpin->setValue((int)value);
            if(line.endsWith("binaryCheck")) algo->optionsClassify->binaryCheck->setChecked((int)value);
            if(tab < algo->classifiers.size() && algo->classifiers[tab]) algo->classifiers[tab]->LoadParams(line,value);
        }
        if(line.startsWith(regrGroup))
        {
            bRegr = true;
            algo->options->tabWidget->setCurrentWidget(algo->options->tabRegr);
            if(line.endsWith("tab")) algo->optionsRegress->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("outputDimCombo")) algo->optionsRegress->outputDimCombo->setCurrentIndex((int)value);
            if(tab < algo->regressors.size() && algo->regressors[tab]) algo->regressors[tab]->LoadParams(line,value);
        }
        if(line.startsWith(dynGroup))
        {
            bDyn = true;
            algo->options->tabWidget->setCurrentWidget(algo->options->tabDyn);
            if(line.endsWith("centerType")) algo->optionsDynamic->centerCombo->setCurrentIndex((int)value);
            if(line.endsWith("zeroCheck")) algo->optionsDynamic->zeroCheck->setChecked((int)value);
            if(line.endsWith("resampleType")) algo->optionsDynamic->resampleCombo->setCurrentIndex((int)value);
            if(line.endsWith("resampleCount")) algo->optionsDynamic->resampleSpin->setValue((int)value);
            if(line.endsWith("obstacleType")) algo->optionsDynamic->obstacleCombo->setCurrentIndex((int)value);
            if(line.endsWith("dT")) algo->optionsDynamic->dtSpin->setValue((float)value);
            if(line.endsWith("colorCheck")) algo->optionsDynamic->colorCheck->setChecked((int)value);
            if(line.endsWith("tab")) algo->optionsDynamic->algoList->setCurrentIndex(tab = (int)value);
            if(tab < algo->dynamicals.size() && algo->dynamicals[tab]) algo->dynamicals[tab]->LoadParams(line,value);
        }
        if(line.startsWith(clustGroup))
        {
            bClust = true;
            algo->options->tabWidget->setCurrentWidget(algo->options->tabClust);
            if(line.endsWith("tab")) algo->optionsCluster->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("trainRatio")) algo->optionsCluster->trainRatioCombo->setCurrentIndex((int)value);
            if(line.endsWith("trainTestCombo")) algo->optionsCluster->trainTestCombo->setCurrentIndex((int)value);
            if(line.endsWith("optimizeCombo")) algo->optionsCluster->optimizeCombo->setCurrentIndex((int)value);
            if(line.endsWith("rangeStart")) algo->optionsCluster->rangeStartSpin->setValue((int)value);
            if(line.endsWith("rangeStop")) algo->optionsCluster->rangeStopSpin->setValue((int)value);
            if(tab < algo->clusterers.size() && algo->clusterers[tab]) algo->clusterers[tab]->LoadParams(line,value);
        }
        if(line.startsWith(maximGroup))
        {
            bMaxim = true;
            algo->options->tabWidget->setCurrentWidget(algo->options->tabMax);
            if(line.endsWith("tab")) algo->optionsMaximize->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("gaussVarianceSpin")) algo->optionsMaximize->varianceSpin->setValue((double)value);
            if(line.endsWith("iterationsSpin")) algo->optionsMaximize->iterationsSpin->setValue((int)value);
            if(line.endsWith("stoppingSpin")) algo->optionsMaximize->stoppingSpin->setValue((double)value);
            if(line.endsWith("benchmarkCombo")) algo->optionsMaximize->benchmarkCombo->setCurrentIndex((int)value);
            if(tab < algo->maximizers.size() && algo->maximizers[tab]) algo->maximizers[tab]->LoadParams(line,value);
        }
        if(line.startsWith(reinfGroup))
        {
            bReinf = true;
            algo->options->tabWidget->setCurrentWidget(algo->options->tabReinf);
            if(line.endsWith("tab")) algo->optionsReinforcement->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("gaussVarianceSpin")) algo->optionsReinforcement->varianceSpin->setValue((double)value);
            if(line.endsWith("iterationsSpin")) algo->optionsReinforcement->iterationsSpin->setValue((int)value);
            if(line.endsWith("displayIterationSpin")) algo->optionsReinforcement->displayIterationSpin->setValue((int)value);
            if(line.endsWith("problemCombo")) algo->optionsReinforcement->problemCombo->setCurrentIndex((int)value);
            if(line.endsWith("rewardCombo")) algo->optionsReinforcement->rewardCombo->setCurrentIndex((int)value);
            if(line.endsWith("policyCombo")) algo->optionsReinforcement->policyCombo->setCurrentIndex((int)value);
            if(line.endsWith("quantizeCombo")) algo->optionsReinforcement->quantizeCombo->setCurrentIndex((int)value);
            if(line.endsWith("resolutionSpin")) algo->optionsReinforcement->resolutionSpin->setValue((int)value);
            if(line.endsWith("benchmarkCombo")) algo->optionsReinforcement->benchmarkCombo->setCurrentIndex((int)value);
            if(tab < algo->reinforcements.size() && algo->reinforcements[tab]) algo->reinforcements[tab]->LoadParams(line,value);

        }
        if(line.startsWith(projGroup))
        {
            bProj = true;
            algo->options->tabWidget->setCurrentWidget(algo->options->tabProj);
            if(line.endsWith("tab")) algo->optionsProject->algoList->setCurrentIndex(tab = (int)value);
            if(line.endsWith("fitCheck")) algo->optionsProject->fitCheck->setChecked((int)value);
            if(tab < algo->projectors.size() && algo->projectors[tab]) algo->projectors[tab]->LoadParams(line,value);
        }
    }
    ResetPositiveClass();
    ManualSelectionUpdated();
    InputDimensionsUpdated();
    if(bClass) algo->Classify();
    if(bRegr) algo->Regression();
    if(bDyn) algo->Dynamize();
    if(bClust) algo->Cluster();
    if(bProj) algo->Project();
    actionAlgorithms->setChecked(algo->algorithmWidget->isVisible());
}

void MLDemos::ExportOutput()
{
    if(!algo->classifier && !algo->regressor && !algo->clusterer && !algo->projector) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Output Data"), "", tr("Data (*.txt *.csv)"));
    if(filename.isEmpty()) return;
    if(!filename.endsWith(".txt") && !filename.endsWith(".csv")) filename += ".txt";

    QFile file(filename);
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    if(!file.isOpen()) return;

    if(algo->classifier || algo->clusterer || algo->regressor)
    {
        out << "#Sample(n-dims) Label ComputedValue(s)\n";
        vector<fvec> samples = algo->sourceDims.size() ? canvas->data->GetSampleDims(algo->sourceDims) : canvas->data->GetSamples();
        ivec labels = canvas->data->GetLabels();
        FOR(i, samples.size())
        {
            fvec &sample = samples[i];
            fvec res;
            if(algo->classifier) res = algo->classifier->TestMulti(sample);
            else if (algo->clusterer) res = algo->clusterer->Test(sample);
            else if (algo->regressor) res = algo->regressor->Test(sample);
            FOR(d, sample.size()) out << QString("%1,").arg(sample[d]);
            out << QString("%1,").arg(labels[i]);
            FOR(d, res.size()) out << QString("%1%2").arg(res[d]).arg(d<res.size()-1?",":"");
            out << "\n";
        }
    }
    else if(algo->projector)
    {
        //out << "#Sample x (n-dims), Label, Projected x (m-dims)\n";
        vector<fvec> samples = algo->projector->source;
        //ivec labels = canvas->data->GetLabels();
        FOR(i, samples.size())
        {
            fvec &sample = samples[i];
            fvec projected = algo->projector->Project(sample);
            //FOR(d, sample.size()) out << QString("%1,").arg(sample[d]);
            //out << QString("%1").arg(labels[i]);
            FOR(d, projected.size()) out << QString("%1%2").arg(projected[d]).arg(d<projected.size()-1?",":"");
            out << "\n";
        }
    }
    file.close();
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
