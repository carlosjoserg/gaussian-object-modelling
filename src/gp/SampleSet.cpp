/** @file SampleSet.cpp
 * 
 * 
 * @author	Claudio Zito
 *
 */
#include "gp/SampleSet.h"
#include <Eigen/StdVector>

//------------------------------------------------------------------------------

namespace gp {

//------------------------------------------------------------------------------

SampleSet::SampleSet() {
	n = 0;
}

SampleSet::SampleSet(const Vec3Seq& inputs, const Vec& targets) {
	assert(inputs.size()==targets.size());

	n = inputs.size();
//        X.resize(n, 3);
//        for (size_t i = 0; i < n; ++i) {
//        	X.row(i) = Eigen::Map<Eigen::VectorXd>((double *)inputs[i].v, 3);
//        }
	X.insert(X.end(), inputs.begin(), inputs.end());
        Vec tmp = targets;
        Y.insert(Y.end(), tmp.begin(), tmp.end());
//        Y.resize(n);
//        Y = Eigen::Map<Eigen::VectorXd>((double *)targets.data(), targets.size());
}

SampleSet::~SampleSet() {
	clear();
}

//------------------------------------------------------------------------------

//void SampleSet::add(const double x[], double y) {
//	Eigen::VectorXd * v = new Eigen::VectorXd(input_dim);
//	for (size_t i=0; i<input_dim; ++i) (*v)(i) = x[i];
//	X.push_back(v);
//	Y.push_back(y);
//	assert(X.size()==Y.size());
//	n = X.size();
//}

//void SampleSet::add(const Eigen::VectorXd x, double y) {
//	Eigen::VectorXd * v = new Eigen::VectorXd(x);
//	X.push_back(v);
//	Y.push_back(y);
//	assert(X.size()==Y.size());
//	n = X.size();
//}

void SampleSet::add(const Vec3Seq& newInputs, const Vec& newTargets) {
	assert(newInputs.size()==newTargets.size());
	
	n += newInputs.size(); 
//        X.resize(n, 3);
//        for (size_t i = 0; i < n; ++i) {
//        	X.row(i) = Eigen::Map<Eigen::VectorXd>((double *)newInputs[i].v, 3);
//        }
        X.insert(X.end(), newInputs.begin(), newInputs.end());
        Vec tmp = newTargets;
        Y.insert(Y.end(), tmp.begin(), tmp.end());
//	Y.resize(Y.size() + newTargets.size());
//        Y = Eigen::Map<Eigen::VectorXd>((double *)newTargets.data(), newTargets.size());
}

//------------------------------------------------------------------------------

const Vec3& SampleSet::x(size_t k) {
	return X[k];
}

double SampleSet::y(size_t k) {
	return Y[k];
}

const Vec& SampleSet::y() {
	return Y;
}

bool SampleSet::set_y(size_t i, double y) {
	if (i>=n) return false;
	Y[i] = y;
	return true;
}

//------------------------------------------------------------------------------

void SampleSet::clear()	{
	n = 0;
	X.clear();
	Y.clear();
}

//------------------------------------------------------------------------------

}
