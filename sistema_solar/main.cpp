#include <windows.h>
#include <GL/freeglut.h>

#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>
#include <random>

#define WIDTH  1900
#define HEIGHT 1200

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// Teclado
float camX = 0.0f, camY = 5.0f, camZ = 0.0f;
float pitch = 0.0f, yaw = -90.0f;
float dirX = 0.0f, dirY = 0.0f, dirZ = -1.0f;
float lastX = WIDTH / 2, lastY = HEIGHT / 2;
bool firstMouse = true;
bool keyStates[256] = { false };

bool mousePressed = false;
float rotX = 0.0f, rotY = 0.0;
GLuint texturaLunaID = 0;

namespace stb {
    struct Centro {
        float x, y, z;
    };

    struct Escala {
        float factor = 0.0001f;
    };
}

struct Planeta {
    std::string nombre;
    float radioOriginal;
    float radioEscalado;
    stb::Centro centro;
};

struct TexturaPlaneta {
    GLuint id;
    const char* archivo;
};

struct Vec3 {
    float x, y, z;
};

struct Luna {
    float anguloOrbita;
    float distancia;
    float velocidad;
    float radio;
    float color[3];
    Vec3 posicion;
};


void updateCameraDirection() {
    float radYaw = yaw * 3.14159f / 180.0f;
    float radPitch = pitch * 3.14159f / 180.0f;

    dirX = cosf(radPitch) * cosf(radYaw);
    dirY = sinf(radPitch);
    dirZ = cosf(radPitch) * sinf(radYaw);

    float len = sqrtf(dirX * dirX + dirY * dirY + dirZ * dirZ);
    dirX /= len; dirY /= len; dirZ /= len;
}


// -------------------- mouseMotion --------------------
void mouseMotion(int x, int y) {
glutSetCursor(GLUT_CURSOR_NONE);

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }
    float sensitivity = 0.5f;
    float offsetX = (x - lastX) * sensitivity;
    float offsetY = (lastY - y) * sensitivity;

    lastX = x;
    lastY = y;

    yaw += offsetX;   // movimiento horizontal
    pitch += offsetY; // movimiento vertical

    // ❗ Permitir hasta 90° arriba y abajo
    if (pitch > 89.9f) pitch = 89.9f;
    if (pitch < -89.9f) pitch = -89.9f;

    updateCameraDirection();
}

stb::Escala escala;

std::vector<Planeta> planetas = {
    {"Sol",      30950 ,  0.0f, {  0.0f, 0.0f, 0.0f}},
    {"Mercurio", 2440  ,  0.0f, { 10.0f, 0.0f, 0.0f}},
    {"Venus",    2052  ,  0.0f, { 14.0f, 0.0f, 0.0f}},
    {"Tierra",   2371  ,  0.0f, { 18.0f, 0.0f, 0.0f}},
    {"Luna",      637  ,  0.0f, { 20.0f, 0.0f, 0.0f}},
    {"Marte",    2790  ,  0.0f, { 27.0f, 0.0f, 0.0f}},
    {"Júpiter", 18991  ,  0.0f, { 36.0f, 0.0f, 0.0f}},
    {"Saturno",  6823  ,  0.0f, { 44.0f, 0.0f, 0.0f}},
    {"Urano",    5536  ,  0.0f, { 48.0f, 0.0f, 0.0f}},
    {"Neptuno",  5262  ,  0.0f, { 54.0f, 0.0f, 0.0f}},

};

std::vector<float> angulosOrbita;
std::vector<float> angulosRotacion;


enum ModoCamara {LIBRE, SEGUIMIENTO};
    ModoCamara modoCamara = LIBRE;

    int planetaSeguido = -1;
    float camX_original, camY_original, camZ_original;


// --------------- MOVIMIENTO DE CAMARA -------------------
float cameraSpeed = 0.12f;

float theta = 90.0f;
float distanciaOrbital = 2.0f;

void CamaraMovimiento() {
    static int lastTime = glutGet(GLUT_ELAPSED_TIME);
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    float rightX = -dirZ, rightZ = dirX;
    float len = sqrtf(rightX * rightX + rightZ * rightZ);
    rightX /= len; rightZ /= len;


    if (modoCamara == SEGUIMIENTO && planetaSeguido >= 0) {
        auto& planeta = planetas[planetaSeguido];

        // Controlar el ángulo con A y D
        if (keyStates['a']) theta += 60.0f * deltaTime;
        if (keyStates['d']) theta -= 60.0f * deltaTime;

        // Controlar la distancia con W y S
        if (keyStates['w']) distanciaOrbital -= 1.1f * deltaTime;
        if (keyStates['s']) distanciaOrbital += 1.1f * deltaTime;

        // Limitar distancia
        if (distanciaOrbital < 0.01) distanciaOrbital = 0.01f;
        if (distanciaOrbital > 5.0f) distanciaOrbital = 5.0f;

        // Convertir ángulo a radianes
        float rad = theta * (3.14159265f / 180.0f);

        // Posición de la cámara
        camX = planeta.centro.x + cos(rad) * distanciaOrbital;
        camY = planeta.centro.y + 0.3f;
        camZ = planeta.centro.z + sin(rad) * distanciaOrbital;

        // Dirección hacia el planeta
        dirX = planeta.centro.x - camX;
        dirY = planeta.centro.y - camY;
        dirZ = planeta.centro.z - camZ;
        float lenDir = sqrtf(dirX*dirX + dirY*dirY + dirZ*dirZ);
        if (lenDir < 0.001f) lenDir = 1.0f;
        dirX /= lenDir;
        dirY /= lenDir;
        dirZ /= lenDir;
    }
    if (modoCamara == LIBRE) {
        // Detectar si se está moviendo
        bool moving = keyStates['w'] || keyStates['a'] || keyStates['s'] || keyStates['d'] || keyStates['q'] || keyStates[' '];

        // Aceleración solo si se mueve y Ctrl izquierdo está presionado
        float speed = cameraSpeed;
        if (moving && (keyStates[17]))
            speed *= 2.0f;

        // Límites
        const float LIM_INF = -60.0f;
        const float LIM_SUP =  60.0f;
        const float LIM_DOWN = -60.0f;

        // Movimiento Adelante (W)
        float newX = camX + dirX * speed;
        float newY = camY + dirY * speed;
        float newZ = camZ + dirZ * speed;
        if (keyStates['w'] && newX >= LIM_INF && newX <= LIM_SUP && newY >= LIM_DOWN && newY <= LIM_SUP && newZ >= LIM_INF && newZ <= LIM_SUP) {
            camX = newX; camY = newY; camZ = newZ;
        }
        // Atrás (S)
        newX = camX - dirX * speed;
        newY = camY - dirY * speed;
        newZ = camZ - dirZ * speed;
        if (keyStates['s'] && newX >= LIM_INF && newX <= LIM_SUP && newY >= LIM_DOWN && newY <= LIM_SUP && newZ >= LIM_INF && newZ <= LIM_SUP) {
            camX = newX; camY = newY; camZ = newZ;
        }
        // Izquierda (A)
        newX = camX - rightX * speed;
        newZ = camZ - rightZ * speed;
        if (keyStates['a'] && newX >= LIM_INF && newX <= LIM_SUP && newZ >= LIM_INF && newZ <= LIM_SUP) {
            camX = newX; camZ = newZ;
        }
        // Derecha (D)
        newX = camX + rightX * speed;
        newZ = camZ + rightZ * speed;
        if (keyStates['d'] && newX >= LIM_INF && newX <= LIM_SUP && newZ >= LIM_INF && newZ <= LIM_SUP) {
            camX = newX; camZ = newZ;
        }
        // Bajar (Q)
        newY = camY - speed;
        if (keyStates['q'] && newY >= LIM_DOWN) {
            camY = newY;
        }
        // Subir (Espacio)
        newY = camY + speed;
        if (keyStates[' '] && newY <= LIM_SUP) {
            camY = newY;
        }

        // Límite de distancia de la cámara (radio máximo = 50)
        float distancia = sqrtf(camX * camX + camY * camY + camZ * camZ);
        if (distancia > 50.0f) {
            float factor = 50.0f / distancia;
            camX *= factor;
            camY *= factor;
            camZ *= factor;
        }
    }
    glutPostRedisplay();

}

std::vector<Luna> lunasMarte;
std::vector<Luna> lunasJupiter;

std::vector<float> velocidadOrbitalRelativa = {
    0.0f, // Sol
    1.0f, // Mercurio
    0.615f, // Venus
    1.0f, // Tierra
    1.0f,
    1.88f, // Marte
    11.86f, // Júpiter
    29.46f, // Saturno
    84.01f, // Urano
    164.8f  // Neptuno
};

std::vector<float> velocidadRotacion = {
    0.0f,   // Sol
    6.14f,  // Mercurio
    -1.48f, // Venus
    1.0f,   // Tierra
    2.0f,
    0.97f,  // Marte
    2.41f,  // Júpiter
    2.24f,  // Saturno
    -1.41f, // Urano
    1.49f   // Neptuno
};


bool rotacionActiva = false;
bool translacionActiva = false;


void keyDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 't' || key == 'T') {
        translacionActiva = !translacionActiva;
    }

    if (key == 'r' || key == 'R') {
        rotacionActiva = !rotacionActiva;
    }

    if (key == 27) {
    exit(0);
    }

    if (key >= '1' && key <= '9') {
        int idx = key - '0';
        if (idx < planetas.size()) {
            planetaSeguido = idx;
            modoCamara = SEGUIMIENTO;
        }
    }

    if (key == 'f' || key == 'F') {
        modoCamara = LIBRE;
        camX = camX_original;
        camY = camY_original;
        camZ = camZ_original;
    }
}


void keyUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}


void mostrarTexto(float x, float y, const std::string& texto) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    glRasterPos2f(x, y);
    for (char c : texto) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopAttrib();
}

struct Estrella {
    float x, y, z;
    float size;
};
std::vector<Estrella> estrellas;


void generarEstrellas(int cantidad, float radio = 150.0f, float distanciaMinima = 3.0f) {
    estrellas.clear();
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> distAngulo(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> distCos(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distSize(1.0f, 3.5f); // Tamaño variable

    auto distanciaCuadrada = [](const Estrella& a, const Estrella& b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return dx*dx + dy*dy + dz*dz;
    };

    const float minDistSq = distanciaMinima * distanciaMinima;

    while (estrellas.size() < (size_t)cantidad) {
        float theta = distAngulo(gen);
        float phi = acos(distCos(gen));
        float x = radio * sin(phi) * cos(theta);
        float y = radio * sin(phi) * sin(theta);
        float z = radio * cos(phi);
        float size = distSize(gen);

        Estrella nueva = {x, y, z, size};

        bool muyCerca = false;
        for (const auto& e : estrellas) {
            if (distanciaCuadrada(nueva, e) < minDistSq) {
                muyCerca = true;
                break;
            }
        }
        if (!muyCerca) {
            estrellas.push_back(nueva);
        }
    }
}

std::vector<TexturaPlaneta> texturas = {
    {0, "C:/texturas/sol.png"},
    {0, "C:/texturas/mercurio.jpg"},
    {0, "C:/texturas/venus.jpg"},
    {0, "C:/texturas/tierra.jpg"},
    {0, "C:/texturas/luna.png"},
    {0, "C:/texturas/marte.jpg"},
    {0, "C:/texturas/jupiter.jpg"},
    {0, "C:/texturas/saturno.jpg"},
    {0, "C:/texturas/urano.jpg"},
    {0, "C:/texturas/neptuno.jpg"}
};

GLuint cargarTextura(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Error cargando textura: " << filename << std::endl;
        return 0;
    }

    GLuint texturaID;
    glGenTextures(1, &texturaID);
    glBindTexture(GL_TEXTURE_2D, texturaID);

    GLenum formato = (channels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, formato, width, height, 0, formato, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texturaID;
}

void dibujarPlanetas_Sol_Lunas() {
    for (size_t i = 0; i < planetas.size(); ++i) {
        const auto& planeta = planetas[i];

        glPushMatrix();
        glTranslatef(planeta.centro.x, planeta.centro.y, planeta.centro.z);
        glRotatef(90, 1, 0, 0);

        // Brillo blanco en todos los planetas
        GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat mat_shininess[] = { 400.0f };
        GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

        // Textura del planeta
        glBindTexture(GL_TEXTURE_2D, texturas[i].id);
        GLUquadric* quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);

        glRotatef(angulosRotacion[i], 0, 0, 1);  // Rotación sobre el eje de cada planeta
        gluSphere(quad, planeta.radioEscalado, 50, 50);
        gluDeleteQuadric(quad);

        // --- AROS DE SATURNO ---
        if (planeta.nombre == "Saturno") {
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            struct Aro {
                float color[4];
                float radioInterno;
                float radioExterno;
            };

            std::vector<Aro> aros = {
                { {0.95f, 0.90f, 0.80f, 0.4f}, planeta.radioEscalado * 1.20f, planeta.radioEscalado * 1.65f }, // crema
                { {0.87f, 0.76f, 0.58f, 0.4f}, planeta.radioEscalado * 1.70f, planeta.radioEscalado * 1.80f }, // beige
                { {0.75f, 0.75f, 0.75f, 0.3f}, planeta.radioEscalado * 1.90f, planeta.radioEscalado * 2.10f }  // gris
            };

            const int segmentos = 100;

            for (const auto& aro : aros) {
                glColor4fv(aro.color);
                glBegin(GL_TRIANGLE_STRIP);
                for (int j = 0; j <= segmentos; ++j) {
                    float theta = 2.0f * 3.14159f * j / segmentos;
                    float x = cos(theta), y = sin(theta);
                    glVertex3f(x * aro.radioInterno, y * aro.radioInterno, 0.0f);
                    glVertex3f(x * aro.radioExterno, y * aro.radioExterno, 0.0f);
                }
                glEnd();
            }

            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }

        glPopMatrix();

        // --- HALO DEL SOL ---
        if (planeta.nombre == "Sol") {
            glPushMatrix();
            glTranslatef(planeta.centro.x, planeta.centro.y, planeta.centro.z);

            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            glColor4f(1.0f, 1.0f, 0.3f, 0.6f);
            GLUquadric* glow1 = gluNewQuadric();
            gluSphere(glow1, planeta.radioEscalado * 1.03f, 50, 50);
            gluDeleteQuadric(glow1);

            glColor4f(1.0f, 1.0f, 0.2f, 0.3f);
            GLUquadric* glow2 = gluNewQuadric();
            gluSphere(glow2, planeta.radioEscalado * 1.08f, 50, 50);
            gluDeleteQuadric(glow2);

            glColor4f(1.0f, 0.9f, 0.1f, 0.1f);
            GLUquadric* glow3 = gluNewQuadric();
            gluSphere(glow3, planeta.radioEscalado * 1.16f, 50, 50);
            gluDeleteQuadric(glow3);

            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glPopMatrix();
        }

         // --- ÓRBITA CIRCULAR DE LOS PLANETAS ---
        if (planeta.nombre != "Sol" && planeta.nombre != "Luna") {
            glDisable(GL_LIGHTING);
            glColor3f(0.5f, 0.5f, 0.5f);
            glBegin(GL_LINE_LOOP);
            const int segmentos = 100;
            float radio = sqrt(planeta.centro.x * planeta.centro.x + planeta.centro.z * planeta.centro.z);
            for (int j = 0; j < segmentos; ++j) {
                float ang = 2.0f * M_PI * j / segmentos;
                float x = cos(ang) * radio;
                float z = sin(ang) * radio;
                glVertex3f(x, 0.0f, z);
            }
            glEnd();
            glEnable(GL_LIGHTING);
        }

    glPopMatrix();
    }

    // Lunas de Marte
    for (const auto& luna : lunasMarte) {
        glPushMatrix();
        glTranslatef(luna.posicion.x, luna.posicion.y, luna.posicion.z);
        // Activar transparencia
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Activar textura
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaLunaID);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        // Color con opacidad
        glColor4f(luna.color[0], luna.color[1], luna.color[2], 0.6f);
        // Dibujar esfera texturizada
        GLUquadric* q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        gluQuadricNormals(q, GLU_SMOOTH);
        gluSphere(q, luna.radio, 20, 20);
        gluDeleteQuadric(q);

        // Restaurar estado
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glPopMatrix();
    }

    // Lunas de Júpiter
    for (const auto& luna : lunasJupiter) {
        glPushMatrix();
        glTranslatef(luna.posicion.x, luna.posicion.y, luna.posicion.z);
        // Activar transparencia
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Activar textura
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaLunaID);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        // Color con opacidad
        glColor4f(luna.color[0], luna.color[1], luna.color[2], 0.6f);
        // Dibujar esfera texturizada
        GLUquadric* q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        gluQuadricNormals(q, GLU_SMOOTH);
        gluSphere(q, luna.radio, 15, 15);
        gluDeleteQuadric(q);

        glPopMatrix();
    }
}

void escalarRadios() {
    for (auto& planeta : planetas) {
        planeta.radioEscalado = planeta.radioOriginal * escala.factor;
    }
}

void loop(int value) {
    float velocidadOrbitalBase = 0.1f;
    float velocidadBase = 0.2f;
    // Lunas de Marte
    for (size_t i = 0; i < lunasMarte.size(); ++i) {
        Luna& luna = lunasMarte[i];
        if (translacionActiva) {
            luna.anguloOrbita += luna.velocidad;
            float rad = luna.anguloOrbita * M_PI / 180.0f;
            luna.posicion.x = planetas[5].centro.x + cos(rad) * luna.distancia;
            luna.posicion.z = planetas[5].centro.z + sin(rad) * luna.distancia;
            luna.posicion.y = planetas[5].centro.y;
        }
    }

    // Lunas de Júpiter
    for (size_t i = 0; i < lunasJupiter.size(); ++i) {
        Luna& luna = lunasJupiter[i];
        if (translacionActiva) {
            luna.anguloOrbita += luna.velocidad;
            float rad = luna.anguloOrbita * M_PI / 180.0f;
            luna.posicion.x = planetas[6].centro.x + cos(rad) * luna.distancia;
            luna.posicion.z = planetas[6].centro.z + sin(rad) * luna.distancia;
            luna.posicion.y = planetas[6].centro.y;
        }
    }

    for (size_t i = 1; i < planetas.size(); ++i) {
        if (translacionActiva) {
            angulosOrbita[i] += velocidadBase / velocidadOrbitalRelativa[i];
            float factor = 1.0f / (float)i;
            angulosOrbita[i] += velocidadOrbitalBase * factor;

            float radio = sqrt(planetas[i].centro.x * planetas[i].centro.x + planetas[i].centro.z * planetas[i].centro.z);
            float rad = angulosOrbita[i] * M_PI / 180.0f;
            planetas[i].centro.x = cos(rad) * radio;
            planetas[i].centro.z = sin(rad) * radio;
        }

        if (rotacionActiva) {
            angulosRotacion[i] += velocidadRotacion[i];
        }
    }

    size_t tierraIndex = 3;
    size_t lunaIndex = 4;
    if (translacionActiva) {
        angulosOrbita[lunaIndex] += 1.5f;
        float rad = angulosOrbita[lunaIndex] * M_PI / 180.0f;
        float distanciaLuna = 2.0f;

        planetas[lunaIndex].centro.x = planetas[tierraIndex].centro.x + cos(rad) * distanciaLuna;
        planetas[lunaIndex].centro.z = planetas[tierraIndex].centro.z + sin(rad) * distanciaLuna;
        planetas[lunaIndex].centro.y = planetas[tierraIndex].centro.y;
    }
    if (rotacionActiva) {
        angulosRotacion[lunaIndex] += 2.0f;
    }
    CamaraMovimiento();
    glutPostRedisplay();
    glutTimerFunc(16, loop, 0);
}


void dibujarEstrellas() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_POINTS);
    for (const auto& estrella : estrellas) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glPointSize(estrella.size);
        glVertex3f(estrella.x, estrella.y, estrella.z);
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_POINT_SMOOTH);
    glPopAttrib();
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Establecer la vista desde la posición de la cámara
    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    // Posicionar la luz respecto al sol
    GLfloat lightPos[] = {0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Dibujar estrellas
    dibujarEstrellas();

    // --- Mostrar texto con la ubicación de la cámara ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    std::stringstream ss;
    ss.precision(2);
    ss << std::fixed << "Ubicacion: X=" << camX << " Y=" << camY << " Z=" << camZ;
    mostrarTexto(10.0f, HEIGHT - 30.0f, ss.str());

    glEnable(GL_LIGHTING);

    // --- Mostrar para rotación y traslación  ---
    std::string estadoRot = rotacionActiva ? "activa" : "pausada";
    std::string estadoTrans = translacionActiva ? "activa" : "pausada";

    std::stringstream textoRot, textoTrans;
    textoRot << "R / r  -> Rotacion: " << estadoRot;
    textoTrans << "T / t -> Traslacion: " << estadoTrans;
    std::string textoSalir = "Esc -> Salir";

    float xrt = WIDTH - 280.0f;
    float yrt = HEIGHT - 40.0f;

    mostrarTexto(xrt, yrt, textoRot.str());
    mostrarTexto(xrt, yrt - 25.0f, textoTrans.str());
    mostrarTexto(xrt + 3, yrt - 50.0f, textoSalir);

    // --- Mostrar lista de planetas ---
    float xLista = 40.0f;
    float yLista = 60.0f;

    mostrarTexto(xLista, yLista + 180, "1 -> Mercurio");
    mostrarTexto(xLista, yLista + 160, "2 -> Venus"   );
    mostrarTexto(xLista, yLista + 140, "3 -> Tierra"  );
    mostrarTexto(xLista, yLista + 120, "4 -> Luna"    );
    mostrarTexto(xLista, yLista + 100, "5 -> Marte"   );
    mostrarTexto(xLista, yLista + 80 , "6 -> Jupiter" );
    mostrarTexto(xLista, yLista + 60 , "7 -> Saturno" );
    mostrarTexto(xLista, yLista + 40 , "8 -> Urano"   );
    mostrarTexto(xLista, yLista + 20 , "9 -> Neptuno" );
    mostrarTexto(xLista - 15, yLista + 210, "F / f -> POSICION ORIGINAL" );

    // --- Mostrar CONTROLES ---
    float xctrl = 1730.0f;
    float yctrl = 60.0f;

    mostrarTexto(xctrl- 6, yctrl + 140, "W / w -> adelante");
    mostrarTexto(xctrl, yctrl + 120, "S / s -> atras");
    mostrarTexto(xctrl, yctrl + 100, "A / a -> izquierda" );
    mostrarTexto(xctrl, yctrl + 80 , "D / d -> derecha"     );
    mostrarTexto(xctrl, yctrl + 60 , "Q / q -> abajo"   );
    mostrarTexto(xctrl - 19, yctrl + 40 , "espacio -> arriba" );

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Dibujar planetas
    escalarRadios();

    dibujarPlanetas_Sol_Lunas();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;

    float ratio = (float)w / (float)h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(90.0f, ratio, 0.1f, 200.0f);

    glMatrixMode(GL_MODELVIEW);
}


void initScene() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);

    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);

    GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat luz_difusa[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat luz_especular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luz_difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luz_especular);

    // Cargar texturas de planetas
    for (auto& tex : texturas) {
        tex.id = cargarTextura(tex.archivo);
    }

    // Usamos la misma textura de la luna del índice 4 para las lunas
    texturaLunaID = texturas[4].id;

    generarEstrellas(4000);

    // Establecer posiciones iniciales de lunas de Marte
    for (auto& luna : lunasMarte) {
        float rad = luna.anguloOrbita * M_PI / 180.0f;
        luna.posicion.x = planetas[5].centro.x + cos(rad) * luna.distancia;
        luna.posicion.z = planetas[5].centro.z + sin(rad) * luna.distancia;
        luna.posicion.y = planetas[5].centro.y;
    }

    // Establecer posiciones iniciales de lunas de Júpiter
    for (auto& luna : lunasJupiter) {
        float rad = luna.anguloOrbita * M_PI / 180.0f;
        luna.posicion.x = planetas[6].centro.x + cos(rad) * luna.distancia;
        luna.posicion.z = planetas[6].centro.z + sin(rad) * luna.distancia;
        luna.posicion.y = planetas[6].centro.y;
    }
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Sistema Planetario - Modo Libre");

    // Lunas de Marte
    float radioFobos  = 11.2667f * escala.factor * 32.7f;
    float radioDeimos = 6.2f * escala.factor * 32.7f;

    lunasMarte = {
        {0.0f, 1.0f, 1.5f, radioFobos,  {0.8f, 0.6f, 0.6f}},
        {180.0f, 1.5f, 1.2f, radioDeimos, {0.6f, 0.6f, 0.8f}}
    };

    // Modelo base de color para lunas
    std::vector<std::array<float, 3>> coloresLunas = {
        {0.7f, 0.7f, 0.7f}, // Gris claro
        {0.6f, 0.6f, 0.7f}, // Gris azulado
        {0.8f, 0.7f, 0.6f}, // Beige pálido
        {0.5f, 0.5f, 0.5f}, // Gris medio
        {0.9f, 0.9f, 0.9f}, // Casi blanco
        {0.6f, 0.5f, 0.4f}, // Marrón claro
        {0.4f, 0.5f, 0.6f}  // Gris azulado oscuro
    };

    // 95 lunas para Júpiter
    for (int i = 0; i < 95; ++i) {
        float angulo = (360.0f / 95) * i;
        float distancia = 3.0f + (i % 6) * 0.45f;
        float velocidad = 0.5f + fabs(cos(i * 0.21f)) * 1.2f;

        float radio_km = 2.0f + fabs(sin(i * 0.37f)) * 16.0f;
        float radioEscalado = radio_km * escala.factor * 32.7f;

        auto color = coloresLunas[i % coloresLunas.size()];

        lunasJupiter.push_back({angulo, distancia, velocidad, radioEscalado, {color[0], color[1], color[2]}});
    }

    initScene();
    escalarRadios();

    angulosOrbita.resize(planetas.size(), 0.0f);
    angulosRotacion.resize(planetas.size(), 0.0f);

    camX_original = camX;
    camY_original = camY;
    camZ_original = camZ;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMotion);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutTimerFunc(0, loop, 0);

    glutMainLoop();
    return 0;
}
