#include "reinforcementProblem.h"
#include "drawUtils.h"

ReinforcementProblem::ReinforcementProblem()
    : dim(2), data(NULL), w(1), h(1),
      policyType(0), gridSize(4), quantizeType(0), directions(fvec()), tempDirections(fvec()),
      simulationSteps(10), problemType(0), displayIterationsCount(1), rewardType(0)
{

}

ReinforcementProblem::~ReinforcementProblem()
{
    if(data) delete [] data;
}

void ReinforcementProblem::Initialize(float *dataMap, fVec size, fvec startingPoint)
{
    if(problemType == 0)
    {
        w = gridSize;
        h = gridSize;
        if(data) delete [] data;
        data = new float[w*h];
        FOR(x, w)
        {
            FOR(y, h)
            {
                int xIndex = x*size.x/w;
                int yIndex = y*size.y/h;
                int xIndexStop = (x+1)*size.x/w;
                int yIndexStop = (y+1)*size.y/h;
                float value = 0;
                int cnt = 0;
                for(int X=xIndex; X<xIndexStop; X++)
                {
                    for(int Y=yIndex; Y<yIndexStop; Y++)
                    {
                        value += dataMap[(int)(Y*size.x) + X];
                        cnt++;
                    }
                }
                data[y*w + x] = value/cnt;
            }
        }
    }
    else
    {
        w = size.x;
        h = size.y;
        if(data) delete [] data;
        data = new float[w*h];
        memcpy(data, dataMap, w*h*sizeof(float));
    }
    directions.resize(gridSize*gridSize);

    if(quantizeType) FOR(d, gridSize*gridSize) directions[d] = rand()%(quantizeType==1?9:5);
    else FOR(d, gridSize*gridSize) directions[d] = drand48()*2*M_PI;

    if(startingPoint.size()) directions = startingPoint;
}

float ReinforcementProblem::GetValue(fvec sample)
{
    int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
    int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
    int index = yIndex*w + xIndex;
    return data[index];
}
void ReinforcementProblem::SetValue(fvec sample, float value)
{
    int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
    int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
    int index = yIndex*w + xIndex;
    data[index] = value;
}

float ReinforcementProblem::GetSimulationValue(fvec sample)
{
    float reward = 0;
    switch(problemType)
    {
    case 0:
    {
        switch(rewardType)
        {
        case 0: // Sum of Rewards
        {
            reward += GetValue(sample);
            FOR(i, simulationSteps)
            {
                fvec newSample = PerformAction(sample);
                if(newSample == sample) break;
                reward += GetValue(newSample);
                sample = newSample;
            }
        }
            break;
        case 1: // Sum - Harsh Turns (> 90 deg)
        {
            fvec direction(dim);
            reward += GetValue(sample);
            FOR(i, simulationSteps)
            {
                fvec newSample = PerformAction(sample);
                float currentReward = GetValue(newSample);
                fvec newDirection = newSample - sample;
                if(newSample == sample) break;
                if(i && direction*newDirection < 0.f) currentReward = 0;
                reward += currentReward;
                sample = newSample;
                direction = newDirection;
            }
        }
            break;
        case 2: // Average
        {
            fvec direction(dim);
            reward += GetValue(sample);
            int cnt = 0;
            FOR(i, simulationSteps)
            {
                fvec newSample = PerformAction(sample);
                float currentReward = GetValue(newSample);
                if(newSample == sample) break;
                reward += currentReward;
                sample = newSample;
                cnt++;
            }
            reward /= cnt;
        }
            break;
        case 3: // depleting reward
        {
            // we backup the data
            float *dataCopy = new float[w*h];
            memcpy(dataCopy, data, w*h*sizeof(float));

            int xIndex = max(0, min(w-1, (int)(sample[0]*w)));
            int yIndex = max(0, min(h-1, (int)(sample[1]*h)));
            int index = yIndex*w + xIndex;
            reward += data[index];
            data[index] = 0;
            FOR(i, simulationSteps)
            {
                fvec newSample = PerformAction(sample);
                if(newSample == sample)break;
                xIndex = max(0, min(w-1, (int)(newSample[0]*w)));
                yIndex = max(0, min(h-1, (int)(newSample[1]*h)));
                index = yIndex*w + xIndex;
                reward += data[index];
                data[index] = 0;
                sample = newSample;
            }
            // we swap the backup
            delete [] data;
            data = dataCopy;
        }
            break;
        }

    }
        break;
    }
    return reward;
}

fvec ReinforcementProblem::NextStep(fvec sample, fvec directions)
{
    int index = 0;
    int mult = 0;
    int xIndex = sample[0]*gridSize;
    int yIndex = sample[1]*gridSize;
    // the policy direction at state (xIndex,yIndex)
    float direction = directions[yIndex*gridSize + xIndex]; // direction, in radians
    // we move one case in the direction determined by the policy
    switch(quantizeType)
    {
        case 0:
        {
            if(direction != -1)
            {
                sample[0] += cosf(direction)*(0.5f/gridSize);
                sample[1] += sinf(direction)*(0.5f/gridSize);
            }
        }
            break;
        case 1:
        {
            switch((int)direction)
            {
            case 0:
                sample[0] += 1./gridSize;
            break;
            case 1:
                sample[0] += 1./gridSize;
                sample[1] += 1./gridSize;
            break;
            case 2:
                sample[1] += 1./gridSize;
            break;
            case 3:
                sample[0] -= 1./gridSize;
                sample[1] += 1./gridSize;
            break;
            case 4:
                sample[0] -= 1./gridSize;
            break;
            case 5:
                sample[0] -= 1./gridSize;
                sample[1] -= 1./gridSize;
            break;
            case 6:
                sample[1] -= 1./gridSize;
            break;
            case 7:
                sample[0] += 1./gridSize;
                sample[1] -= 1./gridSize;
            break;
            case 8:
                break;
            }
        }
            break;
        case 2:
        {
            switch((int)direction)
            {
            case 0:
                sample[0] += 1./gridSize;
            break;
            case 1:
                sample[1] += 1./gridSize;
            break;
            case 2:
                sample[0] -= 1./gridSize;
            break;
            case 3:
                sample[1] -= 1./gridSize;
            break;
            case 4:
                break;
            }
        }
            break;
    }

    // we bound the space to a 0-1 range
    sample[0] = min(1.f, max(0.f, sample[0]));
    sample[1] = min(1.f, max(0.f, sample[1]));
    return sample;
}

fvec ReinforcementProblem::PerformAction(fvec sample)
{
    return NextStep(sample, directions);
}

float ReinforcementProblem::GetReward()
{
    return GetReward(directions);
}

float ReinforcementProblem::GetReward(fvec directions)
{
    fvec backup = this->directions;
    this->directions = directions;
    fvec sample(dim);
    float fullReward = 0;
    FOR(i, gridSize*gridSize)
    {
        sample[0] = (i%gridSize + 0.5f)/(float)gridSize;
        sample[1] = (i/gridSize + 0.5f)/(float)gridSize;
        fullReward += GetSimulationValue(sample);
    }
    fullReward /= (gridSize*gridSize);
    this->directions = backup;
    return fullReward;
}

void ReinforcementProblem::Draw(QPainter &painter)
{
    int w = painter.viewport().width(), h = painter.viewport().height();
    int xSize = w / gridSize, ySize = h / gridSize;
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    switch(policyType)
    {
    case 0:
    {
        // we draw a grid of the different 'cases'
        FOR(i, gridSize*gridSize)
        {
            int xIndex = i%gridSize;
            int yIndex = i/gridSize;
            int x = xIndex*w/gridSize, y = yIndex*h/gridSize;
            painter.setPen(QPen(Qt::black, 0.5));
            painter.drawRect(x+xSize*0.05, y+ySize*0.05, xSize*.9, ySize*.9);
            QPointF point1, point2;
            float len1 = 1.f, len2 = 1.f;
            if(quantizeType)
            {
                fvec s(2,0);
                s[0] = xIndex/(float)gridSize;
                s[1] = yIndex/(float)gridSize;
                fvec s2 = NextStep(s, directions);
                point1 = QPointF(s2[0]-s[0], s2[1]-s[1]);
                len1 = sqrtf(point1.x()*point1.x()+point1.y()*point1.y());
                point1 = QPointF(point1.x()/len1, point1.y()/len1);
                s2 = NextStep(s, tempDirections);
                point2 = QPointF(s2[0]-s[0], s2[1]-s[1]);
                len2 = sqrtf(point2.x()*point2.x()+point2.y()*point2.y());
                point2 = QPointF(point2.x()/len2, point2.y()/len2);
            }
            else
            {
                point1 = QPointF(cosf(directions[i]), sinf(directions[i]));
                point2 = QPointF(cosf(tempDirections[i]), sinf(tempDirections[i]));
            }
            painter.setPen(QPen(Qt::gray, 1));
            int radius = min(xSize, ySize)/6;
            if(len1 == 0) painter.drawEllipse(QPointF(x+xSize/2, y+ySize/2), radius, radius);
            else DrawArrow(QPointF(x+xSize/2 - point1.x()*xSize/5, y+ySize/2 - point1.y()*ySize/5),
                          QPointF(x+xSize/2 + point1.x()*xSize/5, y+ySize/2 + point1.y()*ySize/5), xSize/10., painter);
            painter.setPen(QPen(Qt::black, 1));
            if(len2 == 0) painter.drawEllipse(QPointF(x+xSize/2, y+ySize/2), radius, radius);
            else DrawArrow(QPointF(x+xSize/2 - point2.x()*xSize/5, y+ySize/2 - point2.y()*ySize/5),
                      QPointF(x+xSize/2 + point2.x()*xSize/5, y+ySize/2 + point2.y()*ySize/5), xSize/10., painter);
        }
    }
        break;
    }
}

