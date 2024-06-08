#version 330 core

out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse; 

#define MAX_DIST 20.0
#define STEPS 100
#define PI 3.141592
#define DEG2RAD 0.01745329251

vec2 dPlane(vec3 p, float h, float i) {
    return vec2(i, p.y - h);
}

vec2 dPlane1(vec3 p, vec3 n, float h, float i) {
    return vec2(i, dot(p, n) - h);
}

vec2 dTorus(vec3 p, float r, float t, float i) {
    return vec2(i, length(vec2(length(p.xz) - r, p.y)) - t);
}

vec2 dSphere(vec3 p, float r, float i) {
    return vec2(i, length(p) - r);
}

vec2 dCylinder(vec3 p, float r, float h, float i) {
    float dX = length(p.xz) - r;
    float dY = abs(p.y) - h;

    float dE = length(vec2(max(dX, 0.0), max(dY, 0.0)));
    float dI = min(max(dX, dY), 0.0);

    float d = dE + dI;

    return vec2(i, d);
}

vec2 dBox(vec3 p, vec3 s, float i) {
    vec3 diff = abs(p) - s;
    float dE = length(max(diff,0.0));
    float dI = min(max(diff.x,max(diff.y,diff.z)),0.0);
    float d = dE + dI;
    return vec2(i,d);
}

vec2 minVec2(vec2 a, vec2 b) {
    return a.y < b.y ? a : b;
}

vec2 scene(vec3 p) {
    vec2 dp = dPlane(p, 0.0, 0.0);
    vec2 ds = dSphere(p - vec3(0.0, 0.0, 0.0), 0.5, 1.0);
    vec2 ds2 = dSphere(p - vec3(0.0, 0.5, -0.5), 0.3, 2.0);
    vec2 dT = dTorus(p, 1.0, 0.2, 0.0);
    vec2 dC = dCylinder(p - vec3(0.3, 1, 0), 0.3, 0.2, 0.0);
    vec2 dB = dBox(p - vec3(0.8, 0.5, 0.3), vec3(0.3, 0.1, 0.3), 2.0);
    
    dB.y -= 0.1;
    
    dC -= 0.05;

    return minVec2(dB, minVec2(dC, minVec2(dT, minVec2(dp, minVec2(ds, ds2)))));
}

vec2 march(vec3 r0, vec3 rD) {
    vec3 cP = r0;
    float d = 0.0;
    vec2 s = vec2(0.0);

    for (int i = 0; i < STEPS; i++) {
        cP = r0 + rD * d;
        s = scene(cP);
        d += s.y;

        if (s.y < 0.001) {
            break;
        }

        if (d > MAX_DIST) {
            return vec2(100.0, MAX_DIST + 10.0);
        }
    }

    s.y = d;
    return s;
}

vec3 normal(vec3 p) {
    float dp = scene(p).y;
    
    vec2 eps = vec2(0.01, 0.0);
    
    float dx = scene(p + eps.xyy).y - dp;
    float dy = scene(p + eps.yxy).y - dp;
    float dz = scene(p + eps.yyx).y - dp;
    
    return normalize(vec3(dx, dy, dz));
}

float lighting(vec3 p, vec3 n) {
    vec3 lP = vec3(cos(iTime) * 2.0, 1.0, sin(iTime) * 2.0);
    vec3 lD = lP - p;
    vec3 lN = normalize(lD);

    if (march(p + n * 0.01, lN).y < length(lD)) {
        return 0.0;
    }

    return max(0.0, dot(n, lN));
}

vec3 material(float i) {
    vec3 col = vec3(0.0, 0.0, 0.0);

    if (i < 0.5) {
        col = vec3(1,2,2);
    } else if (i < 1.5) {
        col = vec3(1.0, 0.2, 0.3);
    } else if (i < 2.5) {
        col = vec3(0.3, 0.2, 5.0);
    }

    return col * vec3(0.2);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord - (iResolution.xy * 0.5)) / iResolution.y;

    // Utilisez les coordonnées de la souris ici
    vec2 mouse = iMouse / iResolution;

    float initA = -DEG2RAD * 90.0;
    float fov = 1.0;

    vec3 r0 = vec3(
        cos(mouse.x * 2.0 * PI + initA) * 2.0,
        mouse.y + 0.5,
        sin(mouse.x * 2.0 * PI + initA) * 2.0
    );

    vec3 target = vec3(0, 0.5, 0);

    vec3 fwd = normalize(target - r0);
    vec3 side = normalize(cross(vec3(0, 1.0, 0), fwd));
    vec3 up = cross(fwd, side);

    vec3 rD = normalize(fov * fwd + side * uv.x + up * uv.y);

    vec2 s = march(r0, rD);
    float d = s.y;

    vec3 sCol = vec3(0.5, 0.8, 1.0);
    vec3 col = mix(vec3(0.5, 0.8, 1.0), vec3(0.08, 0.3, 1.0), pow(uv.y + 0.5, 2.5));

    if(d < MAX_DIST) {
        col = material(s.x);
        vec3 p = r0 + rD * d;
        vec3 nor = normal(p);
        float l = lighting(p, nor);
        vec3 a = vec3(5.0, 0.0, 10.0) * 0.03;
        vec3 aS = (nor.y * sCol) * 0.2;
        col = col * (a + l + aS);
    }

    col = pow(col, vec3(0.4545));
    fragColor = vec4(col.rgb, 1.0);
}

void main() {
    mainImage(FragColor, gl_FragCoord.xy);
}
