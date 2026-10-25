#version 330 core
in vec2 uv;
uniform sampler2D scene;
uniform vec2 resolution;
uniform int bloomOn;
out vec4 fragColor;
void main(){
    vec3 c=texture(scene,uv).rgb;
    if(bloomOn==1){
        vec3 glow=vec3(0.0); float total=0.0;
        for(int x=-2;x<=2;x++) for(int y=-2;y<=2;y++){
            float w=exp(-float(x*x+y*y)*.35);
            glow+=max(texture(scene,uv+vec2(x,y)*4.0/resolution).rgb-1.0,0.0)*w;
            total+=w;
        }
        c+=glow/total*.28;
    }
    c=vec3(1.0)-exp(-c*1.15);
    c=pow(c,vec3(1.0/2.2));
    float vignette=1.0-.16*dot((uv-.5)*1.35,(uv-.5)*1.35);
    fragColor=vec4(c*vignette,1.0);
}
