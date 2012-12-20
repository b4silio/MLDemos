#ifndef JAC_PLANE_H
#define JAC_PLANE_H

class gridT;
class VertexSender;

class JACPlane
{
public:
    JACPlane();
    JACPlane(unsigned int xdim, unsigned int ydim);
    ~JACPlane();

    void SetParams(float unit,
                   float *org,
                   float *xprime, float *yprime, float *zprime);


    void LoadFromGrid(const gridT &grid);
    void Contour(float threshold, VertexSender &vs);

    void ContourGrid(const gridT &grid, char axis,
                     float threshold, VertexSender &vs);


    void Transform(float *v, float igrid, float jgrid) const;

    unsigned int GetXdim() const
    {
        return xdim;
    }
    unsigned int GetYdim() const
    {
        return ydim;
    }

private:
    unsigned int xdim;
    unsigned int ydim;
    float org[3];      /* origin of the plane */
    float unit;        /* length of the size of a square */
    float xprime[3];   /* these form an orthonormal basis vector set */
    float yprime[3];   /* oriented along the screen */
    float zprime[3];
    float *data;       /* of course, the data */
};

#endif // ifndef JAC_PLANE_H
