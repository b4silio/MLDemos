#include "DSModule.h"

RobotInterface::Status DSModule::RobotInit(){
        //stuffs for the obstacle avoidance
    obs = new obstacle[num_obs_max];

    b_obstacle = loadObstacleModel();
    if (b_obstacle){
        GetConsole()->Print("The obstacle module was activated.");
        Gamma.Resize(num_obs);Gamma.Zero();
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
    }else{
        GetConsole()->Print("No obstacle is defined.");
        GetConsole()->Print("The obstacle module is de-active.");
    }

    return STATUS_OK;
}
