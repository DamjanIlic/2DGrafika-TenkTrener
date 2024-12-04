//Damjan Ilic RA-103/2021
//Tenk trener

#define _CRT_SECURE_NO_WARNINGS





#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
using namespace std;

//stb_image.h je header-only biblioteka za ucitavanje tekstura.
//Potrebno je definisati STB_IMAGE_IMPLEMENTATION prije njenog ukljucivanja
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <ft2build.h>
#include FT_FREETYPE_H
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju

void drawBullet(unsigned int shaderProgram, unsigned int bullet);
void drawNeedle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue, float angleDegrees);

void shootBullet(unsigned int shaderProgram);
void updateReadyIndicator(float currentTime);

double mapValue(double value, double minSource, double maxSource, double minTarget, double maxTarget) {
    // Proveri da li je vrednost unutar raspona izvora
    if (value < minSource || value > maxSource) {
        std::cerr << "Value out of source range!" << std::endl;
        return 0.0;
    }

    // Mapiraj vrednost na ciljani raspon
    return minTarget + (value - minSource) * (maxTarget - minTarget) / (maxSource - minSource);
}



struct Target {
    float x;
    float targetOffset = 0.0f;
    bool isAlive = true;
};

void shoot(float currentTime, std::vector<Target> &targets);

void drawtxt(unsigned int shader, float x1, float x2, float y1, float y2, unsigned texture) {
    glUseProgram(shader);
    float vertices[] =


        //x1, y1, 0.0f, // Gornji levi ugao
        //x2, y1, 0.0f, // Gornji desni ugao
        //x2, y2, 0.0f, // Donji desni ugao
        //x1, y2, 0.0f  // Donji levi ugao
    //(1.55f, 0.75f, 0.0, 0.9

    {   //X    Y      S    T 
    //    0.4f, 0.0,   0, 0.0,//prvo tjeme
    //    0.9, 0.0,  1.0, 0.0, //drugo tjeme
    //    0.9, 0.9,    1.0, 1, //trece tjeme
    //    0.4f, 0.9,  0.0, 1
        x1, y1,   0, 0.0,//prvo tjeme
        x2, y1,  1.0, 0.0, //drugo tjeme
        x2, y2,    1.0, 1, //trece tjeme
        x1,y2,  0.0, 1
    };

    // notacija koordinata za teksture je STPQ u GLSL-u (ali se cesto koristi UV za 2D teksture i STR za 3D)
    //ST koordinate u nizu tjemena su koordinate za teksturu i krecu se od 0 do 1, gdje je 0, 0 donji lijevi ugao teksture
    //Npr. drugi red u nizu tjemena ce da mapira boje donjeg lijevog ugla teksture na drugo tjeme
    unsigned int stride = (2 + 2) * sizeof(float);

    unsigned int VAOt;
    glGenVertexArrays(1, &VAOt);
    glBindVertexArray(VAOt);
    unsigned int VBOt;
    glGenBuffers(1, &VBOt);
    glBindBuffer(GL_ARRAY_BUFFER, VBOt);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Tekstura zavisnost od slike, ne valja ovako al nisam se snasao
    //unsigned checkerTexture = loadImageToTexture("res/voltmeter.png"); //Ucitavamo teksturu
    //unsigned checkerTexture = loadImageToTexture(filePath);
    unsigned checkerTexture = texture;
    //if (imgNum == 0) {
    //    loadImageToTexture("res/voltmeter.png");
    //}
    //else if (imgNum == 1) {
    //    loadImageToTexture("res/")
    //}
    glBindTexture(GL_TEXTURE_2D, checkerTexture); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D); //Generisemo mipmape 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(shader);
    unsigned uTexLoc = glGetUniformLocation(shader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)
    glUseProgram(0);

    glUseProgram(shader);
    glBindVertexArray(VAOt);

    glActiveTexture(GL_TEXTURE0); //tekstura koja se bind-uje nakon ovoga ce se koristiti sa SAMPLER2D uniformom u sejderu koja odgovara njenom indeksu
    glBindTexture(GL_TEXTURE_2D, checkerTexture);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    //glDeleteTextures(1, &checkerTexture);
    glDeleteBuffers(1, &VBOt);
    glDeleteVertexArrays(1, &VAOt);
    //glDeleteProgram(shader);
}

/*class CountdownTimer {
public:
    CountdownTimer(int startSeconds) : secondsLeft(startSeconds) {
        // Postavi pocetni broj sekundi
    }

    // Funkcija koja odbrojava i azurira tekst
    void update() {
        if (secondsLeft > 0) {
            --secondsLeft;  // Smanji broj sekundi za 1
            text = std::to_string(secondsLeft);  // Azuriraj tekst
        }
        else {
            text = "0";  // Ako je odbrojavanje zavrseno, postavi tekst na "0"
        }
    }

    // Funkcija za dobijanje trenutnog teksta
    string getText() const {
        return text;
    }

private:
    int secondsLeft;  // Preostali broj sekundi
    string text;      // Tekst koji prikazuje preostalo vreme
};*/

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};
std::map<char, Character> Characters;
void RenderText(unsigned int shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VAOF, unsigned int VBOF)
{
    // activate corresponding render state
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAOF);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBOF);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawCircle() {
    unsigned int shaderProgram = createShader("circle.vert", "circle.frag");

    // Generisanje verteksa za krug
    const int numSegments = 100; // Broj segmenta za krug (vise segmenta = gladji krug)
    float radius = 0.5f;
    float circleVertices[3 * (numSegments + 1)]; // +1 za centar kruga

    // Centar kruga (0, 0, 0)
    circleVertices[0] = 0.0f;
    circleVertices[1] = 0.0f;
    circleVertices[2] = 0.0f;

    // Generisanje tacaka na krugu
    for (int i = 0; i <= numSegments; ++i) {
        float angle = 2.0f * 3.14159f * i / numSegments; // Razliciti uglovi za svaku tacku
        circleVertices[3 * i + 0] = radius * cos(angle); // X koordinata
        circleVertices[3 * i + 1] = radius * sin(angle); // Y koordinata
        circleVertices[3 * i + 2] = 0.0f;               // Z koordinata
    }

    unsigned int VAOk, VBOk;
    glGenVertexArrays(1, &VAOk);
    glGenBuffers(1, &VBOk);

    glBindVertexArray(VAOk);
    glBindBuffer(GL_ARRAY_BUFFER, VBOk);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Prvo cisti stencil bufer
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // 1. Postavljanje za crtanje u stencil bufer (unutrasnjost kruga)
    glStencilFunc(GL_ALWAYS, 1, 0xFF);  // Svi pikseli prolaze, postavlja vrednost 1 za sve
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // Zamenjujemo vrednost stencila sa 1 unutar kruga

    // Nacrtaj krug (u stencil buferu ce biti postavljena vrednost 1 unutar kruga)
    glUseProgram(shaderProgram);
    glBindVertexArray(VAOk);
    GLuint colorSwitchLocation = glGetUniformLocation(shaderProgram, "colorSwitch");
    glUniform1f(colorSwitchLocation, 1.0f);  // Boja unutar kruga (ako je potrebno)
    glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 1);  // Crtanje kruga

    // 2. Postavljanje pravila za crtanje samo van kruga (sve osim unutrasnjosti kruga)
    glStencilFunc(GL_EQUAL, 0, 0xFF);  // Samo pikseli sa vrednoscu 0 u stencil buferu se crtaju (izvan kruga)
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);  // Ne menjaj stencil vrednost

    // Omoguci blendovanje za lepu transparenciju (ako je potrebno)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Nacrtaj vanjski deo (van kruga, moze biti neka pozadina)
    //glClearColor(1.0f, 0.5f, 0.2f, 0.0f);  // Narandzasta boja pozadine
    //glClear(GL_COLOR_BUFFER_BIT);  // Cisti boju ekrana

    // Crtanje samo van kruga
    glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 1);  // Ispunjava celu pozadinu van kruga
}








                                                                                                                    //SETTINGS


bool shouldDrawBullet = true;
int ammo = 10;
bool canShoot = true;
const double cooldown = 7.5;
float lastShotTime = 0.0f;
float voltage = 75.0f;
float hydraulic = 0.0f;
bool toggleMode = true;
float offset = 0.0f; //za panoramu
float targetOffset = 0.0f;
int targetNum = 3;
int hit = 0;
float targetFPS = 60.0f;           // Ciljani FPS
float frameTime = 1.0f / targetFPS;
float missionDoneTime = 0.0f;

//////////////////targets

float randBetween(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

unsigned checkerTexture = loadImageToTexture("res/texel_checker.png"); //Ucitavamo teksturu


unsigned int rectVAO, rectVBO;

void initRectangle();


void drawRectangle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue);
//void drawTexture(float x1, float x2, float y1, float y2, int shaderProgram, GLuint textureID, float shaderValue);




int main(void)
{




    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ INICIJALIZACIJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (!glfwInit()) {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    float aspectRatio = (float)mode->width / (float)mode->height;
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "[Tenk Trener]", glfwGetPrimaryMonitor(), NULL);

    if (window == NULL) {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    //freetype
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    std::map<char, Character> Characters;
    unsigned int textures[128];

    glGenTextures(128, textures);
    for (int i = 0; i < 128; i++) {
        std::cout << "TEXTURE : " << textures[i] << std::endl;
    }
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture = textures[c];
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        std::cout << "Texture ID for character " << c << ": " << texture << std::endl;
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        std::cout << "Glyph Width: " << face->glyph->bitmap.width
            << ", Height: " << face->glyph->bitmap.rows << std::endl;
        Characters.insert(std::pair<char, Character>(c, character));
    }

    //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Tamna pozadina
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ TEKSTURE ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned bulletG = loadImageToTexture("res/tank_bullet.png");
    unsigned voltmeterG = loadImageToTexture("res/voltmeter.png");
    unsigned flamesG = loadImageToTexture("res/flames.png");
    unsigned panoramaG = loadImageToTexture("res/panorama.png");
    unsigned targetG = loadImageToTexture("res/target.png");
    unsigned zoomG = loadImageToTexture("res/zoom.png");
    unsigned deadG = loadImageToTexture("res/dead.png");
    unsigned tankCannonG = loadImageToTexture("res/tank_cannon.png");
    unsigned backgroundG = loadImageToTexture("res/background.png");
    unsigned semaphoreONG = loadImageToTexture("res/semaphoreON.png");
    unsigned semaphoreOFFG = loadImageToTexture("res/semaphoreOFF.png");
    unsigned aimG = loadImageToTexture("res/aim.png");
    unsigned dottedG = loadImageToTexture("res/dotted.png");
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ SEJDERI ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int lightBulbShader = createShader("lightBulb.vert", "lightBulb.frag");
    unsigned int panoramaShader = createShader("panorama.vert", "panorama.frag");


    /* */
    unsigned int VAOF, VBOF;
    glGenVertexArrays(1, &VAOF);
    glGenBuffers(1, &VBOF);
    glBindVertexArray(VAOF);
    glBindBuffer(GL_ARRAY_BUFFER, VBOF);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /**/

    float vertices[] =
    {   //X    Y      S    T 
        0.25, 0.0,   1.0, 0.0,//prvo tjeme
        -0.25, 0.0,  0.0, 0.0, //drugo tjeme
        0.0, 0.5,    0.5, 1.0 //trece tjeme
    };
    // notacija koordinata za teksture je STPQ u GLSL-u (ali se cesto koristi UV za 2D teksture i STR za 3D)
    //ST koordinate u nizu tjemena su koordinate za teksturu i krecu se od 0 do 1, gdje je 0, 0 donji lijevi ugao teksture
    //Npr. drugi red u nizu tjemena ce da mapira boje donjeg lijevog ugla teksture na drugo tjeme
    unsigned int stride = (2 + 2) * sizeof(float);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    glUseProgram(0);

    //Odnosi se na glActiveTexture(GL_TEXTURE0) u render petlji
    //Moguce je sabirati indekse, tj GL_TEXTURE5 se moze dobiti sa GL_TEXTURE0 + 5 , sto je korisno za iteriranje kroz petlje

    /*while (!glfwWindowShouldClose(window))
    {

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }*/
    initRectangle();
    ///slike ucitavanje

/*unsigned bullet = loadImageToTexture("res/tank_bullet");
unsigned voltmeter = loadImageToTexture("res/voltmeter.png");
unsigned flames = loadImageToTexture("res/flames.png");*/

    unsigned bullet = bulletG;
    unsigned voltmeter = voltmeterG;
    unsigned flames = flamesG;
    unsigned target = targetG;
    unsigned zoom = zoomG;
    unsigned dead = deadG;
    unsigned tankCannon = tankCannonG;

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ KRUZNI OBLIK (TELO SIJALICE) ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    const int numSegments = 100; // Preciznost kruga
    float radius = 0.15f; // Poluprecnik za telo sijalice
    float circleVertices[3 * (numSegments + 1)];
    circleVertices[0] = 0.0f; // Centar kruga
    circleVertices[1] = 0.0f;
    circleVertices[2] = 0.0f;

    for (int i = 0; i <= numSegments; ++i) {
        float angle = 2.0f * 3.14159f * i / numSegments;
        circleVertices[3 * i + 0] = -1.5f + radius * cos(angle);
        circleVertices[3 * i + 1] = 0.5f + radius * sin(angle);
        circleVertices[3 * i + 2] = 0.0f;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PRAVOUGAONIK (GRLO SIJALICE) ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    float rectangleVertices[] = {
    -1.55f, 0.4f, 0.0f, // Gornji levi ugao
     -1.45f, 0.4f, 0.0f, // Gornji desni ugao
     -1.45f, 0.2f, 0.0f, // Donji desni ugao
    -1.55f, 0.2f, 0.0f  // Donji levi ugao
    };

    unsigned int rectVAO, rectVBO;
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);

    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ SVETLOST (AURORA) ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    float lightRadius = 0.2f;
    float lightVertices[3 * (numSegments + 1)];
    lightVertices[0] = 0.0f;
    lightVertices[1] = 0.0f;
    lightVertices[2] = 0.0f;

    for (int i = 0; i <= numSegments; ++i) {
        float angle = 2.0f * 3.14159f * i / numSegments;
        lightVertices[3 * i + 0] = -1.5f + lightRadius * cos(angle);
        lightVertices[3 * i + 1] = 0.5f + lightRadius * sin(angle);
        lightVertices[3 * i + 2] = 0.0f;
    }

    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ CILINDAR TOPOVSKOG METKA ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    float bulletCillinderVertices[]{
    -1.55f, -0.6f, 0.0f, // Gornji levi ugao
     -1.45f, -0.6f, 0.0f, // Gornji desni ugao
     -1.45f, -0.8f, 0.0f, // Donji desni ugao
    -1.55f, -0.8f, 0.0f  // Donji levi ugao
    };


    unsigned int bulletCillinderVAO, bulletCillinderVBO;

    //unsigned int rectVAO, rectVBO;


    glGenVertexArrays(1, &bulletCillinderVAO);
    glGenBuffers(1, &bulletCillinderVBO);

    glBindVertexArray(bulletCillinderVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bulletCillinderVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bulletCillinderVertices), bulletCillinderVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //da moze odma da puca
    lastShotTime = static_cast<float>(glfwGetTime()) - 8.0f;



    //freetype
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    unsigned int textShader = createShader("textShader.vert", "textShader.frag");
    glUseProgram(textShader);
    int position = glGetUniformLocation(textShader, "position");
    glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    //tajmer
    //CountdownTimer timer(60);

    float startedTime = static_cast<float>(glfwGetTime());
    /////////////                                                                   TARGETS
    std::vector<Target> targets;
    for (int i = 0; i < targetNum; ++i) {
        Target newTarget;
        newTarget.x = randBetween(-0.7f, 1.0f);
        //std::cout << "xd";
        targets.push_back(newTarget);
    }
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER PETLJA ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }


        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            toggleMode = true;
        }

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        {
            toggleMode = false;
        }

        //Trenutno vreme
        float currentTime = static_cast<float>(glfwGetTime());
        //std::cout << currentTime << endl;
        updateReadyIndicator(currentTime);


        /*std::cout << bulletG; 1
        std::cout << voltmeterG;*/ //2
        //unsigned bullet = loadImageToTexture("res/tank_bullet.png");

        //std::cout << bullet;
        //unsigned voltmeter = loadImageToTexture("res/voltmeter.png")
        //unsigned voltmeter = 2;
        //unsigned flames = loadImageToTexture("res/flames.png");

        //unsigned bullet = bulletG;


        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glStencilFunc(GL_ALWAYS, 1, 0xFF); // Svi pikseli prolaze
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // Zamenjujemo vrednost stencila sa 1

        glUniform1f(glGetUniformLocation(lightBulbShader, "aspectRatio"), aspectRatio);
        glUseProgram(lightBulbShader);

        // Crtanje pravougaonika (grlo sijalice)
        /*glBindVertexArray(rectVAO);
        glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);*/

        glUseProgram(unifiedShader);
        glBindVertexArray(VAO);

        glActiveTexture(GL_TEXTURE0); //tekstura koja se bind-uje nakon ovoga ce se koristiti sa SAMPLER2D uniformom u sejderu koja odgovara njenom indeksu
        glBindTexture(GL_TEXTURE_2D, checkerTexture);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        //
        //std::cout << missionDoneTime << endl;
        //std::cout << currentTime << endl << endl;
        if ((hit == targetNum) && (abs(currentTime - missionDoneTime) > 5.0f) && missionDoneTime != 0.0f) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }


        string text = "xdd";
        //KUPOLA POV
        if (toggleMode) {
            drawtxt(unifiedShader, -1, 1, -1, 1, backgroundG);
            //drawRectangle(0, 0, 0, 0, lightBulbShader, 1.0f);
            {
                glUseProgram(textShader);
                glUniform3f(glGetUniformLocation(textShader, "textColor"), 1.5f, 1.8f, 1.4f);
                glActiveTexture(GL_TEXTURE0);
                glBindVertexArray(VAOF);
                //timer.update();  // Azuriraj odbrojavanje
                //this_thread::sleep_for(chrono::seconds(1));  // Cekaj 1 sekundu
                int remaining = 60.0f + startedTime - currentTime;

                if (hit==targetNum && remaining>0.0f) {
                    text = "Uspesna misija";
                    if (missionDoneTime == 0.0f) {
                        std::cout << "kraj";
                        missionDoneTime = static_cast<float>(glfwGetTime());
                    }
                  
                    
                    //glfwSetWindowShouldClose(window, GL_TRUE);
                }
                else if (remaining <= 0.0f) {
                    if (hit != targetNum) {
                        text = "Neuspesna misija";
                    }
                    else {
                        glfwSetWindowShouldClose(window, GL_TRUE);
                    }
                }
                else {
                    text = std::to_string(remaining);
                }
                
                
                float scale = 0.5f;
                float x = 350.0f; float y = 550.0f;
                // iterate through all characters
                std::string::const_iterator c;
                for (c = text.begin(); c != text.end(); c++)
                {
                    Character ch = Characters[*c];

                    float xpos = x + ch.Bearing.x * scale;
                    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

                    float w = ch.Size.x * scale;
                    float h = ch.Size.y * scale;
                    // update VBO for each character
                    float vertices[6][4] = {
                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos,     ypos,       0.0f, 1.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },

                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },
                        { xpos + w, ypos + h,   1.0f, 0.0f }
                    };
                    // render glyph texture over quad
                    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                    // update content of VBO memory
                    glBindBuffer(GL_ARRAY_BUFFER, VBOF);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    // render quad
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                    x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
                }
                //RenderText(textShader, "cao", 1.0f, 1.0f, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f), VAOF, VBOF);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            //RenderText(textShader, "cao", 1.0f, 1.0f, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f), VAOF, VBOF);
            //drawRectangle(-1.55f, -1.45f, 0.4, 0.2, lightBulbShader, 0.0f);
            //drawRectangle(-1.55f, -1.45f, 0.4, 0.2, lightBulbShader, 2.0f);

            //drawRectangle(-1.55f, -1.45f, 0.7, 0.5, lightBulbShader, 0.0f);
            

            // Crtanje svetlosnog efekta                                                                 //SIJALICA 
           // glBindVertexArray(lightVAO);
            //if (canShoot && ammo) {
            //    glUniform1f(glGetUniformLocation(lightBulbShader, "colorSwitch"), 2.0f);  //ima ga 
            //}
            //else {
            //    glUniform1f(glGetUniformLocation(lightBulbShader, "colorSwitch"), 4.0f);
            //    drawRectangle(-1.55f, -1.45f, 0.4, 0.2, lightBulbShader, 0.0f);
            //}

           // glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 1);

            // Crtanje kruga (telo sijalice)
            //glBindVertexArray(VAO);
            if (canShoot && ammo) {
                //glUniform1f(glGetUniformLocation(lightBulbShader, "colorSwitch"), 1.0f);  //ima ga 
                drawtxt(unifiedShader, -.9, -.5, 0.2, 0.7, semaphoreONG);
            }
            else {
                //glUniform1f(glGetUniformLocation(lightBulbShader, "colorSwitch"), 2.0f);
                drawtxt(unifiedShader, -.9, -.5, 0.2, 0.7, semaphoreOFFG);
            }
            //glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 1);




            //VOLTMETAR
//drawTexture(-1.55f, -1.45f, 0.7f, 0.5f, unifiedShader, voltmeterTexture, 2.0f);
//drawRectangle(-1.0f, -1.0f, 0.5f, 0.5f, lightBulbShader, 2.0f);
//drawRectangle(1.55f, 0.75f, 0.0f, 0.9f, lightBulbShader, 2.0f);

//voltage engl(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                                         // Provera tastera +
            if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
                voltage += 1.0f; // Povecaj napon
                if (voltage > 90.0f) voltage = 90.0f; // Ogranici maksimalan napon
            }

            // Provera tastera -
            if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
                voltage -= 1.0f; // Smanji napon
                if (voltage < 0.0f) voltage = 0.0f; // Ogranici minimalan napon
            }
            hydraulic = (voltage+10.0) / 100.0;

            //drawtxt();

            drawtxt(unifiedShader, 0.4f, 0.9f, 0.0f, 0.9f, voltmeter);
            //drawtxt(unifiedShader, 0.1f, 0.3f, 0.0f, 0.9f, flames);

            //drawRectangle(-1.55f, -1.45f, 0.4, 0.2, lightBulbShader, 2.0f);
            drawRectangle(-0, -0, 0, 0, lightBulbShader, 2.0f);

            //(-0.5f, 0.5f, -0.25f, 0.25f, shaderProgram, 1.0f, 45.0f);
            //drawtxt(unifiedShader);
            //drawRectangle(-1.55f, -1.45f, 0.4, 0.2, lightBulbShader, 2.0f);
            //drawRectangle(-1.55f, -1.45f, 0.4, 0.2, lightBulbShader, 2.0f);
            // Crtanje municije

            //Metak Cilindar
            /*glBindVertexArray(bulletCillinderVAO);
            glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), 3.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);*/

            drawBullet(unifiedShader, bulletG);

            drawRectangle(-0, -0, 0, 0, lightBulbShader, 2.0f);
            //drawtxt(unifiedShader);
            //shootBullet(shaderProgram);
            if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)) {
                currentTime = static_cast<float>(glfwGetTime());
                shoot(currentTime, targets);
            }
            //drawtxt();

            /*if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)) {
                canShoot = true;
            }
            if (canShoot) {
                ammo = ammo - 1;
            }*/
            //shootBullet(shaderProgram);
            drawRectangle(-0, -0, 0, 0, lightBulbShader, 2.0f);
            ///indeks
            {
                glUseProgram(textShader);
                glUniform3f(glGetUniformLocation(textShader, "textColor"), 1.5f, 1.8f, 1.4f);
                glActiveTexture(GL_TEXTURE0);
                glBindVertexArray(VAOF);
                //timer.update();  // Azuriraj odbrojavanje
                //this_thread::sleep_for(chrono::seconds(1));  // Cekaj 1 sekundu
                //int remaining = 60.0f + startedTime - currentTime;

                text = "Damjan Ilic RA-103/2021";

                float scale = 0.5f;
                float x = 525.0f; float y = 50.0f;
                // iterate through all characters
                std::string::const_iterator c;
                for (c = text.begin(); c != text.end(); c++)
                {
                    Character ch = Characters[*c];

                    float xpos = x + ch.Bearing.x * scale;
                    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

                    float w = ch.Size.x * scale;
                    float h = ch.Size.y * scale;
                    // update VBO for each character
                    float vertices[6][4] = {
                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos,     ypos,       0.0f, 1.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },

                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },
                        { xpos + w, ypos + h,   1.0f, 0.0f }
                    };
                    // render glyph texture over quad
                    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                    // update content of VBO memory
                    glBindBuffer(GL_ARRAY_BUFFER, VBOF);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    // render quad
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                    x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
                }
                //RenderText(textShader, "cao", 1.0f, 1.0f, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f), VAOF, VBOF);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            //
            drawNeedle(0.9f, 1.40f, 0.157, 0.207, lightBulbShader, 5.0f, -voltage);
            drawRectangle(-0, -0, 0, 0, lightBulbShader, 2.0f);
            while (static_cast<float>(glfwGetTime()) - currentTime < frameTime) {
                //zadrzi frejm
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        else {
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glStencilFunc(GL_ALWAYS, 1, 0xFF); // Svi pikseli prolaze
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // Zamenjujemo vrednost stencila sa 1


            if (hit == targetNum) {
                //text = "Uspesna misija";
               // while ((static_cast<float>(glfwGetTime()) - currentTime) < 5.0f) {
                    //zadrzi da procita
               // }
               // glfwSetWindowShouldClose(window, GL_TRUE);
                if (missionDoneTime == 0.0f) {
                    missionDoneTime = static_cast<float>(glfwGetTime());
                   // std::cout << missionDoneTime << endl;
                   // std::cout << currentTime;
                }

            }

            //test nisana
            //
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS) {
                
                
                std::cout << "Mouse position: (" << xpos << ", " << ypos << ")" << std::endl;
                std::cout << "width: " << mode->width << std::endl;
                if (((float)xpos / (float)mode->width > 0.27) and (float)xpos / (float)mode->width < 0.73) {
                    std::cout << "dobar" << std::endl;
                    offset += 0.005f * hydraulic; // Pomeranje udesno
                    for (Target& t : targets) {
                        // Pristupamo vrednosti x svakog targeta
                        //std::cout << "xaxa";
                        t.targetOffset += 0.005f * 2. * hydraulic;
                        //t.x = t.x + t.targetOffset;
                        //std::cout << "x: " << target.x << std::endl;
                    }
                }
            }



            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                offset += 0.005f * hydraulic; // Pomeranje udesno
                for (Target& t : targets) {
                    // Pristupamo vrednosti x svakog targeta
                    //std::cout << "xaxa";
                    t.targetOffset += 0.005f * 2. * hydraulic;
                    //t.x = t.x + t.targetOffset;
                    //std::cout << "x: " << target.x << std::endl;
                }
                //targetOffset += 0.01f*2.;
            }

            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                offset -= 0.005f * hydraulic; // Pomeranje ulevo
                for (Target& t : targets) {
                    t.targetOffset -= 0.005f * 2. * hydraulic;
                }
                //targetOffset -= 0.01f*2.;
            }

            if (offset > 1.0f) offset -= 1.0f; // Petlja kroz panoramu
            if (offset < -1.0f) offset += 1.0f;
            for (Target& t : targets) {
                //if (t.targetOffset < -0.89f) t.targetOffset = 1.099f;
                //if (t.targetOffset > 1.1f) t.targetOffset = -0.889f;

                if (t.x - t.targetOffset > 1.05) {
                    t.targetOffset = 0.94 + t.x;
                }
                if (t.x - t.targetOffset < -1.05) {
                    t.targetOffset = -0.94 + t.x;
                }
            }

            // if (targetOffset < -0.89f) targetOffset = 1.099f;
            // if (targetOffset > 1.1f) targetOffset = -0.889f;
             //drawtxt(panoramaShader, -1, 1, -0.5, 0.5, panoramaG);

            glUseProgram(panoramaShader);

            glUniform1f(glGetUniformLocation(panoramaShader, "offset"), offset);

            // Binduj teksturu
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, panoramaG);
            glUniform1i(glGetUniformLocation(panoramaShader, "panorama"), 0);
            drawtxt(panoramaShader, -1, 1, -0.9, 0.9, panoramaG);
            // targetOffset = offset * 3.55;
            if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) {
                for (Target& t : targets) {
                    std::cout << "TARGET" << endl;

                    std::cout << "X=" << std::to_string(t.x);
                    std::cout << "Offset=" << std::to_string(t.targetOffset);
                }
            }

            //::cout << targetOffset;
            for (const Target& t : targets) {
                // Pristupamo vrednosti x svakog targeta
                //std::cout << "xaxa";
                if (t.isAlive) {
                    drawtxt(unifiedShader, t.x - t.targetOffset, t.x + 0.2 - t.targetOffset, -0.1, 0.1, target);
                    drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);
                }
                else {
                    drawtxt(unifiedShader, t.x - t.targetOffset, t.x + 0.2 - t.targetOffset, -0.1, 0.1, dead);
                    drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);
                }
                
                //drawtxt(unifiedShader, t.x, t.x + 0.2, 0.0, 0.2, target);
                //drawRectangle(t.x, t.x+.2, 0.6, 0.1, lightBulbShader, 0.0f);
                //std::cout << "x: " << target.x << std::endl;
            }
            //drawtxt(unifiedShader, 0.0 - targetOffset, 0.2 - targetOffset, 0.0, 0.2, target);
            drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);
            //drawRectangle(0.0 - targetOffset, 0.1 - targetOffset, 0.0, 0.1, lightBulbShader, 0.0f);
            //drawRectangle(0.0-targetOffset*3.55, 0.1-targetOffset*3.55, 0.0, 0.1, lightBulbShader, 0.0f);
            // Nacrtaj kvadrat ili pravougaonik
            //drawRectangle(-2, -0.7, -1., 1., lightBulbShader, 5.0f);
            
            //drawCircle();
            drawtxt(unifiedShader, -.1, 0.6, -1.1, 0, tankCannon);
            drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);

            if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)) {
                currentTime = static_cast<float>(glfwGetTime());
                shoot(currentTime, targets);
                if ((currentTime - lastShotTime) <= 0.5) {
                    drawtxt(unifiedShader, 0.1, 0.3, -0.35, -0.15, flames);
                }
            }

            drawtxt(unifiedShader, -1.0, 1.0, -1.0, 1.0, zoom);
            if ((((float)xpos / (float)mode->width) > 0.27) and ((float)xpos / (float)mode->width) < 0.5) {
                float distanceSpeed = (float)xpos / (float)mode->width;
                distanceSpeed = abs(distanceSpeed - 0.5);
                distanceSpeed *= 4;
                offset -= 0.005f * hydraulic * distanceSpeed; // Pomeranje ulevo
                for (Target& t : targets) {
                    t.targetOffset -= 0.005f * 2. * hydraulic * distanceSpeed;
                }
                double x_map = mapValue((double)((float)xpos / (float)mode->width), 0.27, 0.5, -0.45, 0);
                //drawRectangle((float)x_map-0.05, (float)x_map+0.05, 0.0, 0.1, lightBulbShader, 1.0f);
                drawtxt(unifiedShader, (float)x_map - 0.075, (float)x_map + 0.075, -0.05f, 0.1f, aimG);
                if (x_map < -0.065) {
                    drawtxt(unifiedShader, 0.0, (float)x_map + 0.065, -0.175f, 0.1f, dottedG);
                    drawtxt(unifiedShader, 0.0, (float)x_map + 0.065, -0.0725f, 0.175f, dottedG);
                }
                //drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);
                //drawRectangle(-.8, 0, 0.0, 0.1, lightBulbShader, 1.0f);
            }
            else if (((float)xpos / (float)mode->width >= 0.5) and (float)xpos / (float)mode->width < 0.73) {
                float distanceSpeed = (float)xpos / (float)mode->width;
                distanceSpeed = abs(distanceSpeed - 0.5);
                distanceSpeed *= 4;
                offset += 0.005f * hydraulic * distanceSpeed; // Pomeranje udesno
                for (Target& t : targets) {
                    // Pristupamo vrednosti x svakog targeta
                    //std::cout << "xaxa";
                    t.targetOffset += 0.005f * 2. * hydraulic * distanceSpeed;
                    //t.x = t.x + t.targetOffset;
                    //std::cout << "x: " << target.x << std::endl;
                }
                double x_map = mapValue((double)((float)xpos / (float)mode->width), 0.5, 0.73, 0, .45);
                //drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);
                //drawRectangle((float)x_map-0.05, (float)x_map+0.05, 0.0, 0.1, lightBulbShader, 1.0f);
                drawtxt(unifiedShader, (float)x_map - 0.075, (float)x_map + 0.075, -0.05f, 0.1f, aimG);
                //drawRectangle(0., 0, 0.0, 0.1, lightBulbShader, 0.0f);
                if (x_map > 0.065) {
                    drawtxt(unifiedShader, 0.0, (float)x_map - 0.065, -0.175f, 0.1f, dottedG);
                    drawtxt(unifiedShader, 0.0, (float)x_map - 0.065, -0.0725f, 0.175f, dottedG);
                }

                //drawRectangle(0.0, 0.0, 0.0, 0.0, lightBulbShader, 0.0f);
            }
            else if ((float)xpos / (float)mode->width < 0.27 or (float)xpos / (float)mode->width>0.73) {
                drawtxt(unifiedShader, -0.075f, +0.075f, -0.05f, 0.1f, aimG);
            }
            glBindVertexArray(0);

            ///indeks
            {
                glUseProgram(textShader);
                glUniform3f(glGetUniformLocation(textShader, "textColor"), 1.5f, 1.8f, 1.4f);
                glActiveTexture(GL_TEXTURE0);
                glBindVertexArray(VAOF);
                //this_thread::sleep_for(chrono::seconds(1));  // Cekaj 1 sekundu
                int remaining = 60.0f + startedTime - currentTime;

                text = "Damjan Ilic RA-103/2021";

                float scale = 0.5f;
                float x = 525.0f; float y = 50.0f;
                // iterate through all characters
                std::string::const_iterator c;
                for (c = text.begin(); c != text.end(); c++)
                {
                    Character ch = Characters[*c];

                    float xpos = x + ch.Bearing.x * scale;
                    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

                    float w = ch.Size.x * scale;
                    float h = ch.Size.y * scale;
                    // update VBO for each character
                    float vertices[6][4] = {
                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos,     ypos,       0.0f, 1.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },

                        { xpos,     ypos + h,   0.0f, 0.0f },
                        { xpos + w, ypos,       1.0f, 1.0f },
                        { xpos + w, ypos + h,   1.0f, 0.0f }
                    };
                    // render glyph texture over quad
                    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                    // update content of VBO memory
                    glBindBuffer(GL_ARRAY_BUFFER, VBOF);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    // render quad
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                    x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
                }
                //RenderText(textShader, "cao", 1.0f, 1.0f, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f), VAOF, VBOF);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            //
            while ((static_cast<float>(glfwGetTime()) - currentTime) < frameTime){
                //zadrzi frejm;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        //indeks
        

    }

    glDeleteTextures(1, &checkerTexture);
    glDeleteTextures(1, &bullet);
    glDeleteTextures(1, &voltmeter);
    glDeleteTextures(1, &target);
    glDeleteTextures(1, &dead);
    glDeleteTextures(1, &panoramaG);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    //glDeleteTextures(1, &voltmeterTexture);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(unifiedShader);

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{

    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);


    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
void drawBullet(unsigned int shaderProgram, unsigned int bullet) {
    if (!shouldDrawBullet) {
        return;
    }
    unsigned int bulletCillinderVAO, bulletCillinderVBO;
    for (float i = 0; i < ammo; i++) {
        // drawRectangle(-1.55 + i / 5, -1.45 + i / 5, -0.8, -0.6, shaderProgram, 3.0f);
        drawtxt(shaderProgram, -1 + i / 9, -0.9 + i / 9, -0.8, -0.6, bullet);
    }
}

void shootBullet(unsigned int shaderProgram) {


    unsigned int bulletCillinderVAO, bulletCillinderVBO;
    for (float i = ammo / 1.0; i < 10; i++) {
        float bulletCillinderVertices[]{
        -1.55f + i / 5 , -0.6f, 0.0f, // Gornji levi ugao
         -1.45f + i / 5 , -0.6f, 0.0f, // Gornji desni ugao
         -1.45f + i / 5 , -0.8f, 0.0f, // Donji desni ugao
        -1.55f + i / 5 , -0.8f, 0.0f  // Donji levi ugao
        };




        //unsigned int rectVAO, rectVBO;


        glGenVertexArrays(1, &bulletCillinderVAO);
        glGenBuffers(1, &bulletCillinderVBO);

        glBindVertexArray(bulletCillinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, bulletCillinderVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(bulletCillinderVertices), bulletCillinderVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(bulletCillinderVAO);
        glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void updateReadyIndicator(float currentTime) {
    if (ammo > 0 && (currentTime - lastShotTime) >= cooldown) {
        canShoot = true;
    }
    else {
        canShoot = false;
    }
}

void shoot(float currentTime, std::vector<Target> &targets) {
    if (canShoot && ammo > 0) {
        ammo--;  // Smanji municiju
        lastShotTime = currentTime;
        std::cout << "Paljba!" << std::endl;

        //for (auto it = targets.begin(); it != targets.end(); ) {
        //    if (std::abs(it->x - it->targetOffset) < 0.15) {
        //        std::cout << "Pogodak" << std::endl;
        //        std::cout << "Pogodjen: " << std::to_string(it->x) << std::endl;

        //        
        //        it = targets.erase(it); // Erase i automatski pomera iterator
        //    }
        //    else {
        //        ++it;  // Ako nije pogodjena, predji na sledeci cilj
        //    }
        //}
        for (Target &t : targets) {
            //t.x + w/2 (0.1) = sredina mete
            if ((std::abs((t.x+0.1) - t.targetOffset) < 0.065) && t.isAlive) {
                t.isAlive = false;
                hit++;
            }
        }
    }
}
void initRectangle() {
    // Kreiraj koordinate pravougaonika (oko centra)
    float rectangleVertices[]{
        -0.5f,  0.5f, 0.0f, // Gornji levi ugao
         0.5f,  0.5f, 0.0f, // Gornji desni ugao
         0.5f, -0.5f, 0.0f, // Donji desni ugao
        -0.5f, -0.5f, 0.0f  // Donji levi ugao
    };

    // Generisemo VAO i VBO samo jednom
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
     //glBindVertexArray(0);
}

void drawRectangle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue) {
    initRectangle();
    glUseProgram(shaderProgram); // Koristi sejder program
    //initRectangle();
    // Prilagodjavamo koordinate pravougaonika
    float rectangleVertices[]{
        x1, y1, 0.0f, // Gornji levi ugao
         x2, y1, 0.0f, // Gornji desni ugao
         x2, y2, 0.0f, // Donji desni ugao
        x1, y2, 0.0f  // Donji levi ugao
    };

    // Binduj VAO i VBO
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rectangleVertices), rectangleVertices); // Azuriraj koordinate

    // Posaljite uniformnu promenljivu za boju
    glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), shaderValue);
    // Nacrtaj pravougaonik
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Oslobodi resurse
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // glDeleteBuffers(1, &rectVAO);
    // glDeleteVertexArrays(1, &rectVAO);
    // glDeleteProgram(shaderProgram);
}


void drawNeedle(float x1, float x2, float y1, float y2, int shaderProgram, float shaderValue, float angleDegrees) {
    glUseProgram(shaderProgram);

    // Pretvorite ugao iz stepeni u radijane
    float angleRadians = angleDegrees * (3.14f / 180.0f);

    float shakeAngle = (rand() % 5 - 2) * 0.0025f;

    if (voltage < 5) {
        shakeAngle = 0;
    }
    else if (voltage < 25) {
        shakeAngle = (rand() % 5 - 2) * 0.0025f;
    }
    else if (voltage < 60) {
        shakeAngle = (rand() % 5 - 2) * 0.0085f;
    }
    else if (voltage <= 90) {
        shakeAngle = (rand() % 5 - 2) * 0.015f;
    }

    angleRadians = angleRadians + shakeAngle;


    // Izracunajte sinus i kosinus za ugao
    float cosTheta = cos(angleRadians);
    float sinTheta = sin(angleRadians);

    // Pivot tacka (desna ivica)
    float pivotX = x2;
    float pivotY = (y1 + y2) / 2.0f;

    // Definisite vrhove u lokalnom prostoru (u odnosu na pivot)
    float localVertices[4][2] = {
        {x1 - pivotX, y1 - pivotY}, // Gornji levi
        {x2 - pivotX, y1 - pivotY}, // Gornji desni
        {x2 - pivotX, y2 - pivotY}, // Donji desni 
        {x1 - pivotX, y2 - pivotY}  // Donji levi
    };

    // Transformisani vrhovi
    float transformedVertices[4][3]; // Dodajemo Z koordinatu
    for (int i = 0; i < 4; i++) {
        transformedVertices[i][0] = localVertices[i][0] * cosTheta - localVertices[i][1] * sinTheta + pivotX;
        transformedVertices[i][1] = localVertices[i][0] * sinTheta + localVertices[i][1] * cosTheta + pivotY;
        transformedVertices[i][2] = 0.0f; // Z ostaje 0
    }

    // Binduj VAO i VBO
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(transformedVertices), transformedVertices); // Azuriraj koordinate

    // Posaljite uniformnu promenljivu za boju
    glUniform1f(glGetUniformLocation(shaderProgram, "colorSwitch"), shaderValue);

    // Nacrtaj pravougaonik
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Oslobodi resurse
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}