/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#ifndef _INTERFACEGPCLASSIFIER_H_
#define _INTERFACEGPCLASSIFIER_H_

#include <vector>
#include <interfaces.h>
#include "classifierGP.h"
#include "ui_paramsGPClassifier.h"

/**
  GP of plugin interface for a classifier
  */
class ClassGP : public QObject, public ClassifierInterface
{
	Q_OBJECT
	Q_INTERFACES(ClassifierInterface)
private:
    QWidget *widget; // the widget that will hold the hyperparameter panel
    Ui::ParametersGP *params; // the hyperparameter panel (instantiated from the ui form)

public:
    /*!
      Constructor, it instantiates the widget and parameter panel
      */
    ClassGP();
    ~ClassGP();

    /*!
      The function called by the main program to obtain the classifier.
      It should generate the classifier and set its parameter to match the user choice.
      */
    Classifier *GetClassifier();

    /*!
      The function called by the main program to draw the model information
      (e.g. the projection axes, cluster centers or class boundaries, if these are available)
      */
    void DrawInfo(Canvas *canvas, QPainter &painter, Classifier *classifier);


    /*!
      The function called by the main program to draw the algorithm information on OpenGL
      (e.g. the projection axes, cluster centers or class boundaries, if these are available)
      */
    void DrawGL(Canvas *canvas, GLWidget *glw, Classifier *classifier){}

    /*!
      Function that returns the name of the algorithm (will appear in the algorithm options panel tab)
      */
    QString GetName(){return QString("Gaussian Process Classification");}

    /*!
      The unique string describing the algorithm and options currently used (displayed in the Compare panel)
      */
    QString GetAlgoString();

    /*!
      Link to the html page containing a description of the algorithm and/or implementation details
      */
    QString GetInfoFile(){return "GP.html";}

    /*!
      Determines whether the plugin requires a timer to query informations from it
      */
    bool UsesDrawTimer(){return true;}

    /*!
      Returns the option widget, used by the main program to get the hyperparameter panel
      */
    QWidget *GetParameterWidget(){return widget;}

    /*!
      Sets the parameter for the classifier
      */
    void SetParams(Classifier *classifier);

    /*!
      Saves the hyperparameter options to the system's registry
      */
    void SaveOptions(QSettings &settings);

    /*!
      Loads the hyperparameter options from the system's registry
      */
    bool LoadOptions(QSettings &settings);

    /*!
      Saves the parameter to file (or to string for the compare dialog)
      */
    void SaveParams(QTextStream &stream);

    /*!
      Loads the parameter from file (or from string for the compare dialog)
      */
    bool LoadParams(QString name, float value);

    void SetParams(Classifier *classifier, fvec parameters);
    fvec GetParams();
    void GetParameterList(std::vector<QString> &parameterNames,
                                 std::vector<QString> &parameterTypes,
                                 std::vector< std::vector<QString> > &parameterValues);

};

#endif // _INTERFACEGPCLASSIFIER_H_
