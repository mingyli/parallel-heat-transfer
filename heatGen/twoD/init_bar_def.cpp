void init_bar( node_t *tnodes, double bar_size, double ltem, double rtem )
{        
    // Number of nodes to create
    double step = 1.0/(mesh_pts-1);
    for (int j = 0; j < mesh_pts; j++) {
        tnodes[j].T = ltem;
        tnodes[j].T_sum = 0;
        tnodes[j].x = step*j;
        tnodes[j].y = 0;
        tnodes[j].fixed = true;
        tnodes[j].edge = true;

        tnodes[(mesh_pts-1)*mesh_pts + j].T = rtem;
        tnodes[(mesh_pts-1)*mesh_pts + j].T_sum = 0;
        tnodes[(mesh_pts-1)*mesh_pts + j].x = step*j;
        tnodes[(mesh_pts-1)*mesh_pts + j].y = bar_size;
        tnodes[(mesh_pts-1)*mesh_pts + j].fixed = true;
        tnodes[(mesh_pts-1)*mesh_pts + j].edge = true;
    }
    
    for (int i = 1; i < mesh_pts-1; i++) {
        for (int j = 0; j < mesh_pts; j++) {
            tnodes[mesh_pts*i + j].T = T_default;
            tnodes[mesh_pts*i + j].T_sum = 0;
            tnodes[mesh_pts*i + j].x = (double) step*j;
            tnodes[mesh_pts*i + j].y = (double) step*i;
            tnodes[mesh_pts*i + j].fixed = false;
            tnodes[mesh_pts*i + j].edge = false;
        }  
        tnodes[mesh_pts*i].edge = true;
        tnodes[mesh_pts*i + (mesh_pts-1)].edge = true;
    }
}
