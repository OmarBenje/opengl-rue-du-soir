#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stb_easy_font.h>
#include <array>
#include <iostream>

namespace {
std::string root=STREET_ROOT;
glm::mat4 transform(glm::vec3 p,glm::vec3 s,float yaw) {
    return glm::scale(glm::rotate(glm::translate(glm::mat4(1.f),p),glm::radians(yaw),{0,1,0}),s);
}
}
Mesh* Scene::load(const std::string& name,const std::string& path) {
    auto mesh=std::make_unique<Mesh>(); mesh->loadOBJ(root+"/"+path);
    auto ptr=mesh.get(); meshes[name]=std::move(mesh); ++importedModels; return ptr;
}
Texture2D* Scene::texture(const std::string& name,const std::string& path) {
    auto t=std::make_unique<Texture2D>();
    if(!t->loadTexture(root+"/"+path,true)) throw std::runtime_error("Missing texture "+path);
    auto ptr=t.get(); textures[name]=std::move(t); return ptr;
}
void Scene::add(const std::string& mesh,glm::vec3 p,glm::vec3 s,Material m,float yaw,bool shadow) {
    objects.push_back({meshes.at(mesh).get(),transform(p,s,yaw),m,shadow});
}
void Scene::box(glm::vec3 p,glm::vec3 scale,glm::vec3 color,int surface) {
    Material m; m.color=color; m.surface=surface; add("cube",p,scale,m);
}

void Scene::sign(const std::string& text,glm::vec3 p,float yaw,glm::vec3 ink,float size,bool withBacking) {
    // Small vector letters mounted on an actual 3D storefront sign.
    alignas(float) std::array<unsigned char,20000> buffer{};
    int quads=stb_easy_font_print(0,0,const_cast<char*>(text.c_str()),nullptr,buffer.data(),buffer.size());
    float width=static_cast<float>(stb_easy_font_width(const_cast<char*>(text.c_str())));
    std::vector<Vertex> verts;
    for(int q=0;q<quads;q++) for(int i:{0,1,2,0,2,3}) {
        const float* v=reinterpret_cast<const float*>(buffer.data()+(q*4+i)*16);
        verts.push_back({{(v[0]-width*.5f)*.036f*size,(6-v[1])*.036f*size,.0f},{0,0,1},{0,0}});
    }
    auto mesh=std::make_unique<Mesh>(); mesh->setVertices(verts);
    Material m; m.color=ink; m.emission=ink*.25f;
    objects.push_back({mesh.get(),transform(p,{1,1,1},yaw),m,false});
    meshes["text-"+std::to_string(meshes.size())]=std::move(mesh);
    if(withBacking) {
        Material backing; backing.color={.035f,.08f,.09f};
        objects.push_back({meshes.at("cube").get(),transform(p-glm::vec3(sin(glm::radians(yaw)),0,cos(glm::radians(yaw)))*.075f,{(width*.036f+.35f)*size,.62f*size,.08f},yaw),backing,true});
    }
}

void Scene::bench(glm::vec3 p,float yaw) {
    auto part=[&](glm::vec3 offset,glm::vec3 scale,glm::vec3 color) {
        Material m; m.color=color;
        auto model=transform(p,{1,1,1},yaw)*transform(offset,scale,0);
        objects.push_back({meshes.at("cube").get(),model,m,true});
    };
    for(int i=0;i<4;i++) part({0,.52f,-.24f+i*.16f},{1.7f,.09f,.12f},{.32f,.14f,.065f});
    for(float x:{-.62f,.62f}) part({x,.25f,0},{.09f,.5f,.52f},{.06f,.08f,.09f});
    for(int i=0;i<3;i++) part({0,.8f+i*.14f,.29f},{1.7f,.11f,.08f},{.32f,.14f,.065f});
}

void Scene::tree(glm::vec3 p,float scale) {
    box(p+glm::vec3(0,.24f,0),{1.45f,.48f,1.45f},{.22f,.18f,.16f});
    box(p+glm::vec3(0,.50f,0),{1.28f,.08f,1.28f},{.045f,.065f,.04f});
    box(p+glm::vec3(0,1.5f*scale,0),{.18f,2.5f*scale,.18f},{.19f,.10f,.055f});
    for(int i=0;i<3;i++) {
        Material leaf; leaf.color=glm::vec3(.14f,.26f,.095f)+glm::vec3(.025f,.04f,.01f)*float(i);
        add("sphere",p+glm::vec3((i-1)*.42f,2.5f*scale+float(i%2)*.6f,.15f*(i-1)),{1.05f*scale,1.10f*scale,.95f*scale},leaf);
    }
}

void Scene::polytech() {
    // An original, stylised campus building; the front faces the street (+Z).
    const glm::vec3 concrete(.80f,.82f,.80f), frame(.16f,.22f,.28f);
    box({0,.10f,-31.5f},{14.6f,.20f,8.f},{.48f,.49f,.46f},1);
    box({0,4.15f,-34.3f},{13.6f,7.9f,6.8f},concrete);
    box({0,8.15f,-34.3f},{14.f,.25f,7.2f},{.90f,.91f,.87f});
    box({0,8.43f,-34.3f},{12.8f,.3f,5.8f},{.20f,.25f,.29f});

    Material blue; blue.color={.018f,.17f,.48f}; blue.emission={.005f,.035f,.10f};
    add("cube",{0,6.8f,-30.77f},{12.3f,1.9f,.20f},blue);
    add("cube",{-6.28f,4.1f,-30.74f},{.28f,7.4f,.24f},blue);
    sign("POLYTECH",{0,6.95f,-30.65f},0,{.94f,.98f,1.f},3.2f,false);
    sign("MARSEILLE",{0,6.28f,-30.65f},0,{.55f,.82f,1.f},1.45f,false);

    Material glass; glass.color={.035f,.12f,.20f}; glass.emission={.012f,.045f,.07f};
    glass.specular=.65f; glass.shininess=96.f;
    for(float x:{-4.9f,-2.6f,2.6f,4.9f}) for(float y:{1.75f,4.35f}) {
        box({x,y,-30.77f},{2.05f,1.60f,.15f},frame);
        add("cube",{x,y,-30.67f},{1.87f,1.42f,.08f},glass);
        box({x,y,-30.61f},{.055f,1.42f,.05f},frame);
        box({x,y-.86f,-30.65f},{2.16f,.12f,.44f},concrete);
    }
    box({0,1.70f,-30.73f},{2.95f,3.0f,.2f},frame);
    for(float x:{-.70f,.70f}) {
        add("cube",{x,1.65f,-30.60f},{1.30f,2.75f,.08f},glass);
        box({x*.24f,1.53f,-30.52f},{.045f,.52f,.06f},{.7f,.75f,.78f});
    }
    box({0,3.37f,-30.f},{4.6f,.22f,1.9f},concrete);
    sign("ACCUEIL",{0,3.38f,-29.02f},0,{.83f,.92f,1.f},.8f);
    for(float x:{-1.65f,1.65f}) {
        Material lamp; lamp.emission={2.7f,2.4f,1.7f}; lamp.lampGroup=1;
        add("cube",{x,3.24f,-29.8f},{.38f,.05f,.65f},lamp,0,false);
        points.push_back({{x,3.10f,-29.8f},{1.9f,1.8f,1.5f}});
    }
}

Scene::Scene() {
    meshes["cube"]=std::make_unique<Mesh>(); meshes["cube"]->setVertices(Mesh::cube());
    meshes["sphere"]=std::make_unique<Mesh>(); meshes["sphere"]->setVertices(Mesh::sphere());
    auto* city=texture("city","assets/commercial/Textures/colormap.png");
    auto* street=texture("roads","assets/roads/Textures/colormap.png");
    for(char c='a';c<='h';++c) { std::string n="building-"+std::string(1,c); load(n,"assets/commercial/"+n+".obj"); }
    for(const auto& n:{"detail-awning","detail-parasol-a"}) load(n,"assets/commercial/"+std::string(n)+".obj");
    for(const auto& n:{"light-curved","road-sign-stop","dumpster"}) load(n,"assets/roads/"+std::string(n)+".obj");
    load("crate","reference/course/models/woodcrate.obj");
    load("barrel","reference/course/models/barrel.obj");
    auto* wood=texture("wood","reference/course/textures/woodcrate_diffuse.jpg");
    auto* barrel=texture("barrel","reference/course/textures/barrel_diffuse.png");

    // A compact two-sided street. All dimensions are in metres.
    box({0,-.32f,0},{100,.4f,130},{.12f,.14f,.15f});
    box({0,-.10f,0},{8.2f,.2f,78},{.09f,.095f,.12f},2);
    for(float side:{-1.f,1.f}) {
        box({side*5.35f,.10f,0},{2.5f,.2f,66},{.42f,.37f,.33f},1);
        box({side*4.16f,.13f,0},{.18f,.26f,66},{.62f,.57f,.49f});
        for(int i=-14;i<=14;i++) box({side*4.14f,.265f,i*2.2f},{.19f,.018f,.025f},{.23f,.25f,.26f});
    }
    for(int i=-8;i<=8;i++) box({0,.008f,i*3.8f},{.10f,.015f,1.6f},{.70f,.55f,.29f});
    for(float z:{24.f,-25.f}) for(int i=-4;i<=4;i++) box({i*.78f,.012f,z},{.42f,.02f,2.1f},{.72f,.68f,.55f});

    const char variants[7]={'a','f','c','b','h','d','g'};
    const char* shops[7]={"CAFE DU SOIR","LIBRAIRIE","ATELIER 07","LE MARCHE","FLEURISTE","CINEMA","LA MAISON"};
    for(int side=-1;side<=1;side+=2) for(int i=0;i<7;i++) {
        float z=20.f-i*7.f, yaw=side<0?90.f:-90.f;
        Material m; m.texture=city; m.surface=3;
        m.color= side<0 ? glm::vec3(1.f,.94f,.87f) : glm::vec3(.84f,.94f,1.f);
        char c=variants[(i+(side>0?3:0))%7];
        add("building-"+std::string(1,c),{side*8.1f,.2f,z},{6.2f,6.2f,6.2f},m,yaw);
        sign(shops[(i+(side>0?2:0))%7],{side*4.72f,2.85f,z},yaw,{.95f,.76f,.43f});
        // A warm glowing band above each shop; independent of the sunlight.
        Material glow; glow.color={.7f,.3f,.08f}; glow.emission={1.7f,.55f,.13f};
        add("cube",{side*4.74f,3.20f,z},{.035f,.035f,2.75f},glow,0,false);
    }
    // Polytech closes the street. Keep the imported low-rise as a side annex.
    Material cityMat; cityMat.texture=city; cityMat.surface=3;
    add("building-e",{15.f,.2f,-37},{8.f,8.f,8.f},cityMat);
    add("detail-awning",{15.f,2.2f,-33.5f},{3,3,3},cityMat);
    polytech();
    // A distant neighbourhood closes the horizon without enlarging the visit.
    for(int i=0;i<13;i++) for(float z:{-55.f,55.f}) {
        float height=5.f+float((i*7)%9);
        box({(i-6)*7.f,height*.5f,z},{6.f,height,8.f},{.17f,.18f,.23f});
    }

    Material roadMat; roadMat.texture=street;
    for(float side:{-1.f,1.f}) for(float z:{18.f,6.f,-6.f,-18.f}) {
        float yaw=side<0?-90.f:90.f;
        add("light-curved",{side*4.65f,.2f,z},{6.5f,6.5f,6.5f},roadMat,yaw);
        glm::vec3 bulb(side*3.38f,4.37f,z);
        Material glow; glow.color={1.f,.7f,.3f}; glow.emission={6.f,3.f,.8f}; glow.lampGroup=1;
        add("cube",bulb,{.35f,.07f,.20f},glow,0,false);
        points.push_back({bulb-glm::vec3(0,.12f,0),{5.8f,2.9f,1.05f}});
    }
    // Four facade spotlights; their cones are independent of point lights.
    for(float side:{-1.f,1.f}) for(float z:{13.f,-13.f}) {
        glm::vec3 p(side*4.4f,.45f,z), aim(side*5.25f,4.f,z);
        spots.push_back({p,glm::normalize(aim-p),side<0?glm::vec3(3.8f,1.7f,.65f):glm::vec3(.8f,2.3f,4.f)});
        box(p,{.35f,.25f,.35f},{.04f,.05f,.07f});
        Material glow; glow.emission=spots.back().color; glow.lampGroup=2;
        add("sphere",p+glm::vec3(0,.15f,0),{.10f,.05f,.10f},glow,0,false);
    }
    // The last pair stands beyond the last facades, clear of both buildings.
    for(float side:{-1.f,1.f}) for(float z:{23.f,9.f,-5.f,-27.f}) tree({side*5.15f,.2f,z},.90f);
    // The backrest is at local +Z: face the seat toward the road on each side.
    for(float z:{12.f,-.5f,-15.f}) bench({-4.85f,.2f,z},-90);
    for(float z:{17.f,2.f,-12.5f}) bench({4.85f,.2f,z},90);
    for(float z:{16.5f,14.1f}) {
        add("detail-parasol-a",{-5.1f,.2f,z},{4.5f,4.5f,4.5f},cityMat);
        box({-5.1f,.95f,z},{1.2f,.10f,.9f},{.32f,.17f,.08f});
        box({-5.1f,.55f,z},{.10f,.8f,.1f},{.06f,.07f,.08f});
    }
    add("road-sign-stop",{4.8f,.2f,26},{4.5f,4.5f,4.5f},roadMat,180);
    add("dumpster",{8.1f,.2f,-28},{5,5,5},roadMat,-90);
    Material wooden; wooden.texture=wood;
    add("crate",{-5.45f,.2f,-10},{.38f,.38f,.38f},wooden,17);
    add("crate",{-5.6f,.2f,-11},{.34f,.34f,.34f},wooden,-8);
    Material barrelMat; barrelMat.texture=barrel;
    add("barrel",{8.1f,.2f,-30.2f},{.34f,.34f,.34f},barrelMat);

    // Festoon bulbs across the street: small emissive meshes plus two real lights.
    for(float z:{10.f,-11.f}) {
        for(int j=0;j<=20;j++) {
            float x=-5.2f+j*.52f, y=5.5f+.025f*x*x;
            box({x,y,z},{.56f,.025f,.025f},{.035f,.04f,.05f});
            if(j%2==0) {
                Material glow; glow.emission={3.5f,1.6f,.4f}; glow.lampGroup=1;
                add("sphere",{x,y-.10f,z},{.07f,.11f,.07f},glow,0,false);
            }
        }
        points.push_back({{0,5.3f,z},{2.0f,.85f,.26f}});
    }
    std::cout<<"Scene ready: "<<importedModels<<" imported OBJ models, "<<objects.size()<<" objects, "
             <<points.size()<<" point lights, "<<spots.size()<<" facade spotlights + camera torch.\n";
}

glm::mat4 Scene::lightMatrix() const {
    return glm::ortho(-43.f,43.f,-43.f,43.f,1.f,125.f)*glm::lookAt(glm::vec3(-13.2f,12.f,-60.f),glm::vec3(0,0,0),glm::vec3(0,1,0));
}
void Scene::lights(ShaderProgram& s,bool sun,bool point,bool spot,bool torch,glm::vec3 eye,glm::vec3 look) const {
    s.setUniform("sunOn",int(sun)); s.setUniform("pointsOn",int(point)); s.setUniform("spotsOn",int(spot));
    s.setUniform("sunDirection",glm::normalize(glm::vec3(13.2f,-12.f,60.f)));
    s.setUniform("pointCount",int(points.size())); s.setUniform("spotCount",int(spots.size())+int(torch));
    for(size_t i=0;i<points.size();i++) {
        auto key="points["+std::to_string(i)+"]";
        s.setUniform((key+".position").c_str(),points[i].position);
        s.setUniform((key+".color").c_str(),points[i].color);
    }
    for(size_t i=0;i<spots.size();i++) {
        auto key="spots["+std::to_string(i)+"]";
        s.setUniform((key+".position").c_str(),spots[i].position);
        s.setUniform((key+".direction").c_str(),spots[i].direction);
        s.setUniform((key+".color").c_str(),spots[i].color);
    }
    s.setUniform("spots[4].position",eye); s.setUniform("spots[4].direction",look); s.setUniform("spots[4].color",glm::vec3(3.f,3.f,2.5f));
}
void Scene::draw(ShaderProgram& s,bool shadowPass) const {
    for(const auto& o:objects) {
        if(shadowPass && !o.castsShadow) continue;
        s.setUniform("model",o.model);
        if(!shadowPass) {
            const auto& m=o.material;
            s.setUniform("tint",m.color); s.setUniform("emission",m.emission);
            s.setUniform("specular",m.specular); s.setUniform("shininess",m.shininess);
            s.setUniform("surface",m.surface); s.setUniform("lampGroup",m.lampGroup);
            s.setUniform("useTexture",int(m.texture!=nullptr));
            if(m.texture) m.texture->bind(0);
        }
        o.mesh->draw();
    }
}
