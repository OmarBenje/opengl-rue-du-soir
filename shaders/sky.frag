#version 330 core
in vec2 uv;
uniform mat4 inverseViewProjection;
uniform vec3 viewPos;
out vec4 fragColor;
void main(){
    vec4 p=inverseViewProjection*vec4(uv*2.0-1.0,1.0,1.0);
    vec3 ray=normalize(p.xyz/p.w-viewPos);
    vec3 horizon=vec3(.90,.40,.28),zenith=vec3(.075,.105,.25);
    vec3 c=mix(horizon,zenith,pow(clamp(ray.y*1.25,0.0,1.0),.55));
    c=mix(c,vec3(.27,.18,.26),1.0-smoothstep(-.3,0.0,ray.y));
    vec3 sun=normalize(vec3(-.22,.20,-1.0));
    float s=max(dot(ray,sun),0.0);
    c+=vec3(.9,.32,.08)*pow(s,32.0)+vec3(4.0,1.8,.5)*smoothstep(.9991,.9995,s);
    float bands=sin(ray.x*8.0+ray.z*7.0+ray.y*40.0)*sin(ray.z*11.0-ray.y*27.0);
    c+=vec3(.16,.10,.13)*smoothstep(.4,.8,bands)*smoothstep(.05,.2,ray.y)*(1.0-smoothstep(.25,.5,ray.y));
    fragColor=vec4(c,1.0);
}
