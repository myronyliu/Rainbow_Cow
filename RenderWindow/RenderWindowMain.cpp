// RenderWindoeMain.cpp : Creates the 3D Viewer.
//

#include "stdafx.h"
#include "GlutUI.h"

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

    //Scene::MeshObject * meshObject = new Scene::MeshObject("models/sphere.off");
    Scene::ProgressiveMeshObject * meshObject = new Scene::ProgressiveMeshObject("models/sphere.offpm");
    std::vector<float> bounds = meshObject->readGeom();

    float xSpan = bounds[1] - bounds[0];
    float ySpan = bounds[3] - bounds[2];
    float zSpan = bounds[5] - bounds[4];
    float xMid = (bounds[0] + bounds[1]) / 2;
    float yMid = (bounds[2] + bounds[3]) / 2;
    float zMid = (bounds[4] + bounds[5]) / 2;

    meshObject->setTx(-xMid);
    meshObject->setTy(-yMid);
    meshObject->setTz(-zMid);
    world.addObject(meshObject);

    float s = 0.5;
    float xMin = -s*xSpan;
    float yMin = -s*ySpan;
    float zMin = -s*zSpan;
    float xMax = s*xSpan;
    float yMax = s*ySpan;
    float zMax = s*zSpan;

    Scene::Arrow * xAxis = new Scene::Arrow(glm::vec3(xMin, yMin, zMin), glm::vec3(xMax, yMin, zMin), glm::vec4(1, 0, 0, 1));
    Scene::Arrow * yAxis = new Scene::Arrow(glm::vec3(xMin, yMin, zMin), glm::vec3(xMin, yMax, zMin), glm::vec4(0, 1, 0, 1));
    Scene::Arrow * zAxis = new Scene::Arrow(glm::vec3(xMin, yMin, zMin), glm::vec3(xMin, yMin, zMax), glm::vec4(0, 0, 1, 1));
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

    GlutUI::Controls::Keyboard keyboard(&mainPanel);
    GlutUI::Controls::Mouse mouse(&mainPanel, mainPanel.getCamera());

    Scene::Edge e(0, 1); // 0,1,3 for fin removal testing on testpatch.off
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
    auto xlambda = [&]() {
        int nCP = meshObject->nCollapsablePairs();
        int n = fmax(1, sqrt(nCP) / 2);
        printf("Quadric simplifying %i times...\n", n);
        for (int i = 0; i < n; i++) {
            meshObject->quadricSimplify();
        }
        //meshObject->quadricSimplify();
    };
    auto plambda = [&]() {
        printf("Writing progressive mesh data to %s\n", meshObject->outFileName());
        meshObject->makeProgressiveMeshFile();
    };
    //auto pluslambda = [&]() {
    //    meshObject->collapseTo(nVertices())
    //};

    keyboard.register_hotkey('a', alambda);
    keyboard.register_hotkey('s', slambda);
    keyboard.register_hotkey('z', zlambda);
    keyboard.register_hotkey('x', xlambda);
    keyboard.register_hotkey('p', plambda);

    MANAGER.drawElements();

    return 0;
}
