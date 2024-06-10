#version 330 core

out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse; 
uniform sampler2D texture1;
uniform float fov; // Uniform pour le champ de vision
uniform vec3 objectPosition; // Uniform pour la position de l'objet
uniform float objectRotationX; // Uniform pour la rotation de l'objet autour de X
uniform float objectRotationY; // Uniform pour la rotation de l'objet autour de Y
uniform float objectRotationZ; // Uniform pour la rotation de l'objet autour de Z

#define MAX_DIST 20.0
#define STEPS 100
#define PI 3.141592
#define DEG2RAD 0.01745329251

vec3 translate(vec3 p, vec3 t) {
    return p - t;
}

vec3 rotateX(vec3 p, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat3 rot = mat3(1.0, 0.0, 0.0,
                    0.0, c, -s,
                    0.0, s, c);
    return rot * p;
}

vec3 rotateY(vec3 p, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat3 rot = mat3(c, 0.0, s,
                    0.0, 1.0, 0.0,
                    -s, 0.0, c);
    return rot * p;
}

vec3 rotateZ(vec3 p, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat3 rot = mat3(c, -s, 0.0,
                    s, c, 0.0,
                    0.0, 0.0, 1.0);
    return rot * p;
}

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
    float dE = length(max(diff, 0.0));
    float dI = min(max(diff.x, max(diff.y, diff.z)), 0.0);
    float d = dE + dI;
    return vec2(i, d);
}

vec2 minVec2(vec2 a, vec2 b) {
    return a.y < b.y ? a : b;
}

vec2 scene(vec3 p) {
    // Transformation de box2 et du cylindre
    vec3 pBox2 = translate(p, objectPosition); // Utiliser la position de l'objet
    vec3 pCylinder = translate(p, vec3(0.3, 1.2, 0));

    // Rotation appliquée uniquement à box2 et au cylindre
    pBox2 = rotateX(pBox2, objectRotationX); // Utiliser la rotation de l'objet autour de X
    pBox2 = rotateY(pBox2, objectRotationY); // Utiliser la rotation de l'objet autour de Y
    pBox2 = rotateZ(pBox2, objectRotationZ); // Utiliser la rotation de l'objet autour de Z
    pCylinder = rotateX(pCylinder, iTime * 0.3);

    // Mouvement elliptique pour sphere2
    vec3 pSphere2 = p - vec3(0.0, 0.5, -0.5);
    pSphere2.x += 0.1 * cos(iTime); // Mouvement sur l'axe X
    pSphere2.y += 0.1 * sin(iTime); // Mouvement sur l'axe Y

    // Définition des objets avec transformations appliquées
    vec2 dp = dPlane(p, 0.0, 0.0);
    vec2 ds = dSphere(p - vec3(0.0, 0.0, 0.0), 0.5, 1.0);
    vec2 ds2 = dSphere(pSphere2, 0.3, 5.0);
    vec2 dT = dTorus(p, 1.0, 0.2, 3.0);
    vec2 dC = dCylinder(pCylinder, 0.3, 0.2, 4.0);
    vec2 dB = dBox(p - vec3(0.8, 0.5, 0.3), vec3(0.3, 0.1, 0.3), 2.0);
    vec2 dMarbleBox = dBox(pBox2, vec3(0.3, 0.3, 0.05), 6.0);
    
    dB.y -= 0.1;
    dC.y -= 0.05;

    return minVec2(dMarbleBox, minVec2(dB, minVec2(dC, minVec2(dT, minVec2(dp, minVec2(ds, ds2))))));
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

float basicLighting(vec3 p, vec3 n) {
    vec3 lP = vec3(cos(iTime) * 2.0, 1.0, sin(iTime) * 2.0);
    vec3 lD = lP - p;
    vec3 lN = normalize(lD);

    if (march(p + n * 0.01, lN).y < length(lD)) {
        return 0.0;
    }

    return max(0.0, dot(n, lN));
}

vec3 phongLighting(vec3 p, vec3 n, vec3 viewDir, vec3 materialColor) {
    vec3 lightPos = vec3(cos(iTime) * 2.0, 1.0, sin(iTime) * 2.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 ambient = 0.1 * materialColor * lightColor;

    vec3 lightDir = normalize(lightPos - p);
    float diff = max(dot(n, lightDir), 0.0);
    vec3 diffuse = diff * materialColor * lightColor;

    vec3 reflectDir = reflect(-lightDir, n);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor; // Specular component is usually white

    return (ambient + diffuse + specular);
}

vec3 blinnPhongLighting(vec3 p, vec3 n, vec3 viewDir, vec3 materialColor) {
    vec3 lightPos = vec3(cos(iTime) * 2.0, 1.0, sin(iTime) * 2.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 ambient = 0.1 * materialColor * lightColor;

    vec3 lightDir = normalize(lightPos - p);
    float diff = max(dot(n, lightDir), 0.0);
    vec3 diffuse = diff * materialColor * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(n, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor; // Specular component is usually white

    return (ambient + diffuse + specular);
}

vec3 toonLighting(vec3 p, vec3 n, vec3 viewDir, vec3 materialColor) {
    vec3 lightPos = vec3(cos(iTime) * 2.0, 1.0, sin(iTime) * 2.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec3 lightDir = normalize(lightPos - p);
    float diff = max(dot(n, lightDir), 0.0);
    
    // Toon shading: discrete levels
    if (diff > 0.5) {
        diff = 1.0;
    } else if (diff > 0.25) {
        diff = 0.7;
    } else {
        diff = 0.4;
    }

    vec3 diffuse = diff * materialColor * lightColor;
    vec3 ambient = 0.1 * materialColor * lightColor;

    return ambient + diffuse;
}

vec3 marbleShader(vec3 p) {
    float noise = sin(p.x * 10.0 + sin(p.y * 10.0 + iTime) * 0.5);
    noise = noise * 0.5 + 0.5; // Normaliser le bruit pour qu'il soit entre 0 et 1
    vec3 color = mix(vec3(1.0, 1.0, 1.0), vec3(0.1, 0.1, 0.1), noise);
    return color;
}

vec3 material(float i) {
    vec3 col = vec3(0.0, 0.0, 0.0);

    if (i < 0.5) {
        col = vec3(1, 2, 2);
    } else if (i < 1.5) {
        col = vec3(1.0, 0.2, 0.3);
    } else if (i < 2.5) {
        col = vec3(0.3, 0.2, 5.0);
    }
    else if (i < 3.5) {
        col = vec3(0.5, 0.2, 3.0);
    }
    else if (i < 4.5) {
        col = vec3(0.3, 5.0, 5.0);
    }
    else if (i < 5.5) {
        col = vec3(0.7, 0.7, 0.7); // Color for the second sphere
    }
    else if (i < 6.5) {
        col = vec3(0.9, 0.9, 0.9); // Color for the glass box
    }

    return col * vec3(0.2);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord - (iResolution.xy * 0.5)) / iResolution.y;

    // Utilisez les coordonnées de la souris ici
    vec2 mouse = iMouse / iResolution;

    float initA = -DEG2RAD * 90.0;

    vec3 r0 = vec3(
        cos(mouse.x * 2.0 * PI + initA) * 2.0,
        mouse.y + 0.5,
        sin(mouse.x * 2.0 * PI + initA) * 2.0
    );

    vec3 target = vec3(0, 0.5, 0);

    vec3 fwd = normalize(target - r0);
    vec3 side = normalize(cross(vec3(0, 1.0, 0), fwd));
    vec3 up = cross(fwd, side);

    vec3 rD = normalize(tan(fov * 0.5) * fwd + side * uv.x + up * uv.y);

    vec2 s = march(r0, rD);
    float d = s.y;

    vec3 sCol = vec3(0.5, 0.8, 1.0);
    vec3 col = mix(vec3(0.5, 0.8, 1.0), vec3(0.08, 0.3, 1.0), pow(uv.y + 0.5, 2.5));

    if (d < MAX_DIST) {
        col = material(s.x);
        vec3 p = r0 + rD * d;
        vec3 nor = normal(p);
        
        if (s.x == 1.0) {
            vec3 viewDir = normalize(-rD);
            col = toonLighting(p, nor, viewDir, col);
        } else if (s.x == 4.0) {
            vec3 viewDir = normalize(-rD);
            col = phongLighting(p, nor, viewDir, col);
        } else if (s.x == 5.0) {
            vec3 viewDir = normalize(-rD);
            col = blinnPhongLighting(p, nor, viewDir, col);
        } else if (s.x == 2.0) {
            // Lighting with texture for the box
            vec3 viewDir = normalize(-rD);
            vec3 lightPos = vec3(cos(iTime) * 2.0, 1.0, sin(iTime) * 2.0);
            vec3 lightColor = vec3(1.0, 1.0, 1.0);
            vec3 lightDir = normalize(lightPos - p);

            float diff = max(dot(nor, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;

            vec3 reflectDir = reflect(-lightDir, nor);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            vec3 specular = spec * lightColor; // Specular component is usually white

            vec3 lighting = (0.1 * lightColor) + diffuse + specular;

            vec2 texCoords;
            if (abs(nor.y) > 0.99) {
                texCoords = vec2(mod(p.x, 1.0), mod(p.z, 1.0)); // Top and bottom faces
            } else {
                texCoords = vec2(mod(p.x + p.z, 1.0), mod(p.y, 1.0)); // Side faces
            }
            col = texture(texture1, texCoords).rgb * lighting;
        } else if (s.x == 6.0) {
            col = marbleShader(p); // Appliquer le shader de marbre à la box2
        } else {
            float l = basicLighting(p, nor);
            vec3 a = vec3(5.0, 0.0, 10.0) * 0.03;
            vec3 aS = (nor.y * sCol) * 0.2;
            col = col * (a + l + aS);
        }
    }

    // Post-traitement : vignette
    float dist = length(uv);
    col *= smoothstep(0.8, 0.2, dist);

    // Post-traitement : correction gamma
    col = pow(col, vec3(1.0 / 2.2));

    fragColor = vec4(col.rgb, 1.0);
}

void main() {
    mainImage(FragColor, gl_FragCoord.xy);
}
