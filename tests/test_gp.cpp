#include <iostream>
#include <time.h>

#include "gp/SampleSet.h"
#include "gp/GaussianProcess.h"

using namespace gp;
using namespace std;

int main( int argc, char** argv )
{
        /*****  Global variables  ******************************************/
        size_t N_sur = 1000, N_ext = 50, N_int = 1;
        double noise = 0.001;
        const double PI = numeric_const<double>::PI;

        /* initialize random seed: */
  	srand (time(NULL));

  	const bool prtInit = true;
  	const bool prtPreds = true;

        /*****  Generate Input data  ******************************************/
        Vec3Seq cloud;
        Vec targets;
        printf("Generate input data %lu\n", N_sur + N_ext + N_int);
        if (prtInit)
        	printf("Cloud points %lu:\n", N_sur);
        for (size_t i = 0; i < N_sur; ++i) {
        	const double theta = 2*PI*(rand() / double(RAND_MAX)) - PI;
        	const double z = 2*(rand() / double(RAND_MAX)) - 1;
        	Vec3 point(sin(theta)*sqrt(1-z*z), cos(theta)*sqrt(1-z*z), z);
        	point += Vec3(2*noise*(rand() / double(RAND_MAX)) - noise, 2*noise*(rand() / double(RAND_MAX)) - noise, 2*noise*(rand() / double(RAND_MAX)) - noise);
        	cloud.push_back(point);

        	double y = point.magnitudeSqr() - 1;
        	if (prtInit) cout << "points[" << i << "] th(" << theta <<") = <" << point.x << " " << point.y << " " << point.z << ">" << " targets[" << i << "] = " << y << endl;
        	targets.push_back(y);
       	}

       	if (prtInit)
       		printf("\nExternal points %lu:\n", N_ext);
       	for (size_t i = 0; i < N_ext; ++i) {
        	const double theta = 2*PI*(rand() / double(RAND_MAX)) - PI;
        	const double z = 2*(rand() / double(RAND_MAX)) - 1;
        	Vec3 point(sin(theta)*sqrt(2-z*z), cos(theta)*sqrt(2-z*z), z);
        	point += Vec3(2*noise*(rand() / double(RAND_MAX)) - noise, 2*noise*(rand() / double(RAND_MAX)) - noise, 2*noise*(rand() / double(RAND_MAX)) - noise);
        	cloud.push_back(point);

        	double y = point.magnitudeSqr() - 1;
        	if (prtInit)
        		if (prtInit) cout << "points[" << i << "] th(" << theta <<") = <" << point.x << " " << point.y << " " << point.z << ">" << " targets[" << i << "] = " << y << endl;
        	targets.push_back(y);
       	}

       	if (prtInit)
       		printf("\nInternal point(s) %lu:\n", N_int);
       	Vec3 zero; zero.setZero();
       	cloud.push_back(zero);
       	targets.push_back(zero.magnitudeSqr() - 1);
       	if (prtInit) cout << "points[" << 1 << "] th(" << 0 <<") = <" << zero.x << " " << zero.y << " " << zero.z << ">" << " targets[" << 1 << "] = " << zero.magnitudeSqr() - 1 << endl << endl;

       	/*****  Create the model  *********************************************/
       	SampleSet::Ptr trainingData(new SampleSet(cloud, targets));
       	LaplaceRegressor::Desc laplaceDesc;
       	laplaceDesc.noise = noise;
        LaplaceRegressor::Ptr gp = laplaceDesc.create();
        printf("Regressor created %s\n", gp->getName().c_str());
        gp->set(trainingData);
        //ThinPlateRegressor gp(&trainingData);


        /*****  Query the model with a point  *********************************/
        Vec3 q(cloud[0]);
        const double qf = gp->f(q);
        const double qVar = gp->var(q);

        if (prtPreds) cout << "y = " << targets[0] << " -> qf = " << qf << " qVar = " << qVar << endl << endl;

        /*****  Query the model with new points  *********************************/
	const size_t testSize = 100;
	const double range = 1.0;
	Vec3Seq x_star; x_star.resize(testSize);
	Vec y_star; y_star.resize(testSize);
	for (size_t i = 0; i < testSize; ++i) {
		const double theta = 2*PI*(rand() / double(RAND_MAX)) - PI;
        	const double z = 2*(rand() / double(RAND_MAX)) - 1;
        	Vec3 point(sin(theta)*sqrt(1-z*z), cos(theta)*sqrt(1-z*z), z);
        	point += Vec3(2*noise*(rand() / double(RAND_MAX)) - noise, 2*noise*(rand() / double(RAND_MAX)) - noise, 2*noise*(rand() / double(RAND_MAX)) - noise);

        	// save the point for later
        	x_star[i] = point;
        	// compute the y value
        	double y = point.magnitudeSqr() - 1;
        	// save the y value for later
        	y_star[i] = y;

        	// gp estimate of y value
        	const double f_star = gp->f(point);
		const double v_star = gp->var(point);
        	if (prtPreds)
        		cout << "f(x_star) = " << y << " -> f_star = " << f_star << " v_star = " << v_star << endl;
	}

	/*****  Add point to the model  *********************************************/
	gp->add_patterns(x_star, y_star);

	// test on the first x_star point
	Vec3 q2(x_star[0]);
        const double q2f = gp->f(q2);
        const double q2Var = gp->var(q2);

        if (prtPreds) cout << "y = " << y_star[0] << " -> q2f = " << q2f << " q2Var = " << q2Var << endl << endl;




//        /*****  Create the model  *********************************************/
//        Model mug;
//        GaussianRegressor regresor;
//
//        regresor.create(cloud, mug);

//        cout << "Model points: " << endl;
//        cout << mug.P << endl << endl;
//        cout << "Model labels (pre): " << endl;
//        cout << mug.Y << endl << endl;
//        cout << "Model Kpp: " << endl;
//        cout << mug.Kpp << endl << endl;

//        /*****  Query the model with a point  *********************************/
//        Data query;
//        query.coord_x = vector<double>(1, 0.68);
//        query.coord_y = vector<double>(1, 0.5);
//        query.coord_z = vector<double>(1, 0.5);

//        regresor.estimate(mug, query);

//        cout << "Query value: " << endl;
//        cout << query.coord_x.at(0) << " " << query.coord_y.at(0) << " "
//                        << query.coord_z.at(0) << endl << endl;
//        cout << "Query label (post): " << endl;
//        cout << query.label.at(0) << endl << endl;
//        // cout << "Query label covariance: " << endl;
//        // cout << query.ev_x.at(0) << " " << query.ev_y.at(0)
//                        // << " " << query.ev_z.at(0)
//                        // << endl << endl;

//        /*****  Query the model with a INPUT point  ***************************/
//        query.label.clear();
//        query.coord_x = vector<double>(1, 0.1);
//        query.coord_y = vector<double>(1, 0.3);
//        query.coord_z = vector<double>(1, 0.5);

//        regresor.estimate(mug, query);

//        cout << "Query same input: " << endl;
//        cout << query.coord_x.at(0) << " " << query.coord_y.at(0) << " "
//                        << query.coord_z.at(0) << endl << endl;
//        cout << "Query label (post): " << endl;
//        cout << query.label.at(0) << endl << endl;
//        // cout << "Query label covariance: " << endl;
//        // cout << query.ev_x.at(0) << " " << query.ev_y.at(0)
//                        // << " " << query.ev_z.at(0)
//                        // << endl << endl;

        return 0;
}
