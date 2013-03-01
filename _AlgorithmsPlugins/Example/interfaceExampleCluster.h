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
#ifndef _INTERFACEEXAMPLECLUSTER_H_
#define _INTERFACEEXAMPLECLUSTER_H_

#include <vector>
#include <interfaces.h>
#include "clustererExample.h"
#include "ui_paramsExample.h"

/**
  Example of plugin interface for a clustering algorithm
  */
class ClustExample : public QObject, public ClustererInterface
{
	Q_OBJECT
	Q_INTERFACES(ClustererInterface)
private:
    QWidget *widget; // the widget that will hold the hyperparameter panel
    Ui::ParametersExample *params; // the hyperparameter panel (instantiated from the ui form)
public:
    /*!
      Constructor, it instantiates the widget and parameter panel
      */
    ClustExample();

    /*!
      The function called by the main program to obtain the clusterer.
      It should generate the clusterer and set its parameter to match the user choice.
      */
    Clusterer *GetClusterer();

    /*!
      The function called by the main program to draw the learned model (e.g. the classified samples)
      */
    void DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer);

    /*!
      The function called by the main program to draw the model information
      (e.g. the projection axes, cluster centers or class boundaries, if these are available)
      */
    void DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer);

	// virtual functions to manage the GUI and I/O
    /*!
      Function that returns the name of the algorithm (will appear in the algorithm options panel tab)
      */
    QString GetName(){return QString("Example");}

    /*!
      The unique string describing the algorithm and options currently used (displayed in the Compare panel)
      */
    QString GetAlgoString(){ return GetName();}

    /*!
      Link to the html page containing a description of the algorithm and/or implementation details
      */
    QString GetInfoFile(){return "Example.html";}

    /*!
      Returns the option widget, used by the main program to get the hyperparameter panel
      */
    QWidget *GetParameterWidget(){return widget;}

    /*!
      Sets the parameter for the classifier
      */
    void SetParams(Clusterer *clusterer);

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
};

#endif // _INTERFACEEXAMPLECLUSTER_H_
