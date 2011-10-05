/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Evolution-Strategy Mixture of Logisitics Regression
Copyright (C) 2011  Stephane Magnenat
Contact: stephane at magnenat dot net

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

#include "classifierESMLR.h"

ClassifierESMLR::ClassifierESMLR():
	cutCount(2),
	alpha(20)
{
}

ClassifierESMLR::~ClassifierESMLR()
{
}

void ClassifierESMLR::Train(std::vector< fvec > samples, ivec labels)
{
}

float ClassifierESMLR::Test(const fvec &sample)
{
	return 0;
}

char *ClassifierESMLR::GetInfoString()
{
	char *text = new char[1024];
	sprintf(text, "Evolution Strategy, Mixture of Logistic Regressions\n"
	"hyperplane count: %d"
	"alpha:            %f",
	cutCount, alpha
	);
	return text;
}

void ClassifierESMLR::SetParams(u32 cutCount, float alpha)
{
}
