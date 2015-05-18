// RenderWindoeMain.cpp : Creates the 3D Viewer.

#include "stdafx.h"
#include "GlutUI.h"
void SaveAsBMP(const char *fileName)
{
    FILE *file;
    unsigned long imageSize;
    GLbyte *data = NULL;
    GLint viewPort[4];
    GLenum lastBuffer;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    bmfh.bfType = 'MB';
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = 54;
    glGetIntegerv(GL_VIEWPORT, viewPort);
    imageSize = ((viewPort[2] + ((4 - (viewPort[2] % 4)) % 4))*viewPort[3] * 3) + 2;
    bmfh.bfSize = imageSize + sizeof(bmfh) + sizeof(bmih);
    data = (GLbyte*)malloc(imageSize);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SWAP_BYTES, 1);
    glGetIntegerv(GL_READ_BUFFER, (GLint*)&lastBuffer);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, viewPort[2], viewPort[3], GL_BGR, GL_UNSIGNED_BYTE, data);
    data[imageSize - 1] = 0;
    data[imageSize - 2] = 0;
    glReadBuffer(lastBuffer);
    std::FILE* f;
    fopen_s(&f, fileName, "wb");
    file = f;
    bmih.biSize = 40;
    bmih.biWidth = viewPort[2];
    bmih.biHeight = viewPort[3];
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = 0;
    bmih.biSizeImage = imageSize;
    bmih.biXPelsPerMeter = 45089;
    bmih.biYPelsPerMeter = 45089;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;
    fwrite(&bmfh, sizeof(bmfh), 1, file);
    fwrite(&bmih, sizeof(bmih), 1, file);
    fwrite(data, imageSize, 1, file);
    free(data);
    fclose(file);
}//
GlutUI::Manager MANAGER;
int main(int argc, char* argv[])
{
    MANAGER.init(argc, argv);
    int windowWidth = 256;
    int windowHeight = 256;
    GlutUI::Window & mainWindow = MANAGER.createWindow(windowWidth, windowHeight, "Render Window");
    GlutUI::Panel & mainPanel = MANAGER.createPanel(mainWindow, windowWidth, windowHeight, "Render Panel");
    Scene::World world = Scene::createWorld();
    std::cout << std::string((char *)glGetString(GL_VENDOR)) << std::endl;
    std::cout << std::string((char *)glGetString(GL_RENDERER)) << std::endl;
    std::cout << "OpenGL " << std::string((char *)glGetString(GL_VERSION)) << std::endl;
    std::cout << "====================================================" << std::endl;

    Scene::Shader* rainbowShader = new Scene::Shader("shaders/rainbow_vert.glsl", "shaders/rainbow_frag.glsl");

    //char* fileName = argv[1];


    char* fileName = "models/cow.offpm";

    Scene::MeshObject* meshObject = new Scene::MeshObject(fileName);
    world.assignShader(meshObject, rainbowShader);
    meshObject->readGeom();
    world.addObject(meshObject);

    float xMin = meshObject->xMin();
    float xMax = meshObject->xMax();
    float yMin = meshObject->yMin();
    float yMax = meshObject->yMax();
    float zMin = meshObject->zMin();
    float zMax = meshObject->zMax();
    float xSpan = xMax - xMin;
    float ySpan = yMax - yMin;
    float zSpan = zMax - zMin;
    float xMid = (xMin + xMax) / 2;
    float yMid = (yMin + yMax) / 2;
    float zMid = (zMin + zMax) / 2;
    meshObject->setTx(-xMid);
    meshObject->setTy(-yMid);
    meshObject->setTz(-zMid);
    float s = 0.5;
    float xAxisMin = -s*xSpan;
    float yAxisMin = -s*ySpan;
    float zAxisMin = -s*zSpan;
    float xAxisMax = s*xSpan;
    float yAxisMax = s*ySpan;
    float zAxisMax = s*zSpan;
    Scene::Arrow * xAxis = new Scene::Arrow(glm::vec3(xAxisMin, yAxisMin, zAxisMin), glm::vec3(xAxisMax, yAxisMin, zAxisMin), glm::vec4(1, 0, 0, 1));
    Scene::Arrow * yAxis = new Scene::Arrow(glm::vec3(xAxisMin, yAxisMin, zAxisMin), glm::vec3(xAxisMin, yAxisMax, zAxisMin), glm::vec4(0, 1, 0, 1));
    Scene::Arrow * zAxis = new Scene::Arrow(glm::vec3(xAxisMin, yAxisMin, zAxisMin), glm::vec3(xAxisMin, yAxisMin, zAxisMax), glm::vec4(0, 0, 1, 1));
    world.addObject(xAxis);
    world.addObject(yAxis);
    world.addObject(zAxis);
    float maxSpan = std::max({ xSpan, ySpan, zSpan });
    float minSpan = std::min({ xSpan, ySpan, zSpan });
    s = 1.5;
    Scene::Camera * cam = new Scene::Camera();
    if (xSpan == minSpan) {
        cam->setThe(-90);
        cam->setTz(xMid + s * maxSpan);
    }
    else if (ySpan == minSpan) {
        cam->setPhi(90);
        cam->setThe(90);
        cam->setTz(yMid + s * maxSpan);
    }
    else {
        cam->setTz(zMid + s * maxSpan);
    }
    world.addObject(cam);
    mainPanel.setWorld(&world);
    mainPanel.setCamera(cam);
    mainPanel.setMeshObject(meshObject);
    GlutUI::Controls::Keyboard keyboard(&mainPanel);
    GlutUI::Controls::Mouse mouse(&mainPanel, mainPanel.getCamera(), mainPanel.getMeshObject());
    ///////////////////////////////////////
    ///// Keyboard Hotkey Assignments /////
    ///////////////////////////////////////
    int bmpCounter = 0;
    float complexityMultiplier = 1.0 / 32.0;
    float complexityIncrement = 1.0 / 16.0;
    auto alambda = [&]() {
        if (meshObject->format() == "off") {
            meshObject->collapseRandomEdge(Scene::MIDPOINT_APPROXIMATION_METHOD);
        }
    };
    auto slambda = [&]() {
        if (meshObject->format() == "off") {
            int nVV = meshObject->nVisibleVertices();
            int n = fmax(1, nVV / 100);
            printf("Random edge midpoint collapsing %i times...\n", n);
            for (int i = 0; i < n; i++) meshObject->collapseRandomEdge(Scene::MIDPOINT_APPROXIMATION_METHOD);
        }
    };
    auto zlambda = [&]() {
        if (meshObject->format() == "off"){
            meshObject->quadricSimplify();
        }
    };
    auto xlambda = [&]() {
        if (meshObject->format() == "off") {
            int nCP = meshObject->nCollapsablePairs();
            int n = sqrt(nCP) / 2;
            if (n == 0) n = fmin(nCP, 1);
            printf("%i  ", n);
            for (int i = 0; i < n; i++) {
                meshObject->quadricSimplify();
            }
        }
    };
    auto nlambda = [&]() {
        printf("Writing progressive mesh data to %s\n", meshObject->outFileName());
        meshObject->makeProgressiveMeshFile();
    };
    auto mlambda = [&]() {
        if (meshObject->format() == "off") {
            int collapseCount = 0;
            for (int i = 0; i < meshObject->nCollapsablePairs(); i++) {
                if (collapseCount % 10 == 0) printf("Pairs Collapsed: %i\r", collapseCount);
                meshObject->quadricSimplify();
                collapseCount++;
            }
            printf("Pairs Collapsed: %i\n", collapseCount);
            printf("Making Progressive Mesh File: %s\n", meshObject->outFileName());
            meshObject->makeProgressiveMeshFile();
        }
    };
    auto pluslambda = [&]() {
        //std::string bmpName = fileName + std::to_string(bmpCounter) + ".bmp";
        //SaveAsBMP(bmpName.c_str());
        //bmpCounter++;
        if (meshObject->format() == "offpm") meshObject->collapseTo((1.0f + complexityMultiplier)*meshObject->complexity());
    };
    auto minuslambda = [&]() {
        if (meshObject->format() == "offpm") meshObject->collapseTo((1.0 - complexityMultiplier)*meshObject->complexity());
    };
    auto rbracketlambda = [&]() {
        if (meshObject->format() == "offpm") {
            meshObject->collapseTo(meshObject->complexity() + complexityIncrement);
        }
    };
    auto lbracketlambda = [&]() {
        if (meshObject->format() == "offpm") meshObject->collapseTo(meshObject->complexity() - complexityIncrement);
    };
    auto ilambda = [&]() {
        std::string bmpName = fileName + std::to_string(bmpCounter) + ".bmp";
        SaveAsBMP(bmpName.c_str());
        bmpCounter++;
    };
    auto quotelambda = [&]() {
        complexityMultiplier += 1.0 / 256.0;
        printf("Complexity Multiplier:_%f_____\r", complexityMultiplier);
    };
    auto colonlambda = [&]() {
        complexityMultiplier = fmax(0.0, complexityMultiplier - 1.0 / 256);
        printf("Complexity Multiplier:_%f_____\r", complexityMultiplier);
    };
    auto slashlambda = [&]() {
        complexityIncrement += 1.0 / 256.0;
        printf("Complexity Increment:__%f_____\r", complexityIncrement);
    };
    auto periodlambda = [&]() {
        complexityIncrement = fmin(0.0, complexityIncrement - 1.0 / 256.0);
        printf("Complexity Increment:__%f_____\r", complexityIncrement);
    };
    auto tlambda = [&]() {
        meshObject->toggleDrawMode();
    };
    auto ylambda = [&]() {
        meshObject->toggleCustomColors();
    };
    keyboard.register_hotkey('\'', quotelambda);
    keyboard.register_hotkey(';', colonlambda);
    keyboard.register_hotkey('/', slashlambda);
    keyboard.register_hotkey('.', periodlambda);
    keyboard.register_hotkey(']', rbracketlambda);
    keyboard.register_hotkey('[', lbracketlambda);
    keyboard.register_hotkey('i', ilambda);
    keyboard.register_hotkey('=', pluslambda);
    keyboard.register_hotkey('-', minuslambda);
    keyboard.register_hotkey('a', alambda);
    keyboard.register_hotkey('s', slambda);
    keyboard.register_hotkey('x', xlambda);
    keyboard.register_hotkey('z', zlambda);
    keyboard.register_hotkey('t', tlambda);
    keyboard.register_hotkey('y', ylambda);
    keyboard.register_hotkey('n', nlambda);
    keyboard.register_hotkey('m', mlambda);

    MANAGER.drawElements();

    return 0;
}
