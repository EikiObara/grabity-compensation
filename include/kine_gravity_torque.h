//2017/12/18
//auther:eiki obara

#ifndef __KINE_GRAVITY_COMPENSATION_H__
#define __KINE_GRAVITY_COMPENSATION_H__

#include "kine_centerOfGravity.h"
#include "kine_defines.h"
#include "kine_math_consts.h"

namespace Trl{

class GravityTorque{
private:
	ArmCoG armcogObj;

	CoGT weight;

	void GetArmTorque(JointNameT jointName,HTM htmObj,TorqueT &ret);
public:
	void SetCoGParam(CoGT _length, CoGT _weight);

	void GetJointTorque(const int jointNum,HTM htmObj,JointT jointRad,TorqueT &ret);
};

void GravityTorque::SetCoGParam(CoGT _length,CoGT _weight){
	armcogObj.SetCoGLength(_length);
	weight = _weight;

}

void GravityTorque::GetArmTorque(JointNameT jointName,HTM htmObj,TorqueT &torq){
	torq = TorqueT::Zero();

	CoGT bufCog = CoGT::Zero();

	switch(jointName){
	case SHOULDER:
		armcogObj.Get(SHOULDER,UPPER,htmObj,bufCog);
		torq += bufCog.cross(weight(0)*kGravityAccel);	//0=upperWeight
		armcogObj.Get(SHOULDER,FORWARD,htmObj,bufCog);
		torq += bufCog.cross(weight(1)*kGravityAccel);	//1=foreWeight
		armcogObj.Get(SHOULDER,HAND,htmObj,bufCog);
		torq += bufCog.cross(weight(2)*kGravityAccel);	//2=handWeight

		break;
	case ELBOW:
		armcogObj.Get(ELBOW,FORWARD,htmObj,bufCog);
		torq += bufCog.cross(weight(1)*kGravityAccel);	//1=foreWeight
		armcogObj.Get(ELBOW,HAND,htmObj,bufCog);
		torq += bufCog.cross(weight(2)*kGravityAccel);	//2=handWeight
	
		break;
	case WRIST:
		armcogObj.Get(WRIST,HAND,htmObj,bufCog);
		torq += bufCog.cross(weight(2)*kGravityAccel);	//2=handWeight

		break;
	default:
		break;
	}

	//std::cout << "weight\n" << weight << std::endl;
	//std::cout << "torq\n" << torq << std::endl;

	for(int i = 0;i < torq.rows(); ++i){
		if(fabs(torq(i)) < 1.0e-8){
			torq(i) = 0.0;
		}
	}
}

//関節番号、関節角度、返り値のトルク
void GravityTorque::GetJointTorque(const int jointNum,HTM htmObj,JointT jointRad,TorqueT &ret){
	TorqueT torqBuf;
	HtmT retHtm = HtmT::Identity();
	HtmT inv = HtmT::Zero();

	htmObj.CalcHTM(jointRad);
	
	switch(jointNum){
	case 0:
	case 1:
	case 2:
		GetArmTorque(SHOULDER,htmObj,torqBuf);
		htmObj.GetHTM(0,jointNum,retHtm);

		break;
	case 3:
		GetArmTorque(ELBOW,htmObj,torqBuf);
		htmObj.GetHTM(0,jointNum,retHtm);

		break;
	case 4:
	case 5:
	case 6:
		GetArmTorque(WRIST,htmObj,torqBuf);
		htmObj.GetHTM(0,jointNum,retHtm);

		break;
	default:
		torqBuf = TorqueT::Zero();
		break;
	}

	inv = retHtm.inverse();

	//std::cout << "inv\n" << inv << std::endl;
	//std::cout << "torqBuf\n" << torqBuf << std::endl;

	RotMatT rotmat;

	for(int i = 0; i < 3; ++i){
		for(int j = 0;j < 3; ++j){
			rotmat(i,j) = inv(i,j);
		}
	}

	ret = rotmat * torqBuf;
}

}	//namespace Trl

#endif //__KINE_GRAVITY_COMPENSATION_H__