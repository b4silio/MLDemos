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

void MLDemos::ManualSelectionUpdated()
{
    if (!canvas) return;
    // we add the samples
    vector<fvec> samples = canvas->data->GetSamples();
    int dim = samples.size() ? samples[0].size() : 0;
    ivec labels = canvas->data->GetLabels();
    manualSelection->sampleList->clear();
    FOR (i, samples.size())
    {
        QString item = QString("%1: (%2)").arg(i+1).arg(labels[i]);
        FOR (d, dim) item += QString(" %1").arg(samples[i][d], 0, 'f', 2);
        manualSelection->sampleList->addItem(item);
    }
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionChanged()
{
    int count = manualSelection->sampleList->count();
    int trainCount = count, testCount = 0;
    QList<QListWidgetItem*> selected = manualSelection->sampleList->selectedItems();
    if (selected.size())
    {
        trainCount = selected.size();
        testCount = count - trainCount;
    }
    manualSelection->TrainLabel->setText(QString("Train: %1").arg(trainCount));
    manualSelection->TestLabel->setText(QString("Test: %1").arg(testCount));
}

void MLDemos::ManualSelectionClear()
{
    manualSelection->sampleList->clearSelection();
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionInvert()
{
    FOR (i, manualSelection->sampleList->count())
    {
        manualSelection->sampleList->item(i)->setSelected(!manualSelection->sampleList->item(i)->isSelected());
    }
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionRandom()
{
    float ratio = (manualSelection->randomCombo->currentIndex()+1.f)/10.f;
    manualSelection->sampleList->clearSelection();
    u32* perm = randPerm(manualSelection->sampleList->count());
    FOR (i, ratio*manualSelection->sampleList->count())
    {
        manualSelection->sampleList->item(perm[i])->setSelected(true);
    }
    KILL(perm);
    ManualSelectionChanged();
}

void MLDemos::ManualSelectionRemove()
{
    if (!canvas || !canvas->data->GetCount()) return;
    QList<QListWidgetItem*> selected = manualSelection->sampleList->selectedItems();
    if (!selected.size()) return;
    ivec removeList(selected.count());
    FOR (i, selected.count())
    {
        removeList[i] = manualSelection->sampleList->row(selected[i]);
    }
    canvas->data->RemoveSamples(removeList);
    if (canvas->sampleColors.size() && removeList.size() < canvas->sampleColors.size())
    {
        int offset = 0;
        FOR (i, removeList.size())
        {
            int index = i - offset;
            if (index < 0 || index >= canvas->sampleColors.size()) continue;
            canvas->sampleColors.erase(canvas->sampleColors.begin()+index);
            offset++;
        }
    }
    ManualSelectionUpdated();
    ManualSelectionChanged();
    canvas->ResetSamples();
    CanvasOptionsChanged();
    canvas->repaint();
}

void MLDemos::InputDimensionsUpdated()
{
    if (!canvas) return;
    int dim = canvas->data->GetDimCount();
    inputDimensions->dimList->clear();
    fvec xMin(dim,FLT_MAX), xMax(dim,-FLT_MAX);
    vector<fvec> samples = canvas->data->GetSamples();
    FOR (i, samples.size())
    {
        FOR (d, dim)
        {
            xMin[d] = min(xMin[d], samples[i][d]);
            xMax[d] = max(xMax[d], samples[i][d]);
        }
    }
    FOR (d, dim)
    {
        QString item = QString("%1").arg(d+1);
        if (d < canvas->dimNames.size()) item += QString(") %2").arg(canvas->dimNames.at(d));
        item += QString(" : [%1 --> %2]").arg(xMin[d], 0, 'f', 3).arg(xMax[d], 0, 'f', 3);
        inputDimensions->dimList->addItem(item);
    }
    ManualSelectionChanged();
    vis->UpdateDims();
}

void MLDemos::InputDimensionsChanged()
{
    int count = inputDimensions->dimList->count();
    QList<QListWidgetItem*> selected = inputDimensions->dimList->selectedItems();
    inputDimensions->TrainLabel->setText(QString("Used: %1").arg(selected.size()));
    inputDimensions->TestLabel->setText(QString("Unused: %1").arg(count-selected.size()));
}

void MLDemos::InputDimensionsClear()
{
    inputDimensions->dimList->clearSelection();
    ManualSelectionChanged();
}

void MLDemos::InputDimensionsInvert()
{
    FOR (i, inputDimensions->dimList->count())
    {
        inputDimensions->dimList->item(i)->setSelected(!inputDimensions->dimList->item(i)->isSelected());
    }
    ManualSelectionChanged();
}

void MLDemos::InputDimensionsRandom()
{
    float ratio = (inputDimensions->randomCombo->currentIndex()+1.f)/10.f;
    inputDimensions->dimList->clearSelection();
    u32* perm = randPerm(inputDimensions->dimList->count());
    FOR (i, ratio*inputDimensions->dimList->count())
    {
        inputDimensions->dimList->item(perm[i])->setSelected(true);
    }
    KILL(perm);
    ManualSelectionChanged();
}
