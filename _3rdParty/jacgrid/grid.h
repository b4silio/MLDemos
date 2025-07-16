#ifndef JAC_GRID_H
#define JAC_GRID_H

#include <string>
#include <vector>

//start
class gridT
{
public:
    unsigned int npts[3]; /* number of divisions in each direction -- the dimension */
    float unit[3];        /* length of a single edge in each dimension*/
    float size[3];        /* length of entire grid in each dimension */
    float org[3];         /* the origin of the grid i.e. coords of (0,0,0) */
    float center[3];      /* coords of center of grid */
    float *data;          /* the grid data values */

    // it's questionable as to if these even belong in gridT
    unsigned int *owner; /* which atom is closest to grid point */
    unsigned int *type;  /* 0, CONTACT_SURFACE or REENTRANT_SURFACE */

    //endvars
    gridT();
    gridT(float InitVal,
          unsigned int dimx, unsigned int dimy, unsigned int dimz,
          bool with_surface_fields = false);
    ~gridT();

    bool IsValid() const
    {
        return _valid;
    }
    unsigned int GetSize() const
    {
        return _size;
    }

    float &operator[](unsigned int index);

    void Clear();
    bool Resize(float InitVal,
                unsigned int dimx, unsigned int dimy, unsigned int dimz,
                bool with_surface_fields = false);

private:
    bool _valid;
    unsigned int _size;
};


#endif // ifndef JAC_GRID_H
