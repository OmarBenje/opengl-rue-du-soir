#version 330 core
// Extended from lighting_dir_point_spot.frag in the supplied course.
struct PointLight { vec3 position; vec3 color; };
struct SpotLight { vec3 position; vec3 direction; vec3 color; };
uniform PointLight points[12];
uniform SpotLight spots[5];
uniform int pointCount, spotCount, sunOn, pointsOn, spotsOn, useTexture, surface;
uniform int lampGroup;
uniform vec3 viewPos, tint, emission, sunDirection;
uniform float shininess, specular;
uniform sampler2D diffuseMap, shadowMap;
in vec3 FragPos, Normal;
in vec2 TexCoord;
in vec4 ShadowPos;
out vec4 fragColor;

float shadow(vec3 n,vec3 l) {
    vec3 p=ShadowPos.xyz/ShadowPos.w*.5+.5;
    if(p.z>1.0 || p.z<0.0 || p.x<0.0 || p.x>1.0 || p.y<0.0 || p.y>1.0) return 0.0;
    float bias=max(.0012*(1.0-dot(n,l)),.00035), s=0.0;
    vec2 texel=1.0/vec2(textureSize(shadowMap,0));
    for(int x=-1;x<=1;x++) for(int y=-1;y<=1;y++)
        s+=p.z-bias>texture(shadowMap,p.xy+vec2(x,y)*texel).r ? 1.0:0.0;
    return s/9.0;
}
vec3 illuminate(vec3 n,vec3 v,vec3 l,vec3 color,vec3 albedo) {
    float diffuse=max(dot(n,l),0.0);
    float highlight=pow(max(dot(n,normalize(l+v)),0.0),shininess)*specular;
    return color*(albedo*diffuse+highlight*step(.001,diffuse));
}
void main() {
    vec3 n=normalize(Normal), v=normalize(viewPos-FragPos);
    vec3 albedo=tint;
    if(useTexture==1) albedo*=texture(diffuseMap,TexCoord).rgb;
    if(surface==1 && n.y>.5) {
        // Staggered paving joints, evaluated in world space.
        vec2 tile=vec2(FragPos.x*1.7+mod(floor(FragPos.z*1.2),2.0)*.5,FragPos.z*1.2);
        vec2 edge=min(fract(tile),1.0-fract(tile));
        float grout=1.0-smoothstep(.025,.055,min(edge.x,edge.y));
        float grain=fract(sin(dot(floor(tile),vec2(127.1,311.7)))*43758.5453);
        albedo*=mix(.85,1.1,grain)*(1.0-grout*.32);
    }
    if(surface==2) {
        float grain=fract(sin(dot(FragPos.xz*180.0,vec2(12.9898,78.233)))*43758.5453);
        albedo*=.92+.16*grain;
    }
    vec3 sky=mix(vec3(.055,.065,.12),vec3(.16,.20,.34),clamp(n.y*.5+.5,0.0,1.0));
    vec3 color=albedo*sky;
    vec3 l=normalize(-sunDirection);
    if(sunOn==1) color+=illuminate(n,v,l,vec3(.72,.34,.18),albedo)*(1.0-shadow(n,l)*.82);
    if(pointsOn==1) for(int i=0;i<pointCount;i++) {
        vec3 delta=points[i].position-FragPos; float d=length(delta);
        color+=illuminate(n,v,normalize(delta),points[i].color,albedo)/(1.0+.22*d+.14*d*d);
    }
    if(spotsOn==1) for(int i=0;i<spotCount;i++) {
        vec3 delta=spots[i].position-FragPos; float d=length(delta);
        float cone=smoothstep(cos(radians(32.0)),cos(radians(19.0)),dot(normalize(-delta),normalize(spots[i].direction)));
        color+=illuminate(n,v,normalize(delta),spots[i].color,albedo)*cone/(1.0+.08*d+.035*d*d);
    }
    float glow=(lampGroup==1 && pointsOn==0)||(lampGroup==2 && spotsOn==0)?0.0:1.0;
    color+=emission*glow;
    float fog=1.0-exp(-pow(length(viewPos-FragPos)*.011,1.65));
    color=mix(color,vec3(.34,.23,.32),fog*.72);
    fragColor=vec4(color,1.0);
}
