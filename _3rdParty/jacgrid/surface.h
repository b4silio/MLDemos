#ifndef JAC_SURFACE_H
#define JAC_SURFACE_H

#include <string>
#include <vector>
#include <set>

//start
class surfaceT
{
public:
    unsigned int nverts;                 /* number of vertices used */
    unsigned int nconn;                  /* number of verts in connection (triangle) list */
    unsigned int maxverts;               /* maximum allocated verts */
    unsigned int maxconn;                /* maximum allocated verts in connection list */
    std::vector<float> vertices;         /* the coords of a vertex (global surf points here) */
    std::vector<float> normals;          /* normal at the vertex */
    std::vector<unsigned int> triangles; /* list of vertices to be connected */
    std::vector<unsigned int> owner;

    float gridUnit[3];                   /* the grid unit sizes used to make the surface */
    unsigned int needs_resort;           /* for transparent surfaces */

    //endvars
    surfaceT();
    surfaceT(unsigned int verts, unsigned int polys);
    bool Resize(unsigned int verts, unsigned int polys);
    void Clear();

    void BuildNeighborList(std::vector<std::set<unsigned int> > &nbrList) const;
    void BuildVertexToTriangleList(std::vector<std::set<unsigned int> > &v2cList) const;
    void Reduce(float mindist, unsigned int *pruned = 0);
};
#endif // ifndef JAC_SURFACE_H
