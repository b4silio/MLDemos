#include "atom.h"


JACAtomsXYZR::JACAtomsXYZR(const float *xyzr,
                           unsigned int count,
                           const unsigned int *selected,
                           unsigned int sel_mask)
    : _xyzr(xyzr),
      _count(count),
      _selected(selected),
      _sel_mask(sel_mask)
{
}


unsigned int JACAtomsXYZR::GetCount() const
{
    return _count;
}

bool JACAtomsXYZR::IsSelected(unsigned int i) const
{
    return !_selected || (_selected[i] & _sel_mask);
}

//const float *JACAtomsXYZR::GetCoord(unsigned int i) const
//{
//  return &_xyzr[i*4];
//}

const float*JACAtomsXYZR::GetCoord(unsigned int i) const
{
    return &_xyzr[i*4];
}

float JACAtomsXYZR::GetRadius(unsigned int i) const
{
    return _xyzr[i*4+3];
}


JACAtomsXYZROTE::JACAtomsXYZROTE(const float *xyzr,
                                 unsigned int count,
                                 const float *occupancy,
                                 const float *bfactor,
                                 const unsigned short *element,
                                 const unsigned int *selected,
                                 unsigned int sel_mask)
    : JACAtomsXYZR(xyzr, count, selected, sel_mask),
      _occupancy(occupancy),
      _bfactor(bfactor),
      _element(element)
{
}

float JACAtomsXYZROTE::GetOccupancy(unsigned int i) const
{
    return _occupancy[i];
}

float JACAtomsXYZROTE::GetBFactor(unsigned int i) const
{
    return _bfactor[i];
}

unsigned short JACAtomsXYZROTE::GetElement(unsigned int i) const
{
    return _element[i];
}

JACAtomsMerge::JACAtomsMerge(const JACAtomsBase &atoms1,
                             const JACAtomsBase &atoms2)
    : _atoms1(atoms1),
      _atoms2(atoms2)
{
}

unsigned int JACAtomsMerge::GetCount() const
{
    return _atoms1.GetCount() + _atoms2.GetCount();
}

bool JACAtomsMerge::IsSelected(unsigned int i) const
{
    if (i<_atoms1.GetCount())
        return _atoms1.IsSelected(i);
    return _atoms2.IsSelected(i);
}

//const float *JACAtomsMerge::GetCoord(unsigned int i) const
//{
//  if (i<_atoms1.GetCount())
//    return _atoms1.GetCoord(i);
//  return _atoms2.GetCoord(i);
//}

const float*JACAtomsMerge::GetCoord(unsigned int i) const
{
    if (i<_atoms1.GetCount())
        return _atoms1.GetCoord(i);
    return _atoms2.GetCoord(i);
}

float JACAtomsMerge::GetRadius(unsigned int i) const
{
    if (i<_atoms1.GetCount())
        return _atoms1.GetRadius(i);
    return _atoms2.GetRadius(i);
}

float JACAtomsMerge::GetOccupancy(unsigned int i) const
{
    if (i<_atoms1.GetCount())
        return _atoms1.GetOccupancy(i);
    return _atoms2.GetOccupancy(i);
}

float JACAtomsMerge::GetBFactor(unsigned int i) const
{
    if (i<_atoms1.GetCount())
        return _atoms1.GetBFactor(i);
    return _atoms2.GetBFactor(i);
}

unsigned short JACAtomsMerge::GetElement(unsigned int i) const
{
    if (i<_atoms1.GetCount())
        return _atoms1.GetElement(i);
    return _atoms2.GetElement(i);
}

