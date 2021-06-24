#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <algorithm>


// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
    CurvePoint(Vector3f Vertex, Vector3f Tangent):V(Vertex),T(Tangent){}
};

class Curve : public Object3D {
protected:
    std::vector<Vector3f> controls;
public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {
        radius = fabs(controls[0][0]);
        y_min = controls[0][1];
        y_max = controls[0][1];
        for (int i=1; i<controls.size(); i++) {
            radius = fmax(radius, fabs(controls[i][0]));
            radius = fmax(radius, fabs(controls[i][2]));
            y_min = fmin(y_min, controls[i][1]);
            y_max = fmax(y_max, controls[i][1]);
		}
    }

    bool intersect(const Ray &r, Hit &h, float tmin, float tmax) const override {
        return false;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const {
        Vector3f p_min = controls[0];
		Vector3f p_max = controls[0];
		for (int i=1; i<controls.size(); i++) {
			for (int j=0; j<3; j++) {
				p_min[j] = fmin(p_min[j], controls[i][j]);
				p_max[j] = fmax(p_max[j], controls[i][j]);
			}
		}
		p_min = p_min - Vector3f(0.001,0.001,0.001);
		p_max = p_max + Vector3f(0.001,0.001,0.001);
    	output_box = AABB (p_min, p_max);
		return true;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }
 
    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;

    virtual CurvePoint caculate(double mu) = 0;

    float radius, y_min, y_max;
    float range[2];
protected:
    int n;
    int k;
    std::vector<double> knot;
    double **baseFunction;

};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
        n=controls.size()-1;
        k=n;
        knot.reserve(n+k+2);
        for(int i=0;i<n+1;i++){
            knot.push_back(0.0);
        }
        for(int i=n+1;i<n+k+2;i++){
            knot.push_back(1.0);
        }
        baseFunction=new double* [n+k+1];
        for(int i=0;i<n+k+1;i++){
            baseFunction[i]=new double[k+1];// 后续可考虑滚动数组优化
            for(int j=0;j<=k;j++){
                baseFunction[i][j]=0.0;
            }
        }
        range[0] = (float)k/(n+k+1);
        range[1] = (float)(n+1)/(n+k+1);
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // fill in data vector
        for(int i=k;i<n+1;i++){// valid range
            if(knot[i]==knot[i+1])continue;//重复节点不必重复采样
            for(int j=0;j<resolution;j++){// sampling
                double mu=knot[i]+j*(knot[i+1]-knot[i])/double(resolution);
                data.push_back(caculate(mu));
            }
        }
    }

    CurvePoint caculate(double mu) override {
        Vector3f vertex=Vector3f::ZERO;
        Vector3f tangent=Vector3f::ZERO;
        for(int i=0;i<n+k+1;i++){
            if(mu>=knot[i]&&mu<knot[i+1]){
                baseFunction[i][0]=1.0;
            }
            else baseFunction[i][0]=0.0;
        }
        for(int j=1;j<=k;j++){
            for(int i=0;i<n+(k-j)+1;i++){
                double tmp1=0;
                if(knot[i+j]!=knot[i]){
                    tmp1=(mu-knot[i])/(knot[i+j]-knot[i]);
                }
                double tmp2=0;
                if(knot[i+j+1]-knot[i+1]){
                    tmp2=(knot[i+j+1]-mu)/(knot[i+j+1]-knot[i+1]);
                }
                baseFunction[i][j]=tmp1*baseFunction[i][j-1]+tmp2*baseFunction[i+1][j-1];
            }
        }
        vertex=Vector3f::ZERO;
        for(int i=0;i<=n;i++){
            vertex+=baseFunction[i][k]*controls[i];
        }
        tangent=Vector3f::ZERO;
        for(int i=0;i<=n;i++){
            double tmp1=0;
            if(knot[i+k]!=knot[i]){
                tmp1=baseFunction[i][k-1]/(knot[i+k]-knot[i]);
            }
            double tmp2=0;
            if(knot[i+k+1]!=knot[i+1]){
                tmp2=baseFunction[i+1][k-1]/(knot[i+k+1]-knot[i+1]);
            }
            tangent+=k*(tmp1-tmp2)*controls[i];
        }
        return CurvePoint(vertex, tangent);
    }  
};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
        n=controls.size()-1;
        k=3;
        knot.reserve(n+k+2);
        for(int i=0;i<n+k+2;i++){
            knot.push_back(double(i)/double(n+k+1));
        }
        baseFunction=new double* [n+k+1];
        for(int i=0;i<n+k+1;i++){
            baseFunction[i]=new double[k+1];// 后续可考虑滚动数组优化
            for(int j=0;j<=k;j++){
                baseFunction[i][j]=0.0;
            }
        }
        range[0] = (float)k/(n+k+1);
        range[1] = (float)(n+1)/(n+k+1);
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // fill in data vector
        for(int i=k;i<n+1;i++){// valid range
            if(knot[i]==knot[i+1])continue;//重复节点不必重复采样
            for(int j=0;j<resolution;j++){// sampling
                double mu=knot[i]+j*(knot[i+1]-knot[i])/double(resolution);
                data.push_back(caculate(mu));
            }
        }
    }

    CurvePoint caculate(double mu) override {
        Vector3f vertex=Vector3f::ZERO;
        Vector3f tangent=Vector3f::ZERO;
        for(int i=0;i<n+k+1;i++){
            if(mu>=knot[i]&&mu<knot[i+1]){
                baseFunction[i][0]=1.0;
            }
            else baseFunction[i][0]=0.0;
        }
        for(int j=1;j<=k;j++){
            for(int i=0;i<n+(k-j)+1;i++){
                double tmp1=0;
                if(knot[i+j]!=knot[i]){
                    tmp1=(mu-knot[i])/(knot[i+j]-knot[i]);
                }
                double tmp2=0;
                if(knot[i+j+1]-knot[i+1]){
                    tmp2=(knot[i+j+1]-mu)/(knot[i+j+1]-knot[i+1]);
                }
                baseFunction[i][j]=tmp1*baseFunction[i][j-1]+tmp2*baseFunction[i+1][j-1];
            }
        }
        vertex=Vector3f::ZERO;
        for(int i=0;i<=n;i++){
            vertex+=baseFunction[i][k]*controls[i];
        }
        tangent=Vector3f::ZERO;
        for(int i=0;i<=n;i++){
            double tmp1=0;
            if(knot[i+k]!=knot[i]){
                tmp1=baseFunction[i][k-1]/(knot[i+k]-knot[i]);
            }
            double tmp2=0;
            if(knot[i+k+1]!=knot[i+1]){
                tmp2=baseFunction[i+1][k-1]/(knot[i+k+1]-knot[i+1]);
            }
            tangent+=k*(tmp1-tmp2)*controls[i];
        }
        return CurvePoint(vertex, tangent);
    }
};

#endif // CURVE_HPP
