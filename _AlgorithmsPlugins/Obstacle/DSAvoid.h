//testing svn

#ifndef _DSAVOID_H_
#define _DSAVOID_H_

#include "MathLib/MathLib.h"
#include <obstacles.h>

#include <string>

using namespace MathLib;

#define MAX_LOG_TIME_SEC        120
#define CYCLES_PER_SEC          500
#define MAX_LOG_CYCLES          (MAX_LOG_TIME_SEC * CYCLES_PER_SEC)
static int num_obs_max = 5; //maximum number of obstacles
//#define PI						3.141592653589793

//stuffs for the obstacle avoidance
struct DSObstacle{
	Vector			axes;		//the obstacle major axes
	Vector			center;		//the center of the obstacle
	Matrix			Rotation;		//the orientation matrix
	Vector			power;		//Gamma is \sum( (x/a)^m )
	Vector			safetyFactor;		//safety factor
	Matrix			E;              //matrix of the basis vector
	Matrix			M;              //dynamic modulation matrix (dim x dim)
	Vector			e_amp;          //defining the amplitude of each basis vector (to avoid saddle/local minimum)
	//Vector			origin_shift;   //transforming the real center of the obstacle
	DSObstacle(int dim=2);
	DSObstacle(Obstacle obstacle);
	void Print();
};

class DSAvoid : public ObstacleAvoidance
{
public:
	DSAvoid();
	~DSAvoid();
	void Clear();
	void init(int num_obs);
	fvec Avoid(fvec &x, fvec &xdot);
	fVec Avoid(fVec &x, fVec &xdot);
	void SetObstacles(std::vector< Obstacle > obstacles);

protected:
	bool Avoid(Vector &x,Vector &xd);
	int dim;
	DSObstacle*		obs; //to model the obstacle
	Vector			x_t,d,nv,nv_rotated,e; //nv is the normal vector
	Matrix			D;
	bool			b_obstacle;  //check if the obstacle module is activated
	bool			b_contouring; //shall we contour the obstacle
	Vector			xd_contouring;
	Vector			xd_old,vec_tmp;
	int				i_end;
	Vector			Gamma;
	int				num_obs; //the number of obstacles
	IndicesVector	ind; //a vector keeping the priority of obstacles after sorting them based on Gamma
	int				c_obs; //current obstacle number (used for changing the obstacle properties in DS_Command)
	string			Joint_Obstacles_File; //the file name that includes the properties of obstacles in the joint space
	Matrix			R_transpose;
	Matrix			mat_tmp0,mat_tmp1,mat_tmp2,mat_tmp3,mat_tmp4;
};

#endif // _DSAVOID_H_
