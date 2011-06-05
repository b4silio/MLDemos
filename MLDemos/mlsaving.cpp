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

	settings.setValue("algoTab", algorithmOptions->tabWidget->currentIndex());
	settings.setValue("ShowAlgoOptions", algorithmWidget->isVisible());
	settings.setValue("ShowDrawOptions", drawToolbarWidget->isVisible());
	settings.setValue("ShowDisplayOptions", displayDialog->isVisible());
	settings.setValue("ShowStatsOptions", statsDialog->isVisible());
	settings.endGroup();

	settings.beginGroup("displayOptions");
	settings.setValue("infoCheck", displayOptions->infoCheck->isChecked());
	settings.setValue("mapCheck", displayOptions->mapCheck->isChecked());
	settings.setValue("modelCheck", displayOptions->modelCheck->isChecked());
	settings.setValue("samplesCheck", displayOptions->samplesCheck->isChecked());
	settings.setValue("gridCheck", displayOptions->gridCheck->isChecked());
	settings.setValue("spinZoom", displayOptions->spinZoom->value());
	settings.endGroup();

	settings.beginGroup("drawingOptions");
	settings.setValue("infoCheck", drawToolbarContext1->randCombo->currentIndex());
	settings.setValue("spinCount", drawToolbarContext1->spinCount->value());
	settings.setValue("spinSize", drawToolbarContext1->spinSize->value());
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
	settings.setValue("foldCount", optionsClassify->foldCountSpin->value());
	settings.setValue("trainRatio", optionsClassify->traintestRatioCombo->currentIndex());
	settings.setValue("tab", optionsClassify->tabWidget->currentIndex());
	settings.endGroup();

	settings.beginGroup("regressionOptions");
	settings.setValue("foldCount", optionsRegress->foldCountSpin->value());
	settings.setValue("trainRatio", optionsRegress->traintestRatioCombo->currentIndex());
	settings.setValue("tab", optionsRegress->tabWidget->currentIndex());
	settings.endGroup();

	settings.beginGroup("dynamicalOptions");
	settings.setValue("centerType", optionsDynamic->centerCombo->currentIndex());
	settings.setValue("zeroCheck", optionsDynamic->zeroCheck->isChecked());
	settings.setValue("resampleType", optionsDynamic->resampleCombo->currentIndex());
	settings.setValue("resampleCount", optionsDynamic->resampleSpin->value());
	settings.setValue("obstacleType", optionsDynamic->obstacleCombo->currentIndex());
	settings.setValue("dT", optionsDynamic->dtSpin->value());
	settings.setValue("tab", optionsDynamic->tabWidget->currentIndex());
	settings.endGroup();

	settings.beginGroup("clusterOptions");
	settings.setValue("tab", optionsCluster->tabWidget->currentIndex());
	settings.endGroup();

	settings.beginGroup("maximizeOptions");
	settings.setValue("tab", optionsMaximize->tabWidget->currentIndex());
	settings.setValue("varianceSpin", optionsMaximize->varianceSpin->value());
	settings.endGroup();

	settings.beginGroup("statsOptions");
	settings.setValue("tab", showStats->tabWidget->currentIndex());
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
#ifdef MACX // ugly hack to avoid resizing problems on the mac
	if(height() < 400) resize(width(),400);
	if(algorithmWidget->height() < 220) algorithmWidget->resize(636,220);
#endif // MACX

	if(settings.contains("algoTab")) algorithmOptions->tabWidget->setCurrentIndex(settings.value("algoTab").toInt());
	if(settings.contains("ShowAlgoOptions")) algorithmWidget->setVisible(settings.value("ShowAlgoOptions").toBool());
	if(settings.contains("ShowDrawOptions")) drawToolbarWidget->setVisible(settings.value("ShowDrawOptions").toBool());
	if(settings.contains("ShowDisplayOptions")) displayDialog->setVisible(settings.value("ShowDisplayOptions").toBool());
	if(settings.contains("ShowStatsOptions")) statsDialog->setVisible(settings.value("ShowStatsOptions").toBool());
	settings.endGroup();

	actionClassifiers->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabClass);
	actionRegression->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabRegr);
	actionDynamical->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabDyn);
	actionClustering->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabClust);
	actionMaximizers->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabMax);
	actionDrawSamples->setChecked(drawToolbarWidget->isVisible());
	actionDisplayOptions->setChecked(displayDialog->isVisible());
	actionShowStats->setChecked(statsDialog->isVisible());

	settings.beginGroup("displayOptions");
	if(settings.contains("infoCheck")) displayOptions->infoCheck->setChecked(settings.value("infoCheck").toBool());
	if(settings.contains("mapCheck")) displayOptions->mapCheck->setChecked(settings.value("mapCheck").toBool());
	if(settings.contains("modelCheck")) displayOptions->modelCheck->setChecked(settings.value("modelCheck").toBool());
	if(settings.contains("samplesCheck")) displayOptions->samplesCheck->setChecked(settings.value("samplesCheck").toBool());
	if(settings.contains("gridCheck")) displayOptions->gridCheck->setChecked(settings.value("gridCheck").toBool());
	if(settings.contains("spinZoom")) displayOptions->spinZoom->setValue(settings.value("spinZoom").toFloat());
	settings.endGroup();

	settings.beginGroup("drawingOptions");
	if(settings.contains("infoCheck")) drawToolbarContext1->randCombo->setCurrentIndex(settings.value("infoCheck").toInt());
	if(settings.contains("spinAngle")) drawToolbarContext2->spinAngle->setValue(settings.value("spinAngle").toFloat());
	if(settings.contains("spinSize")) drawToolbarContext1->spinSize->setValue(settings.value("spinSize").toFloat());
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
	if(settings.contains("eraseCheck")) drawToolbar->eraseButton->setChecked(settings.value("eraseCheck").toBool());
	if(settings.contains("sprayCheck")) drawToolbar->sprayButton->setChecked(settings.value("sprayCheck").toBool());
	if(settings.contains("singleCheck")) drawToolbar->singleButton->setChecked(settings.value("singleCheck").toBool());
	if(settings.contains("ellipseCheck")) drawToolbar->ellipseButton->setChecked(settings.value("ellipseCheck").toBool());
	if(settings.contains("lineCheck")) drawToolbar->lineButton->setChecked(settings.value("lineCheck").toBool());
	if(settings.contains("trajectoryCheck")) drawToolbar->trajectoryButton->setChecked(settings.value("trajectoryCheck").toBool());
	if(settings.contains("obstacleCheck")) drawToolbar->obstacleButton->setChecked(settings.value("obstacleCheck").toBool());
	if(settings.contains("paintCheck")) drawToolbar->paintButton->setChecked(settings.value("paintCheck").toBool());
	settings.endGroup();

	settings.beginGroup("classificationOptions");
	if(settings.contains("positiveClass")) optionsClassify->positiveSpin->setValue(settings.value("positiveClass").toFloat());
	if(settings.contains("foldCount")) optionsClassify->foldCountSpin->setValue(settings.value("foldCount").toFloat());
	if(settings.contains("trainRatio")) optionsClassify->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
	if(settings.contains("tab")) optionsClassify->tabWidget->setCurrentIndex(settings.value("tab").toInt());
	settings.endGroup();

	settings.beginGroup("regressionOptions");
	if(settings.contains("foldCount")) optionsRegress->foldCountSpin->setValue(settings.value("foldCount").toFloat());
	if(settings.contains("trainRatio")) optionsRegress->traintestRatioCombo->setCurrentIndex(settings.value("trainRatio").toInt());
	if(settings.contains("tab")) optionsRegress->tabWidget->setCurrentIndex(settings.value("tab").toInt());
	settings.endGroup();

	settings.beginGroup("dynamicalOptions");
	if(settings.contains("centerType")) optionsDynamic->centerCombo->setCurrentIndex(settings.value("centerType").toInt());
	if(settings.contains("zeroCheck")) optionsDynamic->zeroCheck->setChecked(settings.value("zeroCheck").toBool());
	if(settings.contains("resampleType")) optionsDynamic->resampleCombo->setCurrentIndex(settings.value("resampleType").toInt());
	if(settings.contains("resampleCount")) optionsDynamic->resampleSpin->setValue(settings.value("resampleCount").toFloat());
	if(settings.contains("obstacleType")) optionsDynamic->obstacleCombo->setCurrentIndex(settings.value("obstacleType").toInt());
	if(settings.contains("dT")) optionsDynamic->dtSpin->setValue(settings.value("dT").toFloat());
	if(settings.contains("tab")) optionsDynamic->tabWidget->setCurrentIndex(settings.value("tab").toInt());
	settings.endGroup();

	settings.beginGroup("clusterOptions");
	if(settings.contains("tab")) optionsCluster->tabWidget->setCurrentIndex(settings.value("tab").toInt());
	settings.endGroup();

	settings.beginGroup("maximizeOptions");
	if(settings.contains("tab")) optionsMaximize->tabWidget->setCurrentIndex(settings.value("tab").toInt());
	if(settings.contains("varianceSpin")) optionsMaximize->varianceSpin->setValue(settings.value("varianceSpin").toDouble());
	settings.endGroup();

	settings.beginGroup("statsOptions");
	if(settings.contains("tab")) showStats->tabWidget->setCurrentIndex(settings.value("tab").toInt());
	settings.endGroup();

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
	canvas->repaint();
}


void MLDemos::SaveParams( QString filename )
{
	if(!classifier && !regressor && !clusterer) return;

	ofstream file(filename.toAscii(), ios::out | ios::app);
	if(!file.is_open()) return;

	char groupName[255];

	if(classifier)
	{
		int tab = optionsClassify->tabWidget->currentIndex();
		sprintf(groupName,"classificationOptions");
		file << groupName << ":" << "tab" << " " << optionsClassify->tabWidget->currentIndex() << std::endl;
		file << groupName << ":" << "positiveClass" << " " << optionsClassify->positiveSpin->value() << std::endl;
		if(tab < classifiers.size() && classifiers[tab])
		{
			classifiers[tab]->SaveParams(file);
		}
	}
	if(regressor)
	{
		int tab = optionsRegress->tabWidget->currentIndex();
		sprintf(groupName,"regressionOptions");
		file << groupName << ":" << "tab" << " " << optionsRegress->tabWidget->currentIndex() << std::endl;
		if(tab < regressors.size() && regressors[tab])
		{
			regressors[tab]->SaveParams(file);
		}
	}
	if(dynamical)
	{
		int tab = optionsDynamic->tabWidget->currentIndex();
		sprintf(groupName,"dynamicalOptions");
		file << groupName << ":" << "centerType" << " " << optionsDynamic->centerCombo->currentIndex() << std::endl;
		file << groupName << ":" << "zeroCheck" << " " << optionsDynamic->zeroCheck->isChecked() << std::endl;
		file << groupName << ":" << "resampleType" << " " << optionsDynamic->resampleCombo->currentIndex() << std::endl;
		file << groupName << ":" << "resampleCount" << " " << optionsDynamic->resampleSpin->value() << std::endl;
		file << groupName << ":" << "obstacleType" << " " << optionsDynamic->obstacleCombo->currentIndex() << std::endl;
		file << groupName << ":" << "dT" << " " << optionsDynamic->dtSpin->value() << std::endl;
		file << groupName << ":" << "tab" << " " << optionsDynamic->tabWidget->currentIndex() << std::endl;
		if(tab < dynamicals.size() && dynamicals[tab])
		{
			dynamicals[tab]->SaveParams(file);
		}
	}
	if(clusterer)
	{
		int tab = optionsCluster->tabWidget->currentIndex();
		sprintf(groupName,"clusterOptions");
		file << groupName << ":" << "tab" << " " << optionsCluster->tabWidget->currentIndex() << std::endl;
		if(tab < clusterers.size() && clusterers[tab])
		{
			clusterers[tab]->SaveParams(file);
		}
	}
	if(maximizer)
	{
		int tab = optionsMaximize->tabWidget->currentIndex();
		double variance = optionsMaximize->varianceSpin->value();
		sprintf(groupName,"maximizationOptions");
		file << groupName << ":" << "tab" << " " << optionsMaximize->tabWidget->currentIndex() << std::endl;
		file << groupName << ":" << "varianceSpin" << " " << optionsMaximize->varianceSpin->value() << std::endl;
		if(tab < maximizers.size() && maximizers[tab])
		{
			maximizers[tab]->SaveParams(file);
		}
	}

	file.close();
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
	ifstream file(filename.toAscii());
	if(!file.is_open()) return;

	int sampleCnt, size;
	file >> sampleCnt;
	file >> size;
	char line[255];
	float value;

	char classGroup[255];
	char regrGroup[255];
	char dynGroup[255];
	char clustGroup[255];
	char maximGroup[255];
	sprintf(classGroup,"classificationOptions");
	sprintf(regrGroup,"regressionOptions");
	sprintf(dynGroup,"dynamicalOptions");
	sprintf(clustGroup,"clusteringOptions");
	sprintf(maximGroup,"maximizationgOptions");

	// we skip the samples themselves
	qDebug() << "Skipping "<< sampleCnt <<" samples" << endl;
	FOR(i, sampleCnt) file.getline(line,255);
	bool bClass = false, bRegr = false, bDyn = false, bClust = false, bMaxim = false;
	qDebug() << "Loading parameter list" << endl;
	int tab = 0;
	while(!file.eof())
	{
		file >> line;
		file >> value;
		qDebug() << line << " " << value << endl;
		if(startsWith(line, classGroup))
		{
			bClass = true;
			algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabClass);
			if(endsWith(line,"tab")) optionsClassify->tabWidget->setCurrentIndex(tab = (int)value);
			if(endsWith(line,"positiveClass")) optionsClassify->positiveSpin->setValue((int)value);
			if(tab < classifiers.size() && classifiers[tab]) classifiers[tab]->LoadParams(line,value);
		}
		if(startsWith(line, regrGroup))
		{
			bRegr = true;
			algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabRegr);
			if(endsWith(line,"tab")) optionsRegress->tabWidget->setCurrentIndex((int)value);
			if(tab < regressors.size() && regressors[tab]) regressors[tab]->LoadParams(line,value);
		}
		if(startsWith(line, dynGroup))
		{
			bDyn = true;
			algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabDyn);
			if(endsWith(line,"centerType")) optionsDynamic->centerCombo->setCurrentIndex((int)value);
			if(endsWith(line,"zeroCheck")) optionsDynamic->zeroCheck->setChecked((int)value);
			if(endsWith(line,"resampleType")) optionsDynamic->resampleCombo->setCurrentIndex((int)value);
			if(endsWith(line,"resampleCount")) optionsDynamic->resampleSpin->setValue((int)value);
			if(endsWith(line,"obstacleType")) optionsDynamic->obstacleCombo->setCurrentIndex((int)value);
			if(endsWith(line,"dT")) optionsDynamic->dtSpin->setValue((int)value);
			if(endsWith(line,"tab")) optionsDynamic->tabWidget->setCurrentIndex((int)value);
			if(tab < dynamicals.size() && dynamicals[tab]) dynamicals[tab]->LoadParams(line,value);
		}
		if(startsWith(line, clustGroup))
		{
			bClust = true;
			algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabClust);
			if(endsWith(line,"tab")) optionsCluster->tabWidget->setCurrentIndex((int)value);
			if(tab < clusterers.size() && clusterers[tab]) clusterers[tab]->LoadParams(line,value);
		}
		if(startsWith(line, maximGroup))
		{
			bMaxim = true;
			algorithmOptions->tabWidget->setCurrentWidget(algorithmOptions->tabMax);
			if(endsWith(line,"tab")) optionsMaximize->tabWidget->setCurrentIndex((int)value);
			if(endsWith(line,"varianceSpin")) optionsMaximize->varianceSpin->setValue((double)value);
			if(tab < maximizers.size() && maximizers[tab]) maximizers[tab]->LoadParams(line,value);
		}
	}
	file.close();
	ResetPositiveClass();
	if(bClass) Classify();
	if(bRegr) Regression();
	if(bDyn) Dynamize();
	if(bClust) Cluster();
	if(bMaxim) Maximize();
	if(algorithmWidget->isVisible())
	{
		actionClassifiers->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabClass);
		actionRegression->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabRegr);
		actionDynamical->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabDyn);
		actionClustering->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabClust);
		actionMaximizers->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabMax);
		//actionClustering->setChecked(algorithmOptions->tabWidget->currentWidget() == algorithmOptions->tabMax);
	}
}
