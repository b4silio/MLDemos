/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Die Jan 25 12:10:48 CET 2000
    copyright            : (C) 2000 by Michael Dittenbach
    email                : mbach@ifs.tuwien.ac.at
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

void usage() {
  std::cout << "Usage: ghsom propertyfile" << std::endl;
}

void setProp(char *prop,char *val){
  if (strcmp(prop,"")==0) {
    std::cout << "   setProp: prop empty" <<std::endl;

  } else if (strcmp(prop,"EXPAND_CYCLES")==0) {
    Globals::EXPAND_CYCLES = atoi(val);
    std::cout << "   XXX setProp: ExpandCycles =" << Globals::EXPAND_CYCLES <<std::endl;
  } else if (strcmp(prop,"MAX_CYCLES")==0) {
    Globals::MAX_CYCLES = atoi(val);
  } else if (strcmp(prop,"TAU_1")==0) {
    Globals::TAU_1 = atof(val);
  } else if (strcmp(prop,"INITIAL_LEARNRATE")==0) {
    Globals::INITIAL_LEARNRATE = atof(val);
    //	} else if (strcmp(prop,"MIN_LEARNRATE")==0) {
    //		Globals::MIN_LEARNRATE = atof(val);
  } else if (strcmp(prop,"INITIAL_NEIGHBOURHOOD")==0) {
    Globals::INITIAL_NEIGHBOURHOOD = atoi(val);
    //	} else if (strcmp(prop,"MIN_NEIGHBOURHOOD")==0) {
    //		Globals::MIN_NEIGHBOURHOOD = atoi(val);
  } else if (strcmp(prop,"NEIGHBOURHOOD_FALLOFF")==0) {
    Globals::NEIGHBOURHOOD_FALLOFF = atof(val);
    //	} else if (strcmp(prop,"MAX_REPRESENTING")==0) {
    //		Globals::MAX_REPRESENTING = atoi(val);
  } else if (strcmp(prop,"TAU_2")==0) {
    Globals::TAU_2 = atof(val);
  } else if (strcmp(prop,"HTML_PREFIX")==0) {
    Globals::HTML_PREFIX=(char*)malloc(strlen(val)+1);
    strcpy(Globals::HTML_PREFIX,val);
  } else if (strcmp(prop,"DATAFILE_EXTENSION")==0) {
    Globals::DATAFILE_EXTENSION=(char*)malloc(strlen(val)+1);
    strcpy(Globals::DATAFILE_EXTENSION,val);
  } else if (strcmp(prop,"randomSeed")==0) {
    Globals::randomSeed = atol(val);
    Globals::setRandom(Globals::randomSeed);
  } else if (strcmp(prop,"inputFile")==0) {
    Globals::inputFile=(char*)malloc(strlen(val)+1);
    strcpy(Globals::inputFile,val);
    std::cout << "   XXX setProp: Input File =" << Globals::inputFile <<std::endl;
  } else if (strcmp(prop,"descriptionFile")==0) {
    Globals::descriptionFile=(char*)malloc(strlen(val)+1);
    strcpy(Globals::descriptionFile,val);
    std::cout << "   XXX setProp: Description File =" << Globals::descriptionFile <<std::endl;
  } else if (strcmp(prop,"savePath")==0) {
    Globals::savePath=(char*)malloc(strlen(val)+1);
    strcpy(Globals::savePath,val);
  } else if (strcmp(prop,"printMQE")==0) {
    if (strcmp(val,"true")==0) {
      Globals::printMQE = true;
    } else {
      Globals::printMQE = false;
    }
  } else if (strcmp(prop,"normInputVectors")==0) {
    if (strcmp(val,"NONE")==0) {
      Globals::normInputVectors = NORM_NONE;
    } else if (strcmp(val,"LENGTH")==0) {
      Globals::normInputVectors = NORM_LENGTH;
    } else if (strcmp(val,"INTERVAL")==0) {
      Globals::normInputVectors = NORM_INTERVAL;
    }
  } else if (strcmp(prop,"saveAsHTML")==0) {
    if (strcmp(val,"true")==0) {
      Globals::SAVE_AS_HTML = true;
    } else {
      Globals::SAVE_AS_HTML = false;
    }
  } else if (strcmp(prop,"saveAsSOMLib")==0) {
    if (strcmp(val,"true")==0) {
      Globals::SAVE_AS_SOMLIB = true;
    } else {
      Globals::SAVE_AS_SOMLIB = false;
    }
  } else if (strcmp(prop,"INITIAL_X_SIZE")==0) {
    Globals::INITIAL_X_SIZE = atoi(val);
  } else if (strcmp(prop,"INITIAL_Y_SIZE")==0) {
    Globals::INITIAL_Y_SIZE = atoi(val);
  } else if (strcmp(prop,"LABELS_NUM")==0) {
    Globals::LABELS_NUM = atoi(val);
  } else if (strcmp(prop,"LABELS_ONLY")==0) {
    if (strcmp(val,"true")==0) {
      Globals::LABELS_ONLY = true;
    } else {
      Globals::LABELS_ONLY = false;
    }
  } else if (strcmp(prop,"LABELS_THRESHOLD")==0) {
    Globals::LABELS_THRESHOLD = atof(val);
  } else if (strcmp(prop,"ORIENTATION")==0) {
    if (strcmp(val,"true")==0) {
      Globals::ORIENTATION = true;
    } else {
      Globals::ORIENTATION = false;
    }	 
  } else if (strcmp(prop,"MQE0_FILE")==0) {
    Globals::MQE0_FILE=(char*)malloc(strlen(val)+1);
    strcpy(Globals::MQE0_FILE,val);
  } else if (strcmp(prop,"NR")==0) {
    Globals::NR = atof(val);
  }
}

void getProps(char *propFileName){
  std::ifstream propFile(propFileName);
	
	if (!propFile) {
		std::cout  << "propertyfile " << propFileName << " not found" << std::endl;
		exit(1);
	}
	//const char *delim = "=";
	char prop[150],val[150];
	while (!propFile.eof()) {
		propFile.getline(prop,150,'=');
		propFile.getline(val,150,'\n');
		if ((prop!=NULL) && (val!=NULL)) {
			setProp(prop,val);
			std::cout << prop << " = " << val << std::endl;
		}
	}
}

void startIt() {
  //std::cout << "XXX  startIt: Globals:inputFile = " << Globals::inputFile << std::endl;
  //std::cout << "XXX  startIt: Globals:EXPAND_CYCLES = " << Globals::EXPAND_CYCLES << std::endl;
	Globals::dataItems = Globals::currentDataLoader->readDataItems(Globals::inputFile);
	//std::cout << "XXX  startIt: descriptions" << std::endl;
	Globals::currentDataLoader->readVectorDescription(Globals::descriptionFile);
	//std::cout << "XXX  startIt: init" << std::endl;
	Globals::initHFM();
	//std::cout << "XXX  startIt: train" << std::endl;
	Globals::trainHFM();
	std::cout << "\a";
	if (Globals::SAVE_AS_HTML) {
		Globals::saveHFMAs(HTML);
	}
	if(Globals::SAVE_AS_SOMLIB) {
		Globals::saveHFMAs(SOMLIB);
	}
	std::cout << "\a";
}

int main(int argc, char *argv[])
{
	if (argc!=2) {
		usage();
		return EXIT_FAILURE;
	} else if (strcmp(argv[1],"-v")==0) {
    std::cout << "ghsom: Version " << VERSION << std::endl;
	} else {
  	getProps(argv[1]);
		startIt();
	}

  return EXIT_SUCCESS;
}
