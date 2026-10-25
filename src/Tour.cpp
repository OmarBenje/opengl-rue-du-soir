#include "Tour.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

namespace {
// Closed route; camera and point of interest have independent smooth tracks.
const std::vector<CameraPose> stops = {
    {{ 1.4f,3.0f, 27.f},{-1.f,2.6f, 3.f}},
    {{-1.2f,2.4f, 19.f},{-5.f,2.8f, 8.f}},
    {{ 0.6f,2.1f, 10.f},{ 4.8f,2.4f,-1.f}},
    {{ 1.1f,2.6f,  0.f},{-4.f,3.0f,-12.f}},
    {{-0.9f,3.6f,-12.f},{ 0.f,4.1f,-31.f}},
    {{-0.6f,5.2f,-19.f},{ 0.f,4.1f,-31.f}},
    // Look sideways during the turn, avoiding a near-vertical sweep at the ground.
    {{-1.0f,9.5f,-23.f},{10.f,4.0f,-13.f}},
    {{ 0.8f,13.f, -6.f},{ 0.f,1.0f, 8.f}},
    {{ 1.8f,11.f, 18.f},{ 0.f,2.0f, 2.f}},
};
glm::vec3 catmull(glm::vec3 a,glm::vec3 b,glm::vec3 c,glm::vec3 d,float t) {
    return .5f*((2.f*b)+(-a+c)*t+(2.f*a-5.f*b+4.f*c-d)*t*t+(-a+3.f*b-3.f*c+d)*t*t*t);
}
}
TourCamera::TourCamera() { mFOV=57.f; sample(0); }
CameraPose TourCamera::pose(float seconds) {
    float s=std::fmod(seconds,duration); if(s<0) s+=duration;
    float segment=s/duration*stops.size();
    int i=static_cast<int>(segment), n=static_cast<int>(stops.size()); float t=segment-i;
    auto a=stops[(i+n-1)%n],b=stops[i],c=stops[(i+1)%n],d=stops[(i+2)%n];
    return {catmull(a.position,b.position,c.position,d.position,t),catmull(a.target,b.target,c.target,d.target,t)};
}
void TourCamera::sample(float seconds) {
    auto p=pose(seconds); mPosition=p.position; mTargetPos=p.target;
    mLook=glm::normalize(mTargetPos-mPosition);
    mRight=glm::normalize(glm::cross(mLook,WORLD_UP));
    mUp=glm::normalize(glm::cross(mRight,mLook));
}
bool TourCamera::verify() {
    float maxStep=0;
    for(int i=0;i<=3600;++i) {
        auto p=pose(i/60.f),next=pose((i+1)/60.f);
        for(int k=0;k<3;++k) if(!std::isfinite(p.position[k]) || !std::isfinite(p.target[k])) return false;
        if(p.position.y<1.6f || std::abs(p.position.x)>2.6f || glm::length(p.target-p.position)<1.f) return false;
        maxStep=std::max(maxStep,glm::length(next.position-p.position));
    }
    auto a=pose(0),b=pose(duration);
    bool ok=glm::length(a.position-b.position)<.0001f && maxStep<.15f;
    std::cout<<"60 s closed tour: finite poses, clear street corridor, seamless loop. Max step @60fps: "<<maxStep<<" m\n";
    return ok;
}
