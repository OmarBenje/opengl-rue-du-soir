#pragma once
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include <memory>
#include <map>

struct Material {
    glm::vec3 color{1.f}, emission{0.f};
    Texture2D* texture=nullptr;
    float specular=.1f, shininess=32.f;
    int surface=0, lampGroup=0;
};
struct Object { Mesh* mesh; glm::mat4 model; Material material; bool castsShadow=true; };
struct PointLight { glm::vec3 position,color; };
struct SpotLight { glm::vec3 position,direction,color; };

class Scene {
public:
    Scene();
    void draw(ShaderProgram& shader,bool shadowPass=false) const;
    void lights(ShaderProgram& shader,bool sun,bool point,bool spot,bool torch,
                glm::vec3 eye,glm::vec3 look) const;
    size_t objectCount() const { return objects.size(); }
    size_t modelCount() const { return importedModels; }
    glm::mat4 lightMatrix() const;
private:
    std::map<std::string,std::unique_ptr<Mesh>> meshes;
    std::map<std::string,std::unique_ptr<Texture2D>> textures;
    std::vector<Object> objects;
    std::vector<PointLight> points;
    std::vector<SpotLight> spots;
    size_t importedModels=0;
    Mesh* load(const std::string& name,const std::string& path);
    Texture2D* texture(const std::string& name,const std::string& path);
    void add(const std::string& mesh,glm::vec3 p,glm::vec3 scale,Material material,float yaw=0.f,bool shadow=true);
    void box(glm::vec3 p,glm::vec3 scale,glm::vec3 color,int surface=0);
    void tree(glm::vec3 p,float scale);
    void bench(glm::vec3 p,float yaw);
    void polytech();
    void sign(const std::string& text,glm::vec3 p,float yaw,glm::vec3 ink,float size=1.f,bool backing=true);
};
