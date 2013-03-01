#include <public.h>
#include "DSAvoid.h"

DSAvoid::DSAvoid()
	: dim(2), obs(0)
{
}

DSAvoid::~DSAvoid()
{
	KILL(obs);
}

void DSAvoid::Clear()
{
	KILL(obs);
}

void DSAvoid::SetObstacles(std::vector< Obstacle > newObstacles)
{
	if(!obstacles.size() || obstacles.size() != newObstacles.size())
	{
		obstacles = newObstacles;
		KILL(obs);
	}
	else // we want to know if something new was added
	{
		FOR(i, obstacles.size())
		{
			if(obstacles[i] != newObstacles[i])
			{
				obstacles = newObstacles;
				KILL(obs);
				break;
			}
		}
	}

	if(!obs && obstacles.size())
	{
		obs = new DSObstacle[obstacles.size()];
		FOR(i, obstacles.size())
		{
			obs[i] = DSObstacle(obstacles[i]);
			//obs[i].Print();
		}
		num_obs = obstacles.size();

		init(num_obs);
	}
}

fvec DSAvoid::Avoid(fvec &x, fvec &xdot)
{
	if(!obstacles.size()) return xdot;
	dim = 2;
	Vector X(2), XDot(2);
	FOR(d, dim)
	{
		X(d) = x[d];
		XDot(d) = xdot[d];
	}

	// we do the actual avoidance
	Avoid(X,XDot);

	fvec newXDot = xdot;
	FOR(d,dim) newXDot[d] = XDot(d);
	return newXDot;
}

fVec DSAvoid::Avoid(fVec &x, fVec &xdot)
{
	if(!obstacles.size()) return xdot;
	dim = 2;
	Vector X(2), XDot(2);
	FOR(d, dim)
	{
		X(d) = x[d];
		XDot(d) = xdot[d];
	}

	// we do the actual avoidance
	Avoid(X,XDot);

	fVec newXDot = xdot;
	FOR(d,dim) newXDot[d] = XDot(d);
	return newXDot;
}


DSObstacle::DSObstacle(Obstacle o)
{
	int dim = o.axes.size();

	center.Resize(dim);

	Rotation.Resize(dim,dim);
	Rotation.Identity();

	if(dim == 2)
	{
		Rotation(0,0) = cos(o.angle);
		Rotation(0,1) = -sin(o.angle);
		Rotation(1,0) = sin(o.angle);
		Rotation(1,1) = cos(o.angle);
	}

	axes.Resize(dim);
	axes.One();

	power.Resize(dim);
	power.One();

	e_amp.Resize(dim+1);
	e_amp.One();

	safetyFactor.Resize(dim);
	safetyFactor = safetyFactor.One();

	if (dim == 3){
		E.Resize(3,4);
	}else{
		E.Resize(dim,dim);
	}
	E.Identity();

	M.Resize(dim,dim);
	M.Identity();

	power = safetyFactor = center = axes = Vector(dim);
	FOR(d,dim)
	{
		axes(d) = o.axes[d];
		center(d) = o.center[d];
		power(d) = o.power[d];
		safetyFactor(d) = o.repulsion[d];
	}
}

DSObstacle::DSObstacle(int dim)
{
	center.Resize(dim);

	Rotation.Resize(dim,dim);
	Rotation.Identity();

	axes.Resize(dim);
	axes.One();

	power.Resize(dim);
	power.One();

	e_amp.Resize(dim+1);
	e_amp.One();

	safetyFactor.Resize(dim);
	safetyFactor = safetyFactor.One();

	if (dim == 3){
		E.Resize(3,4);
	}else{
		E.Resize(dim,dim);
	}
	E.Identity();

	M.Resize(dim,dim);
	M.Identity();
}

void DSObstacle::Print()
{
	std::cout << "Axes" << "\n";
	axes.Print();
	std::cout << "Center" << "\n";
	center.Print();
	std::cout << "Rotation" << "\n";
	Rotation.Print();
	std::cout << "Power" << "\n";
	power.Print();
	std::cout << "Safety Factor" << "\n";
	safetyFactor.Print();
	std::cout << "E_amp" << "\n";
	e_amp.Print();
	std::cout << "E" << "\n";
	E.Print();
	std::cout << "M" << "\n";
	M.Print();
}


void DSAvoid::init(int num_obs)
{
	Gamma.Resize(num_obs);
	Gamma.Zero();

	x_t.Resize(dim); //dim is the dimension of DS, which for your case is 2
	xd_old.Resize(dim);
	vec_tmp.Resize(dim);
	nv.Resize(dim);
	e.Resize(dim);
	b_contouring = false;
	xd_contouring.Resize(dim);
	R_transpose.Resize(dim,dim);
	nv_rotated.Resize(dim);
	mat_tmp3.Resize(dim,dim);
	mat_tmp4.Resize(dim,dim);
	ind.resize(num_obs);
	if (dim == 3){
		d.Resize(4);
		D.Resize(4,4);
		mat_tmp1.Resize(4,3);
		mat_tmp2.Resize(4,3);
	}else{
		d.Resize(dim);
		D.Resize(dim,dim);
		mat_tmp1.Resize(dim,dim);
		mat_tmp2.Resize(dim,dim);
	}
	c_obs = 0; //current obstacle number (used for changing the obstacle properties in DS_Command)

}

bool DSAvoid::Avoid(Vector &x,Vector &xd)
{
    xd_old = xd;

    for (int i=0; i<num_obs;i++){
		x.Sub(obs[i].center,vec_tmp);

		obs[i].Rotation.Transpose(R_transpose);
        R_transpose.Mult(vec_tmp,x_t); //x_t = R_transpose*vec_tmp;
		x_t /= obs[i].axes;
		x_t /= obs[i].safetyFactor;

		nv = obs[i].power;
		nv /= obs[i].axes;
        for (int j=0;j<3;j++)
			nv[j] *= pow(x_t[j],2.0*obs[i].power[j]-1.0);

        // generating the matrix of the basis vector E. For example for a 2D model it simply is: E = [nv [-nv(2);nv(1)]];
        obs[i].E.SetColumn(nv,0); //nv is in fact the normal vector of the tangential hyper-plane
        for (int j=1;j<dim;j++){
            obs[i].E(0,j) = nv[j];
            obs[i].E(j,j) = -nv[0];
        }

        if (dim == 3){
            obs[i].E(0,3) = 0;
            obs[i].E(1,3) = nv(2);
            obs[i].E(2,3) = -nv(1);
        }

        /* its equivalent in 3D
            E(0,0) = nv(0);         E(0,1) = nv(1);         E(0,2) = nv(2);         E(0,3) = 0;
            E(1,0) = nv(1);         E(1,1) = -nv(0);	E(1,2) = 0;		E(1,3) = nv(2);
            E(2,0) = nv(2);         E(2,1) = 0;		E(2,2) = -nv(0);	E(2,3) = -nv(1);
        */

        Gamma[i] = 0;
        for (int j=0;j<dim;j++)
			Gamma[i] += pow(x_t[j],2.0*obs[i].power[j]);

        d.One();

		obs[i].Rotation.Mult(nv,nv_rotated); //obs[i].R*nv
		/*
        if (xd*nv_rotated < 0) //the obstacle is not passed yet
            d[0] = -1.0; //0
        else //the obstacle is already passed, no need to do anything
            d[0] = 0.0;
		*/
		d[0] = -1.0; //0

        d /= Gamma[i]; //Computing eigen-values

        if (d[0] < -1){
			d.One();
			d[0] = (nv_rotated*xd < 0) ? -1.0 : 0.0;
			xd.Zero();
			return false;
        }
        d += 1; //+1 is for the identity matrix
        D.Diag(d);

        //obs[i].M = obs[i].R*(obs[i].E*D*obs[i].E.Inverse())*R_transpose;
        if (dim == 3){
            obs[i].E.MultTranspose2(obs[i].E,mat_tmp3);
            mat_tmp3.SInverseSymmetric();
            obs[i].E.TransposeMult(mat_tmp3,mat_tmp1);
        }else
            obs[i].E.Inverse(mat_tmp1);

        D.Mult(mat_tmp1,mat_tmp2);
        obs[i].E.Mult(mat_tmp2,mat_tmp3);
		obs[i].Rotation.Mult(mat_tmp3,mat_tmp4);
        mat_tmp4.Mult(R_transpose,obs[i].M);

    } //end of for 0:num_obs-1

    Gamma.Sort(&ind); //sorting Gamma decreasingly

    //applying the modulation
    for (int i=0 ; i<num_obs ; i++){
        vec_tmp = xd;
        obs[ind.at(i)].M.Mult(vec_tmp,xd); //xd = obs[ind.at(i)].M*vec_tmp;
    }

    i_end = ind.at(num_obs-1);


    //to avoid instability if we numerically enters into the obstacle
	obs[i_end].Rotation.Mult(nv,nv_rotated); //obs[i_end].R*nv
    if (!b_contouring && d[0] < 0.01 && nv_rotated*xd_old < 0 && xd.Norm() < 0.05){ // Gamma(x_t) <= 1
        b_contouring = true;
        //std::cout << "contouring started ... " << "\n";
    }

    if (b_contouring){
        xd_contouring.Zero();

        for (int i = 1; i<dim ; i++){
            obs[i_end].E.GetColumn(i,e);
            if (e[2] < 0) //shit
				e *= -1;

            if (e.Norm() > 0.0001)
                e *= obs[i_end].e_amp[i]/e.Norm();
			xd_contouring += e; //contouring
        }

        vec_tmp = xd_contouring;
		obs[i_end].Rotation.Mult(vec_tmp,xd_contouring); //xd_contouring = obs[i_end].R * xd_contouring;
        if (((xd_contouring*xd > 0) && xd.Norm() > 0.05) || nv_rotated*xd_old >= 0)  { //
            b_contouring = false;
            //std::cout << "contouring stopped ... " << "\n";
        }

        xd = xd_contouring;
    }

    /*
    if (xd.Norm()!=0)
        xd *= xd_norm/xd.Norm();
    */

	return true;
}
