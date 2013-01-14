#include "datagenerator.h"
#include "ui_datagenerator.h"
#include <QDebug>

using namespace std;

DataGenerator::DataGenerator(Canvas *canvas, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataGenerator)
{
    ui->setupUi(this);
    this->canvas = canvas;
    //connect(ui->addButton, SIGNAL(clicked()), this, SLOT(Generate()));
    connect(ui->generatorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OptionsChanged()));
    OptionsChanged();
}

DataGenerator::~DataGenerator()
{
    delete ui;
}

void DataGenerator::OptionsChanged()
{
    int type = ui->generatorCombo->currentIndex();
    ui->gridCountLabel->setText("Grid Count");
    ui->radiusLabel->setText("Radius");
    ui->classesLabel->setText("Class");
    ui->dimLabel->setText("Dim");
    ui->classesCount->setEnabled(true);
    ui->dimSpin->setEnabled(false);
    switch(type)
    {
    case 0: // checkerboard
        ui->radiusLabel->setText("Size");
        ui->dimSpin->setEnabled(true);
        break;
    case 1: // concentric circles
        ui->gridCountLabel->setText("Circles");
        ui->radiusLabel->setText("Radius");
        break;
    case 2: // spirals
        ui->gridCountLabel->setText("Swirls");
        ui->radiusLabel->setText("Radius");
        break;
    case 3: // sinc
        ui->gridCountLabel->setText("Noise");
        ui->radiusLabel->setText("Width");
        ui->classesCount->setEnabled(false);
        break;
    case 4: // gaussian
        ui->gridCountLabel->setText("Noise");
        ui->radiusLabel->setText("Width");
        ui->classesCount->setEnabled(false);
        break;
    case 5: // cosine
        ui->gridCountLabel->setText("Noise");
        ui->radiusLabel->setText("Width");
        ui->classesCount->setEnabled(false);
    case 6: // swiss rolls
        ui->gridCountLabel->setText("Noise");
        break;
    }
}

void GenerateRecursive(vector<fvec> &samples, ivec &labels, fvec &sample, int &label, fvec &starts,
                       int d, int dim, float radius, int gridCount, int classesCount, int samplesPerCell)
{
    if(d==0)
    {
        FOR(x, gridCount)
        {
            starts[d] = x*radius;
            FOR(i, samplesPerCell)
            {
                FOR(d, dim) sample[d] = (drand48()*radius + starts[d]) / gridCount - radius*0.5f;
                samples.push_back(sample);
                labels.push_back(label);
            }
            label = (label+1)%classesCount;
        }
        if(!(gridCount%classesCount)) label = (label+1)%classesCount;
        return;
    }
    else
    {
        FOR(x, gridCount)
        {
            starts[d] = x*radius;
            GenerateRecursive(samples, labels, sample, label, starts, d-1, dim, radius, gridCount, classesCount, samplesPerCell);
        }
        if(!(gridCount%classesCount)) label = (label+1)%classesCount;
    }
}

pair<vector<fvec>, ivec> DataGenerator::Generate()
{
    int count = ui->countSpin->value();
    int dim = ui->dimSpin->value();
    int gridCount = ui->gridCountSpin->value();
    int classesCount = ui->classesCount->value();
    float radius = ui->radiusSpin->value();
    int type = ui->generatorCombo->currentIndex();
    vector<fvec> samples;
    ivec labels;
    fvec sample(dim);
    switch(type)
    {
    case 0: // checkerboard
    {
        int samplesPerCell = count/(gridCount*gridCount);
        int label = 0;

        fvec starts(dim);
        GenerateRecursive(samples, labels, sample, label, starts,dim-1, dim, radius, gridCount, classesCount, samplesPerCell );

        /*
        float xStart=0, xStop=0, yStart=0, yStop=0;
        FOR(y, gridCount)
        {
            yStart = y*radius;
            yStop = yStart + radius;
            FOR(x, gridCount)
            {
                xStart = x*radius;
                xStop = xStart + radius;
                FOR(i, samplesPerCell)
                {
                    sample[0] = ((drand48()-0.5)*(xStop-xStart) + xStart) / gridCount;
                    sample[1] = ((drand48()-0.5)*(yStop-yStart) + yStart) / gridCount;
                    samples.push_back(sample);
                    labels.push_back(label);
                }
                label = (label+1)%classesCount;
            }
            if(!(gridCount%classesCount)) label = (label+1)%classesCount;
        }
        */
    }
        break;
    case 1: // concentric circles / sphere
    {
        int samplesPerCircle = count/(gridCount*classesCount);
        int cnt = 0;
        if(dim == 2)
        {
            FOR(i, gridCount)
            {
                FOR(c, classesCount)
                {
                    float radStart = radius*cnt/(float)(gridCount*classesCount);
                    float radStop = radius*(cnt+1)/(float)(gridCount*classesCount);
                    FOR(j, samplesPerCircle)
                    {
                        float angle = drand48()*2*M_PI;
                        float rad = drand48()*(radStop-radStart) + radStart;
                        sample[0] = cos(angle)*rad;
                        sample[1] = sin(angle)*rad;
                        samples.push_back(sample);
                        labels.push_back(c);
                    }
                    cnt++;
                }
            }
        }
        else if(dim == 3)
        {
            FOR(i, gridCount)
            {
                FOR(c, classesCount)
                {
                    float radStart = radius*cnt/(float)(gridCount*classesCount);
                    float radStop = radius*(cnt+1)/(float)(gridCount*classesCount);
                    FOR(j, samplesPerCircle)
                    {
                        float phi = drand48()*2*M_PI;
                        float theta = drand48()*M_PI;
                        float rad = drand48()*(radStop-radStart) + radStart;
                        sample[0] = sin(theta)*cos(phi)*rad;
                        sample[1] = sin(theta)*sin(phi)*rad;
                        sample[2] = cos(theta)*rad;
                        samples.push_back(sample);
                        labels.push_back(c);
                    }
                    cnt++;
                }
            }
        }
    }
        break;
    case 2: // spirals
    {
        dim = 2;
        sample.resize(dim);
        int samplesPerClass = count / classesCount;
        FOR(c, classesCount)
        {
            FOR(i, samplesPerClass)
            {
                float swirls = M_PI*2*gridCount;
                float x = (i+1)/(float)samplesPerClass*(swirls*swirls);
                x = sqrtf(x);
                sample[0] = cosf(x + M_PI*2/classesCount*c)*radius*x/swirls;
                sample[1] = sinf(x + M_PI*2/classesCount*c)*radius*x/swirls;
                samples.push_back(sample);
                labels.push_back(c);
            }
        }
    }
        break;
    case 3: // sinc
    {
        dim = 2;
        sample.resize(dim);
        FOR(i, count)
        {
            float x = (i/(float)count*2 - 1)*radius*2*M_PI;
            float y = sinf(M_PI*x) / (x*M_PI);
            if(gridCount > 1) y += drand48()*((gridCount-1)/(float)32);
            sample[0] = x;
            sample[1] = y;
            samples.push_back(sample);
            labels.push_back(0);
        }
    }
        break;
    case 4: // gaussian
    {
        dim = 2;
        sample.resize(dim);
        FOR(i, count)
        {
            float x = (i/(float)count*2 - 1)*10*radius;
            float y = exp(-0.5f*x*x);
            if(gridCount > 1) y += drand48()*((gridCount-1)/(float)32);
            sample[0] = x;
            sample[1] = y;
            samples.push_back(sample);
            labels.push_back(0);
        }
    }
        break;
    case 5: // cosine
    {
        dim = 2;
        sample.resize(dim);
        FOR(i, count)
        {
            float x = (i/(float)count*2 - 1)*radius*2*M_PI;
            float y = cos(x);
            if(gridCount > 1) y += drand48()*((gridCount-1)/(float)32);
            sample[0] = x;
            sample[1] = y;
            samples.push_back(sample);
            labels.push_back(0);
        }
    }
        break;
    case 6: // swiss roll
    {
        dim = 3;
        sample.resize(dim);
        FOR(i, count)
        {
            float t = 1.5f * M_PI * (1.f + 2.f * drand48());
            float x = t * cos(t);
            float y = t * sin(t);
            float z = 21.f * (drand48()-0.5f);
            x = x / 21.f * radius;
            y = y / 21.f * radius;
            z = z / 21.f * radius;

            sample[0] = x + drand48()*((gridCount-1)/32.f);
            sample[1] = y + drand48()*((gridCount-1)/32.f);
            sample[2] = z + drand48()*((gridCount-1)/32.f);
            samples.push_back(sample);
            labels.push_back(0);
        }
    }
        break;
    }
    return make_pair(samples, labels);
}
