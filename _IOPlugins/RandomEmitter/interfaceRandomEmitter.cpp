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
#include "interfaceRandomEmitter.h"
#include <QDebug>
using namespace std;

RandomEmitter::RandomEmitter()
: ThreadID(0)
{
}

RandomEmitter::~RandomEmitter()
{
	Stop();
}

void RandomEmitter::Start()
{
	qDebug() << "Starting threadID: " << ThreadID;
	if(ThreadID) return; // we're already running
	ThreadID = startTimer(100);
}

void RandomEmitter::Stop()
{
	qDebug() << "Stopping threadID: " << ThreadID;
	if(!ThreadID) return; // we're not running anymore
	killTimer(ThreadID);
	ThreadID = 0;
}

void RandomEmitter::timerEvent(QTimerEvent *event)
{
	// we generate a bunch of samples
	vector<fvec> samples;
	ivec labels;
	int count = 200;
	samples.resize(count*2);
	labels.resize(count*2);

	fvec sample;
	sample.resize(2);
	FOR(i, count)
	{
		sample[0] = (rand() / (float)RAND_MAX) * 0.7;
		sample[1] = (rand() / (float)RAND_MAX) * 0.7;
		samples[i] = sample;
		labels[i] = 0;
	}
	FOR(i, count)
	{
		sample[0] = (rand() / (float)RAND_MAX) * 0.7 + 0.3;
		sample[1] = (rand() / (float)RAND_MAX) * 0.7 + 0.3;
		samples[count + i] = sample;
		labels[count + i] = 1;
	}
	emit(SetData(samples, labels, vector<ipair>()));
}

void RandomEmitter::FetchResults(std::vector<fvec> results)
{

}

Q_EXPORT_PLUGIN2(IO_RandomEmitter, RandomEmitter)
