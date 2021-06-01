#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


class Group : public Object3D {

public:

    Group() {

    }

    explicit Group (int num_objects) {
        object.reserve(num_objects);
    }

    ~Group() override {

    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool flag=false;
        for(auto it=object.begin();it!=object.end();it++){
            if((*it)->intersect(r,h,tmin)){
                flag=true;
            }
        }
        return flag;
    }

    void addObject(int index, Object3D *obj) {
        auto pos=object.begin();
        while (index){
            index--;
            pos++;
        }
        object.insert(pos,obj);
    }

    int getGroupSize() {
        return object.size();
    }

private:
    std::vector<Object3D*> object;
};

#endif
	
