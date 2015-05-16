// RenderWindoeMain.cpp : Creates the 3D Viewer.
//

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
    file = fopen(fileName, "wb");
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
}


GlutUI::Manager MANAGER;

int main(int argc, char* argv[])
{
    MANAGER.init(argc, argv);

    int windowWidth = 500;
    int windowHeight = 500;
    GlutUI::Window & mainWindow = MANAGER.createWindow(windowWidth, windowHeight, "Render Window");
    GlutUI::Panel & mainPanel = MANAGER.createPanel(mainWindow, windowWidth, windowHeight, "Render Panel");
    Scene::World world = Scene::createWorld();
    std::cout << std::string((char *)glGetString(GL_VENDOR)) << std::endl;
    std::cout << std::string((char *)glGetString(GL_RENDERER)) << std::endl;
    std::cout << "OpenGL " << std::string((char *)glGetString(GL_VERSION)) << std::endl;
    std::cout << "====================================================" << std::endl;

    Scene::Shader* rainbowShader = new Scene::Shader("shaders/rainbow_vert.glsl", "shaders/rainbow_frag.glsl");

    std::string file = "bunny";
    Scene::MeshObject * meshObject = new Scene::MeshObject("models/" + file + ".off");
    //Scene::MeshObject * meshObject = new Scene::MeshObject("models/" + file + ".offpm");
    //meshObject->allowFins();
    world.assignShader(meshObject, rainbowShader);
    meshObject->readGeom();
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
    world.addObject(meshObject);

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
    s = 2;
    Scene::Camera * cam = new Scene::Camera();
    if (xSpan == minSpan) {
        cam->setThe(-90);
        //cam->setTx(zMid);
        //cam->setTy(-yMid);
        cam->setTz(xMid + s * maxSpan);
    }
    else if (ySpan == minSpan) {
        cam->setPhi(90);
        cam->setThe(90);
        //cam->setTx(-zMid);
        //cam->setTy(-xMid);
        cam->setTz(yMid + s * maxSpan);
    }
    else {
        //cam->setTx(-xMid);
        //cam->setTy(-yMid);
        cam->setTz(zMid + s * maxSpan);
    }
    world.addObject(cam);

    mainPanel.setWorld(&world);
    mainPanel.setCamera(cam);
    mainPanel.setMeshObject(meshObject);

    GlutUI::Controls::Keyboard keyboard(&mainPanel);
    GlutUI::Controls::Mouse mouse(&mainPanel, mainPanel.getCamera(), mainPanel.getMeshObject());

    std::pair<int,int> e(0, 1); // 0,1,3 for fin removal testing on testpatch.off
    /* Keyboard hotkey assignments */
    auto alambda = [&]() {
        e = meshObject->randomEdge();
    };
    auto slambda = [&]() {
        meshObject->collapse(e.first, e.second, Scene::MIDPOINT_APPROXIMATION_METHOD);
    };
    auto zlambda = [&]() {
        int nVV = meshObject->nVisibleVertices();
        int n = fmax(1, nVV / 100);
        printf("Random edge midpoint collapsing %i times...\n", n);
        for (int i = 0; i < n; i++) meshObject->collapseRandomEdge();
    };
    int qsCount = 0;
    auto xlambda = [&]() {
        //meshObject->quadricSimplify();
        int nCP = meshObject->nCollapsablePairs();
        int n = sqrt(nCP) / 2;
        if (n == 0) n = fmin(nCP, 1);
        //n = 1;
        printf("%i  ", n);
        for (int i = 0; i < n; i++) {
            meshObject->quadricSimplify();
        }
        qsCount += n;
        //printf(" total quadric Simplifications: %i\n", qsCount);*/
    };
    auto plambda = [&]() {
        printf("Writing progressive mesh data to %s\n", meshObject->outFileName());
        meshObject->makeProgressiveMeshFile();
    };
    auto pluslambda = [&]() {
        meshObject->collapseTo(meshObject->complexity() + .1);
    };
    auto minuslambda = [&]() {
        meshObject->collapseTo(meshObject->complexity() - .1);
    };
    keyboard.register_hotkey('=', pluslambda);
    keyboard.register_hotkey('-', minuslambda);
    keyboard.register_hotkey('a', alambda);
    keyboard.register_hotkey('s', slambda);
    keyboard.register_hotkey('z', zlambda);
    keyboard.register_hotkey('x', xlambda);
    keyboard.register_hotkey('p', plambda);

    MANAGER.drawElements();

    return 0;
}
