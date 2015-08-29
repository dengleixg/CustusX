#include "cxUr5State.h"


namespace cx
{

Ur5State::Ur5State()
{
    jointAxisVelocity << 0,0,0;
    jointAngleVelocity << 0,0,0;
}

Ur5State::Ur5State(double x,double y,double z,double rx,double ry,double rz)
{
    cartAxis << x, y, z;
    cartAngles << rx,ry,rz;
}

Ur5State::Ur5State(bool updt)
{
    updated = updt;
}

Ur5State::~Ur5State()
{
}


} // cx
