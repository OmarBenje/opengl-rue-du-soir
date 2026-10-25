#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Tour.h"
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <stb_easy_font.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;

struct Options {
    int width=1440,height=900,frames=0,fps=30;
    float time=0,duration=60;
    bool hidden=false,hud=true,sun=true,points=true,spots=true,bloom=true;
    std::string capture,record;
};
struct State {
    bool automatic=true,paused=false,hud=true,sun=true,points=true,spots=true,bloom=true,torch=false,screenshot=false;
    float time=0;
    double mouseX=0,mouseY=0;
    bool firstMouse=true;
    TourCamera tour;
    FPSCamera free;
    Camera& camera() { return automatic ? static_cast<Camera&>(tour) : static_cast<Camera&>(free); }
};

void key(GLFWwindow* w,int k,int,int action,int) {
    if(action!=GLFW_PRESS) return;
    auto& s=*static_cast<State*>(glfwGetWindowUserPointer(w));
    switch(k) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(w,true); break;
        case GLFW_KEY_SPACE: s.paused=!s.paused; break;
        case GLFW_KEY_H: s.hud=!s.hud; break;
        case GLFW_KEY_1: s.sun=!s.sun; break;
        case GLFW_KEY_2: s.points=!s.points; break;
        case GLFW_KEY_3: s.spots=!s.spots; break;
        case GLFW_KEY_B: s.bloom=!s.bloom; break;
        case GLFW_KEY_F: s.torch=!s.torch; break;
        case GLFW_KEY_P: s.screenshot=true; break;
        case GLFW_KEY_R: s.time=0; s.tour.sample(0); s.paused=false; break;
        case GLFW_KEY_C:
            if(s.automatic) {
                auto look=s.tour.getLook();
                s.free.lookAt(s.tour.getPosition(),s.tour.getPosition()+look);
                s.free.setFOV(s.tour.getFOV());
            }
            s.automatic=!s.automatic; s.firstMouse=true;
            glfwSetInputMode(w,GLFW_CURSOR,s.automatic?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);
            break;
    }
}

void update(GLFWwindow* w,State& s,float dt) {
    if(s.automatic) {
        if(!s.paused) s.time=std::fmod(s.time+dt,TourCamera::duration);
        s.tour.sample(s.time); return;
    }
    if(!glfwGetWindowAttrib(w,GLFW_FOCUSED)) { s.firstMouse=true; return; }
    double x,y; glfwGetCursorPos(w,&x,&y);
    if(!s.firstMouse) s.free.rotate(float(s.mouseX-x)*.10f,float(s.mouseY-y)*.10f);
    s.firstMouse=false; s.mouseX=x; s.mouseY=y;
    float speed=glfwGetKey(w,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS?8.f:3.5f;
    auto down=[&](int key){return glfwGetKey(w,key)==GLFW_PRESS;};
    glm::vec3 move(0);
    if(down(GLFW_KEY_W)||down(GLFW_KEY_Z)||down(GLFW_KEY_UP)) move+=s.free.getLook();
    if(down(GLFW_KEY_S)||down(GLFW_KEY_DOWN)) move-=s.free.getLook();
    if(down(GLFW_KEY_A)||down(GLFW_KEY_Q)||down(GLFW_KEY_LEFT)) move-=s.free.getRight();
    if(down(GLFW_KEY_D)||down(GLFW_KEY_RIGHT)) move+=s.free.getRight();
    if(down(GLFW_KEY_E)) move.y+=1;
    if(down(GLFW_KEY_X)) move.y-=1;
    if(glm::length(move)>.001f) s.free.move(glm::normalize(move)*dt*speed);
}

class RenderTarget {
public:
    GLuint fbo=0,color=0,depth=0;
    int width=0,height=0;
    ~RenderTarget(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&color);glDeleteRenderbuffers(1,&depth);}
    void resize(int w,int h) {
        if(width==w && height==h) return;
        width=w;height=h;
        if(!fbo){glGenFramebuffers(1,&fbo);glGenTextures(1,&color);glGenRenderbuffers(1,&depth);}
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glBindTexture(GL_TEXTURE_2D,color);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,w,h,0,GL_RGBA,GL_FLOAT,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
        glBindRenderbuffer(GL_RENDERBUFFER,depth);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,w,h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("HDR framebuffer incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
};
class ShadowMap {
public:
    GLuint fbo=0,texture=0;
    static constexpr int size=2048;
    ShadowMap(){
        glGenFramebuffers(1,&fbo); glGenTextures(1,&texture);
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,size,size,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
        float border[]={1,1,1,1};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,texture,0);
        glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Shadow framebuffer incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
    ~ShadowMap(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);}
};

class Hud {
    ShaderProgram shader;
    GLuint vao=0,vbo=0;
    void vertices(const std::vector<glm::vec2>& v,glm::vec4 color){
        shader.setUniform("color",color);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(glm::vec2),v.data(),GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(v.size()));
    }
public:
    Hud(){
        std::string root=STREET_ROOT;
        if(!shader.loadShaders((root+"/shaders/hud.vert").c_str(),(root+"/shaders/hud.frag").c_str())) throw std::runtime_error("HUD shader failed");
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(glm::vec2),nullptr);glEnableVertexAttribArray(0);
    }
    ~Hud(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);}
    void rect(float x,float y,float w,float h,glm::vec4 color){vertices({{x,y},{x+w,y},{x+w,y+h},{x,y},{x+w,y+h},{x,y+h}},color);}
    void text(const std::string& t,float x,float y,float scale,glm::vec4 color){
        alignas(float) std::array<unsigned char,40000> buffer{};
        int n=stb_easy_font_print(0,0,const_cast<char*>(t.c_str()),nullptr,buffer.data(),buffer.size());
        std::vector<glm::vec2> v;
        for(int q=0;q<n;q++) for(int i:{0,1,2,0,2,3}) {
            const float* p=reinterpret_cast<const float*>(buffer.data()+(q*4+i)*16);
            v.push_back({x+p[0]*scale,y+p[1]*scale});
        }
        vertices(v,color);
    }
    void draw(const State& s,int w,int h){
        shader.use();shader.setUniform("resolution",glm::vec2(w,h));
        glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        const glm::vec4 cream(1.f,.92f,.78f,1),muted(.78f,.79f,.81f,1);
        rect(28,28,340,90,{.025f,.032f,.055f,.72f});
        rect(28,28,3,90,{.96f,.62f,.29f,1});
        text("RUE DU SOIR",46,46,3.3f,cream);
        text("UNE PROMENADE AU CREPUSCULE",46,88,1.35f,muted);
        float x=float(w)-230;
        rect(x,28,202,94,{.025f,.032f,.055f,.72f});
        int row=0;
        for(auto pair:{std::pair<const char*,bool>{"1  SOLEIL",s.sun},{"2  LAMPADAIRES",s.points},{"3  SPOTS",s.spots}}){
            rect(x+14,43+row*24,5,5,pair.second?glm::vec4(1,.65f,.28f,1):glm::vec4(.3f,.3f,.35f,1));
            text(pair.first,x+28,39+row*24,1.5f,pair.second?cream:muted);++row;
        }
        rect(28,h-91,w-56,63,{.025f,.032f,.055f,.78f});
        std::ostringstream label;
        if(s.automatic) label<<(s.paused?"VISITE EN PAUSE":"VISITE GUIDEE")<<"   00:"<<std::setw(2)<<std::setfill('0')<<int(s.time)%60<<" / 01:00";
        else label<<"EXPLORATION LIBRE   ZQSD / WASD + SOURIS   E/X HAUTEUR";
        text(label.str(),44,h-78,1.6f,cream);
        text("ESPACE pause   C camera   R recommencer   H interface   P photo   ECHAP quitter",44,h-51,1.35f,muted);
        rect(28,h-28,w-56,2,{.24f,.25f,.29f,1});
        rect(28,h-28,(w-56)*s.time/TourCamera::duration,2,{1,.64f,.26f,1});
        glDisable(GL_BLEND);
    }
};

void saveFrame(const fs::path& path,int w,int h) {
    if(!path.parent_path().empty()) fs::create_directories(path.parent_path());
    std::vector<unsigned char> pixels(static_cast<size_t>(w)*h*3);
    glPixelStorei(GL_PACK_ALIGNMENT,1);glReadBuffer(GL_BACK);
    glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    stbi_flip_vertically_on_write(1);
    if(!stbi_write_png(path.string().c_str(),w,h,3,pixels.data(),w*3)) throw std::runtime_error("Cannot save "+path.string());
}

void streamFrame(int w,int h) {
    std::vector<unsigned char> pixels(static_cast<size_t>(w)*h*3);
    glPixelStorei(GL_PACK_ALIGNMENT,1);glReadBuffer(GL_BACK);
    glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    // FFmpeg reads top-down RGB24; OpenGL returns bottom-up rows.
    for(int y=h-1;y>=0;y--)
        if(std::fwrite(pixels.data()+static_cast<size_t>(y)*w*3,3,w,stdout)!=static_cast<size_t>(w))
            throw std::runtime_error("Video encoder closed the stream");
}

int run(GLFWwindow* window,const Options& o) {
    State s;s.hud=o.hud;s.sun=o.sun;s.points=o.points;s.spots=o.spots;s.bloom=o.bloom;s.time=o.time;s.tour.sample(s.time);
    glfwSetWindowUserPointer(window,&s);glfwSetKeyCallback(window,key);
    glfwSetScrollCallback(window,[](GLFWwindow* w,double,double dy){
        auto& state=*static_cast<State*>(glfwGetWindowUserPointer(w));
        auto& c=state.camera();c.setFOV(glm::clamp(c.getFOV()-float(dy)*2.f,25.f,85.f));
    });
    Scene scene; RenderTarget target; ShadowMap shadow; Hud hud;
    ShaderProgram lighting,depth,sky,post;
    auto load=[](ShaderProgram& p,const char* v,const char* f){
        std::string root=STREET_ROOT;
        if(!p.loadShaders((root+"/shaders/"+v).c_str(),(root+"/shaders/"+f).c_str())) throw std::runtime_error("Shader failed: "+std::string(f));
    };
    load(lighting,"street.vert","street.frag");load(depth,"depth.vert","depth.frag");
    load(sky,"screen.vert","sky.frag");load(post,"screen.vert","post.frag");
    GLuint screenVAO;glGenVertexArrays(1,&screenVAO);
    // The scene is static, so its solar shadow map needs only one render.
    glEnable(GL_DEPTH_TEST);glBindFramebuffer(GL_FRAMEBUFFER,shadow.fbo);
    glViewport(0,0,ShadowMap::size,ShadowMap::size);glClear(GL_DEPTH_BUFFER_BIT);
    depth.use();depth.setUniform("lightSpace",scene.lightMatrix());scene.draw(depth,true);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    double previous=glfwGetTime(),start=previous,titleTime=previous;
    int frame=0,titleFrames=0;
    int limit=o.frames;
    if(!o.capture.empty()) limit=1;
    if(!o.record.empty()) { limit=int(std::ceil(o.duration*o.fps));if(o.record!="-")fs::create_directories(o.record); }
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();double now=glfwGetTime();
        float dt=std::min(float(now-previous),.1f);previous=now;
        if(!o.record.empty() || !o.capture.empty()) {s.time=std::fmod(o.time+float(frame)/o.fps,TourCamera::duration);s.tour.sample(s.time);}
        else update(window,s,dt);
        int w,h;glfwGetFramebufferSize(window,&w,&h);
        if(w<=0 || h<=0){glfwWaitEventsTimeout(.1);continue;}
        target.resize(w,h);glBindFramebuffer(GL_FRAMEBUFFER,target.fbo);glViewport(0,0,w,h);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        auto& camera=s.camera();auto view=camera.getViewMatrix();
        auto projection=glm::perspective(glm::radians(camera.getFOV()),float(w)/h,.1f,180.f);
        glDisable(GL_DEPTH_TEST);sky.use();sky.setUniform("inverseViewProjection",glm::inverse(projection*view));
        sky.setUniform("viewPos",camera.getPosition());glBindVertexArray(screenVAO);glDrawArrays(GL_TRIANGLES,0,3);
        glEnable(GL_DEPTH_TEST);lighting.use();
        lighting.setUniform("view",view);lighting.setUniform("projection",projection);lighting.setUniform("viewPos",camera.getPosition());
        lighting.setUniform("lightSpace",scene.lightMatrix());lighting.setUniform("diffuseMap",0);lighting.setUniform("shadowMap",1);
        glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,shadow.texture);
        scene.lights(lighting,s.sun,s.points,s.spots,s.torch,camera.getPosition(),camera.getLook());scene.draw(lighting);
        glBindFramebuffer(GL_FRAMEBUFFER,0);glDisable(GL_DEPTH_TEST);
        post.use();post.setUniform("scene",0);post.setUniform("resolution",glm::vec2(w,h));post.setUniform("bloomOn",int(s.bloom));
        glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,target.color);
        glBindVertexArray(screenVAO);glDrawArrays(GL_TRIANGLES,0,3);
        if(s.hud) hud.draw(s,w,h);
        if(!o.capture.empty()) saveFrame(o.capture,w,h);
        if(!o.record.empty()) {
            std::ostringstream name;name<<"frame-"<<std::setfill('0')<<std::setw(5)<<frame<<".png";
            if(o.record=="-") streamFrame(w,h); else saveFrame(fs::path(o.record)/name.str(),w,h);
            if(frame%o.fps==0) std::cout<<"Recording "<<frame/o.fps<<" / "<<o.duration<<" s\n"<<std::flush;
        }
        if(s.screenshot) {
            auto name=fs::path(STREET_ROOT)/"captures"/("photo-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+".png");
            saveFrame(name,w,h);std::cout<<"Saved "<<name<<'\n';s.screenshot=false;
        }
        GLenum error=glGetError();
        if(error!=GL_NO_ERROR) throw std::runtime_error("OpenGL error "+std::to_string(error));
        glfwSwapBuffers(window);++frame;++titleFrames;
        if(now-titleTime>1.0) {
            std::ostringstream t;t<<"Rue du Soir | "<<(s.automatic?"Visite guidee":"Camera libre")<<" | "<<int(titleFrames/(now-titleTime))<<" fps";
            glfwSetWindowTitle(window,t.str().c_str());titleTime=now;titleFrames=0;
        }
        if(limit>0 && frame>=limit) break;
    }
    std::cout<<"Rendered "<<frame<<" frames in "<<glfwGetTime()-start<<" s; OpenGL errors: 0\n";
    glDeleteVertexArrays(1,&screenVAO);glfwSetWindowUserPointer(window,nullptr);
    return 0;
}

int main(int argc,char** argv) {
    Options o;
    try {
        for(int i=1;i<argc;i++) {
            std::string a=argv[i];
            auto value=[&](){if(i+1>=argc)throw std::runtime_error("Missing value for "+a);return std::string(argv[++i]);};
            if(a=="--test-tour") return TourCamera::verify()?0:1;
            if(a=="--capture") o.capture=value();else if(a=="--record") o.record=value();
            else if(a=="--time") o.time=std::stof(value());else if(a=="--duration") o.duration=std::stof(value());
            else if(a=="--width") o.width=std::stoi(value());else if(a=="--height") o.height=std::stoi(value());
            else if(a=="--frames") o.frames=std::stoi(value());else if(a=="--fps") o.fps=std::stoi(value());
            else if(a=="--hidden") o.hidden=true;else if(a=="--no-hud") o.hud=false;
            else if(a=="--no-sun") o.sun=false;else if(a=="--no-points") o.points=false;
            else if(a=="--no-spots") o.spots=false;else if(a=="--no-bloom") o.bloom=false;
            else if(a=="--help") {
                std::cout<<"Rue du Soir\nC camera libre, ESPACE pause, R recommencer, 1/2/3 lumieres, F torche, B bloom, H interface, P photo.\n"
                           "--capture image.png --time 15 --no-hud --hidden\n--record dossier --duration 60 --fps 30\n"
                           "--width 1280 --height 720 --frames 180 --no-sun --no-points --no-spots --no-bloom\n--test-tour\n";
                return 0;
            } else throw std::runtime_error("Unknown argument: "+a);
        }
        if(o.width<320||o.height<240||o.width>3840||o.height>2160||o.fps<1||o.fps>120||o.duration<=0||o.duration>600||!std::isfinite(o.time)||!std::isfinite(o.duration)||o.frames<0)
            throw std::runtime_error("Invalid capture dimensions, duration or frame rate");
        if(!o.capture.empty()&&!o.record.empty()) throw std::runtime_error("Choose --capture or --record");
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
    // Keep the raw frame stream free of diagnostics.
    if(o.record=="-") std::cout.rdbuf(std::cerr.rdbuf());
    glfwSetErrorCallback([](int code,const char* msg){std::cerr<<"GLFW "<<code<<": "<<msg<<'\n';});
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE,o.hidden?GLFW_FALSE:GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER,GLFW_FALSE);
#endif
    GLFWwindow* window=glfwCreateWindow(o.width,o.height,"Rue du Soir",nullptr,nullptr);
    if(!window){glfwTerminate();return 1;}
    glfwMakeContextCurrent(window);glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK){glfwDestroyWindow(window);glfwTerminate();return 1;}
    while(glGetError()!=GL_NO_ERROR){} // GLEW probes unsupported extensions in core contexts.
    std::cout<<"OpenGL "<<glGetString(GL_VERSION)<<" | "<<glGetString(GL_RENDERER)<<'\n';
    glfwSwapInterval(o.hidden||!o.record.empty()||!o.capture.empty()?0:1);
    int result=1;
    try{result=run(window,o);}catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';}
    // All GPU owners in run() are destroyed before the OpenGL context.
    glfwDestroyWindow(window);glfwTerminate();return result;
}
