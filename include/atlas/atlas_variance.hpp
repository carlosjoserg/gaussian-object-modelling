#ifndef GP_ATLAS_VARIANCE_HPP_
#define GP_ATLAS_VARIANCE_HPP_

#include <atlas/atlas.hpp>

namespace gp_atlas_rrt
{

class AtlasVariance : public AtlasBase
{
    public:
    typedef std::shared_ptr<AtlasVariance> Ptr;
    typedef std::shared_ptr<const AtlasVariance> ConstPtr;

    AtlasVariance()=delete;
    AtlasVariance(const gp_regression::Model::Ptr &gp, const gp_regression::ThinPlateRegressor::Ptr &reg):
        AtlasBase(gp,reg), var_factor(0.65), disc_samples_factor(500)
    {
        var_tol = 0.5; //this should be give by user
                       //whoever uses this class will take care of it, by calling
                       //setVarianceTolGoal()
    }

    ///Set the variance tolerance to surpass, for a node to be considered solution
    virtual inline void setVarianceTolGoal(const double vt)
    {
        var_tol = vt;
    }

    // ///reset Atlas with new parameters and then recieve a new starting point (root)
    // virtual void init(const double var_tolerance, const gp_regression::Model::Ptr &gpm, const gp_regression::ThinPlateRegressor::Ptr &gpr)
    // {
    //     clear();
    //     var_tol = var_tolerance;
    //     setGPModel(gpm);
    //     setGPRegressor(gpr);
    // }

    virtual double computeRadiusFromVariance(const double v) const
    {
        if (v > 0.5){
            std::cout<<"Variance is too big "<<v<<std::endl;
            return 0.01;
        }
        if (v < 0){
            std::cout<<"Variance is negative "<<v<<std::endl;
            return 0.3;
        }
        if (std::isnan(v) || std::isinf(v)){
            std::cout<<"Variance is NAN / Inf "<<v<<std::endl;
            return 0.05;
        }
        return ( -var_factor*v + 0.3);
    }

    virtual std::size_t createNode(const Eigen::Vector3d& center)
    {
        if (!gp_reg)
            throw gp_regression::GPRegressionException("Empty Regressor pointer");
        gp_regression::Data::Ptr c = std::make_shared<gp_regression::Data>();
        c->coord_x.push_back(center(0));
        c->coord_y.push_back(center(1));
        c->coord_z.push_back(center(2));
        std::vector<double> f,v;
        Eigen::MatrixXd gg;
        gp_reg->evaluate(gp_model, c, f, v, gg);
        Eigen::Vector3d g = gg.row(0);
        if (g.isZero(1e-3)){
            std::cout<<"[Atlas::createNode] Chart gradient is zero. g = "<<g<<std::endl;
            c->clear();
            c->coord_x.push_back(center(0) + getRandIn(1e-5, 1e-3));
            c->coord_y.push_back(center(1) + getRandIn(1e-5, 1e-3));
            c->coord_z.push_back(center(2) + getRandIn(1e-5, 1e-3));
            gp_reg->evaluate(gp_model, c, f, v, gg);
            g=gg.row(0);
            throw gp_regression::GPRegressionException("Gradient is zero");
        }
        if (std::abs(f.at(0)) > 0.01 || std::isnan(f.at(0)) || std::isinf(f.at(0)))
            std::cout<<"[Atlas::createNode] Chart center is not on GP surface! f(x) = "<<f.at(0)<<std::endl;
        Chart node (center, nodes.size(), g, v.at(0));
        std::cout<<"[Atlas::createNode] Created node. Center is c = "<<center<<std::endl;
        node.setRadius(computeRadiusFromVariance(v.at(0)));
        nodes.push_back(node);
        return node.getId();
    }

    virtual Eigen::Vector3d getNextState(const std::size_t& id)
    {
        if (!gp_reg)
            throw gp_regression::GPRegressionException("Empty Regressor pointer");
        if (id >= nodes.size())
            throw gp_regression::GPRegressionException("Out of Range node id");
        //get some useful constants from the disc
        const Eigen::Vector3d N = nodes.at(id).getNormal();
        const Eigen::Vector3d Tx = nodes.at(id).getTanBasisOne();
        const Eigen::Vector3d Ty = nodes.at(id).getTanBasisTwo();
        const Eigen::Vector3d C = nodes.at(id).getCenter();
        const Eigen::Vector3d G = nodes.at(id).getGradient();
        const double R = nodes.at(id).getRadius();
        //prepare the samples storage
        const std::size_t tot_samples = std::ceil(disc_samples_factor * R);
        std::cout<<"total samples "<<tot_samples<<std::endl;
        nodes.at(id).samples.resize(tot_samples, 3);
        std::vector<double> f,v;
        //transformation into the kinect frame from local
        Eigen::Matrix4d Tkl;
        Tkl <<  Tx(0), Ty(0), N(0), C(0),
                Tx(1), Ty(1), N(1), C(1),
                Tx(2), Ty(2), N(2), C(2),
                0,     0,     0,    1;
        std::cout<<"Tkl "<<Tkl<<std::endl;
        //keep the max variance found
        double max_v(0.0);
        //and which sample it was
        std::size_t s_idx(0);
        //uniform annulus sampling from R/5 to R
        for (std::size_t i=0; i<tot_samples; ++i)
        {
            const double r = getRandIn(0.8, 1.0, true);
            const double th = getRandIn(0.0, 2*M_PI);
            //point in local frame, uniformely sampled
            Eigen::Vector4d pL;
            pL <<   R * std::sqrt(r) * std::cos(th),
                    R * std::sqrt(r) * std::sin(th),
                    0,
                    1;
            //the same point in kinect frame
            Eigen::Vector4d pK = Tkl * pL;
            gp_regression::Data::Ptr query = std::make_shared<gp_regression::Data>();
            query->coord_x.push_back(pK(0));
            query->coord_y.push_back(pK(1));
            query->coord_z.push_back(pK(2));
            //store the sample for future use (even plotting)
            nodes.at(id).samples(i,0) = pK(0);
            nodes.at(id).samples(i,1) = pK(1);
            nodes.at(id).samples(i,2) = pK(2);
            std::cout<<"pK "<<pK<<std::endl;
            //evaluate the sample
            gp_reg->evaluate(gp_model, query, f, v);
            if (v.at(0) > max_v){
                max_v = v.at(0);
                s_idx = i;
            }
        }
        //the winner is:
        Eigen::Vector3d chosen = nodes.at(id).samples.row(s_idx);
        //put winner on top (for visualization)
        nodes.at(id).samples.row(0).swap(nodes.at(id).samples.row(s_idx));
        Eigen::Vector3d nextState;
        //project the chosen
        project(chosen, nextState, G);
        //and done
        return nextState;
    }

    virtual inline bool isSolution(const std::size_t &id)
    {
        return (getNode(id).getVariance() > var_tol);
    }

    protected:
    //radius is inversely proportional to variance
    double var_factor;
    //how many disc samples multiplier (total samples are proportional to radius)
    //which in turn is proportional to variance
    std::size_t disc_samples_factor;
    //variance threshold for solution
    double var_tol;
};
}

#endif
