#ifndef JACATOMS_H
#define JACATOMS_H

class JACAtomsBase
{
public:
    virtual ~JACAtomsBase()
    {
    }
    virtual unsigned int GetCount() const = 0;
    virtual bool IsSelected(unsigned int i) const = 0;
    virtual const float *GetCoord(unsigned int i) const = 0;
    virtual float GetRadius(unsigned int i) const = 0;

    virtual float GetOccupancy(unsigned int i) const = 0;
    virtual float GetBFactor(unsigned int i) const = 0;
    virtual unsigned short GetElement(unsigned int i) const = 0;
};

// simple adaptor to adapt xyzr data to JACAtomsBase API
class JACAtomsXYZR : public JACAtomsBase
{
public:
    JACAtomsXYZR(const float *xyzr,
                 unsigned int count,
                 const unsigned int *selected = 0,
                 unsigned int sel_mask = 1);

    unsigned int GetCount() const;
    bool IsSelected(unsigned int i) const;
    const float *GetCoord(unsigned int i) const;
    float GetRadius(unsigned int i) const;


    float GetOccupancy(unsigned int) const
    {
        return 0.0f;
    }
    float GetBFactor(unsigned int) const
    {
        return 0.0f;
    }
    unsigned short GetElement(unsigned int) const
    {
        return 0;
    }

protected:
    const float *_xyzr;
    unsigned int _count;
    const unsigned int *_selected;
    unsigned int _sel_mask;
};

// expands above to also support occupancy, b-factor and element number
class JACAtomsXYZROTE : public JACAtomsXYZR
{
public:
    JACAtomsXYZROTE(const float *xyzr,
                    unsigned int count,
                    const float *occupancy,
                    const float *bfactor,
                    const unsigned short *element,
                    const unsigned int *selected = 0,
                    unsigned int sel_mask = 1);

    float GetOccupancy(unsigned int i) const;
    float GetBFactor(unsigned int i) const;
    unsigned short GetElement(unsigned int i) const;

private:
    const float *_occupancy;
    const float *_bfactor;
    const unsigned short *_element;
};


//adaptor to take two sets of atoms and treat as a single set
class JACAtomsMerge : public JACAtomsBase
{
public:
    JACAtomsMerge(const JACAtomsBase &atoms1,
                  const JACAtomsBase &atoms2);

    unsigned int GetCount() const;
    bool IsSelected(unsigned int i) const;
    const float *GetCoord(unsigned int i) const;
    float GetRadius(unsigned int i) const;

    float GetOccupancy(unsigned int i) const;
    float GetBFactor(unsigned int i) const;
    unsigned short GetElement(unsigned int i) const;


private:
    const JACAtomsBase &_atoms1;
    const JACAtomsBase &_atoms2;
};
#endif // ifndef JACATOMS_H
