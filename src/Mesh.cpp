// Course OBJ loader extended with polygon triangulation, negative indices,
// missing UVs/normals and bounds checks (Kenney exports use polygons).
#include "Mesh.h"
#include <glm/gtc/constants.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <array>
#include <cstddef>

Mesh::~Mesh() { glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); }

void Mesh::setVertices(const std::vector<Vertex>& vertices) {
    if (vertices.empty()) throw std::runtime_error("Empty mesh");
    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);
    count = static_cast<GLsizei>(vertices.size());
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,texCoords));
    for (int i=0;i<3;++i) glEnableVertexAttribArray(i);
    glBindVertexArray(0);
}

bool Mesh::loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Cannot open OBJ: "+filename);
    std::vector<glm::vec3> positions, normals;
    std::vector<glm::vec2> uvs;
    std::vector<Vertex> vertices;
    auto index = [](const std::string& s, size_t size) {
        int i=std::stoi(s);
        i = i>0 ? i-1 : static_cast<int>(size)+i;
        if(i<0 || i>=static_cast<int>(size)) throw std::runtime_error("OBJ index out of range");
        return i;
    };
    for (std::string line; std::getline(file,line);) {
        std::istringstream in(line); std::string cmd; in>>cmd;
        if (cmd=="v" || cmd=="vn") {
            glm::vec3 v; if(!(in>>v.x>>v.y>>v.z)) throw std::runtime_error("Invalid OBJ vector");
            (cmd=="v" ? positions : normals).push_back(v);
        } else if(cmd=="vt") {
            glm::vec2 uv; if(!(in>>uv.x>>uv.y)) throw std::runtime_error("Invalid OBJ UV");
            uvs.push_back(uv);
        } else if(cmd=="f") {
            std::vector<Vertex> face;
            for(std::string token; in>>token;) {
                if(token[0]=='#') break;
                std::array<std::string,3> fields{};
                std::istringstream parts(token);
                for(int i=0;i<3;++i) std::getline(parts,fields[i],'/');
                Vertex v{};
                v.position=positions.at(index(fields[0],positions.size()));
                if(!fields[1].empty()) v.texCoords=uvs.at(index(fields[1],uvs.size()));
                if(!fields[2].empty()) v.normal=normals.at(index(fields[2],normals.size()));
                face.push_back(v);
            }
            for(size_t j=1;j+1<face.size();++j) {
                std::array<Vertex,3> tri{face[0],face[j],face[j+1]};
                glm::vec3 n=glm::cross(tri[1].position-tri[0].position,tri[2].position-tri[0].position);
                if(glm::dot(n,n)<1e-14f) continue;
                n=glm::normalize(n);
                for(auto& v:tri) { if(glm::dot(v.normal,v.normal)<1e-8f) v.normal=n; vertices.push_back(v); }
            }
        }
    }
    if(vertices.empty()) throw std::runtime_error("No triangles in "+filename);
    setVertices(vertices);
    std::cout<<"OBJ "<<filename<<" : "<<vertices.size()/3<<" triangles\n";
    return true;
}

void Mesh::draw() const { glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES,0,count); }

std::vector<Vertex> Mesh::cube() {
    const glm::vec3 p[8]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},
                         {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    const int faces[6][4]={{4,5,6,7},{1,0,3,2},{0,4,7,3},{5,1,2,6},{3,7,6,2},{0,1,5,4}};
    const glm::vec2 uv[4]={{0,0},{1,0},{1,1},{0,1}};
    std::vector<Vertex> out;
    for(auto& f:faces) {
        auto n=glm::normalize(glm::cross(p[f[1]]-p[f[0]],p[f[2]]-p[f[0]]));
        for(int i:{0,1,2,0,2,3}) out.push_back({p[f[i]],n,uv[i]});
    }
    return out;
}

std::vector<Vertex> Mesh::sphere(int slices,int stacks) {
    std::vector<Vertex> out;
    auto v=[&](int x,int y) {
        float u=float(x)/slices, w=float(y)/stacks;
        float a=u*glm::two_pi<float>(), b=w*glm::pi<float>();
        glm::vec3 n(sin(b)*cos(a),cos(b),sin(b)*sin(a));
        return Vertex{n,n,{u,w}};
    };
    for(int y=0;y<stacks;++y) for(int x=0;x<slices;++x)
        for(auto p:{v(x,y),v(x,y+1),v(x+1,y),v(x+1,y),v(x,y+1),v(x+1,y+1)}) out.push_back(p);
    return out;
}
