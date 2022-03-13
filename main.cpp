
#include"opengl.hpp"
#include <iostream>
#include"shader.hpp"
#include"object.hpp"
#include"transform.hpp"
#include"camera.hpp"
#include"direct_light.hpp"
#include"point_light.hpp"
#include"stb.hpp"
#include"render.hpp"
#include"scene.hpp"
#include"CameraController.hpp"
#include"test.hpp"
#include"reflect_probe.hpp"
#include"debugtool.hpp"
#include <crtdbg.h>
#include"spot_light.hpp"
#include"gc.hpp"
#include"light_flicker.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


Camera* camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float r = 5;

Timer globalTimer;
GCController gcController;
int main()
{
   // _CrtSetBreakAlloc(1067);
    //-----------------------

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLRender", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    //glPatchParameteri(GL_PATCH_VERTICES, 3);
    /*auto sphereObjt = CreateObject("objs/sphere.obj", false);
    glfwTerminate();
    _CrtDumpMemoryLeaks();
    return 0;*/

    test();
    Render render;
    //render.openSSAO();
    //render.openSSDO();
    //render.openAntiNoise();
    //render.openSSGI();
    Scene scene;
    scene.SetSkyBox({ "sky/right.jpg",
        "sky/left.jpg",
        "sky/top.jpg",
        "sky/bottom.jpg",
        "sky/front.jpg",
        "sky/back.jpg"
        });
    
    std::shared_ptr<Shader> pbrMat = std::make_shared<Shader>("shaders/pbr.vs", "shaders/pbr.fs");
    std::shared_ptr<Shader> gBufferShader = std::make_shared<Shader>("shaders/gbuffer.vs", "shaders/gbuffer.fs");
    std::shared_ptr<Shader> gBufferShader2 = std::make_shared<Shader>("shaders/gbuffer.vs", "shaders/gbuffer.fs");
    std::shared_ptr<Shader> defferedShader = std::make_shared<Shader>("shaders/bf.vs", "shaders/deffered.fs");
    
    
    auto sponzaObj= CreateObject("objs/sponza/sponza.obj",false);
    auto sphereObj = CreateObject("objs/sphere.obj",false);
    //auto sphereObj2 = CopyObject(sphereObj);
    render.SetDefferedShader(defferedShader);

    sponzaObj->GetComponent<Transform>()->Translate(0, 0, 0);
    sponzaObj->GetComponent<Transform>()->SetScale(glm::vec3(1, 1, 1));
    sphereObj->GetComponent<Transform>()->Translate(glm::vec3(0, 5,2));
    sphereObj->GetComponent<Transform>()->SetScale(glm::vec3(0.3, 0.3, 0.3));
    sphereObj->AddComponent<Test>();

    scene.AddObject(sponzaObj);
    scene.AddObject(sphereObj);
    //scene.AddObject(sphereObj2);

    auto cameraObj=CreateObject();
    auto cameraTrans=cameraObj->GetComponent<Transform>();
    auto camera = cameraObj->AddComponent<Camera>();
    ::camera = camera;
    cameraTrans->Translate(glm::vec3(0.f,5.f,0.f));
    scene.AddObject(cameraObj);

    auto objL = CreateObject();
    DirectLight *l=new DirectLight(Spectrum(1.f, 244.f/255.f, 179.f/255.f),30.f,glm::normalize(glm::vec3(-0.5f, -1.0f, 0.3f)),true);
    objL->AddComponent(l);
    //l->isStatic = true;
    SpotLight* sl = new SpotLight(Spectrum(1.f, 0, 0), 70, 5, Vector3f(-0.4f,-1.f,-0.3f), true);
    objL->AddComponent(sl);
    sl->pos = { 2,4,1 };
    sl->isStatic = true;
    scene.AddLight(sl);
    scene.AddLight(l);
    scene.AddObject(objL);
    //objL->AddComponent<LightFlicker>();
    auto probeObj = CreateObject();
    probeObj->GetComponent<Transform>()->Translate(0, 3.2, 5);
    auto probe = probeObj->AddComponent<ReflectProbe>();
    scene.AddObject(probeObj);
    auto probeObj2 = CreateObject();
    probeObj2->GetComponent<Transform>()->Translate(0, 3.2, -5);
    auto probe2 = probeObj2->AddComponent<ReflectProbe>();
    scene.AddObject(probeObj2);
    auto probeObj3 = CreateObject();
    probeObj3->GetComponent<Transform>()->Translate(0, 0, 0);
    auto probe3 = probeObj3->AddComponent<ReflectProbe>();
    scene.AddObject(probeObj3);

    unsigned char* tex1,*albedo,*albedo2;
    int w1,h1,channel;
    stbi_set_flip_vertically_on_load(true);
    
    gBufferShader->use();
    albedo = stbi_load("objs/tex/albedo.png", &w1, &h1, &channel, 0);
    gBufferShader->setTexture("albedoMap", albedo, w1, h1, channel);
    tex1 = stbi_load("objs/tex/normal.png", &w1, &h1, &channel, 0);
    gBufferShader->setTexture("normalMap", tex1, w1, h1, channel);
    tex1 = stbi_load("objs/tex/roughness.png", &w1, &h1, &channel, 0);
    gBufferShader->setTexture("roughnessMap", tex1, w1, h1, channel);
    tex1 = stbi_load("objs/tex/metallic.png", &w1, &h1, &channel, 0);
    gBufferShader->setTexture("metallicMap", tex1, w1, h1, channel);
    tex1 = stbi_load("objs/tex/black.png", &w1, &h1, &channel, 0);
    gBufferShader->setTexture("lightMap", tex1, w1, h1, channel);
    tex1 = stbi_load("objs/tex/black.png", &w1, &h1, &channel, 0);
    gBufferShader->setTexture("lightMapDir", tex1, w1, h1, channel);

    gBufferShader2->use();
    albedo2 = stbi_load("objs/tex/red/albedo.png", &w1, &h1, &channel, 0);
    gBufferShader2->setTexture("albedoMap", albedo2, w1, h1, channel);
    tex1 = stbi_load("objs/tex/red/normal.png", &w1, &h1, &channel, 0);
    gBufferShader2->setTexture("normalMap", tex1, w1, h1, channel);
    tex1 = stbi_load("objs/tex/red/roughness.png", &w1, &h1, &channel, 0);
    gBufferShader2->setTexture("roughnessMap", tex1, w1, h1, channel);
    /*tex1 = stbi_load("objs/tex/red/metallic.jng", &w1, &h1, &channel, 0);
    gBufferShader2->setTexture("metallicMap", tex1, w1, h1, channel);*/
    /*tex1 = stbi_load("baking/bks.png", &w1, &h1, &channel, 0);
    gBufferShader2->setTexture("lightMap", tex1, w1, h1, channel);
    tex1 = stbi_load("baking/bks_dir.png", &w1, &h1, &channel, 0);
    gBufferShader2->setTexture("lightMapDir", tex1, w1, h1, channel);*/


    sponzaObj->SetShader(-1, gBufferShader, Deffered);
    sphereObj->SetShader(-1, gBufferShader2, Deffered);
    //sphereObj2->SetShader(-1, gBufferShader, Deffered);

    defferedShader->use();
    tex1= stbi_load("baking/KullaConty/Eavg_LUT.png", &w1, &h1, &channel, 0);
    defferedShader->setTexture("uEavgLut", tex1, w1, h1, channel);
    tex1 = stbi_load("baking/KullaConty/E_LUT.png", &w1, &h1, &channel, 0);
    defferedShader->setTexture("uBRDFLut", tex1, w1, h1, channel);
    {
        defferedShader->setCubeMap("reflectCube0.reflectCube", 0);
        defferedShader->setCubeMap("reflectCube1.reflectCube", 0);
        defferedShader->setCubeMap("reflectCube2.reflectCube", 0);
        render.InitReflectProbe(&scene);
        //scene.SetSkyBox(probe3->GetCubeMap());
        defferedShader->use();
        defferedShader->setCubeMap("reflectCube0.reflectCube", probe->GetCubeMap());
        defferedShader->setBool("reflectCube0.exist", true);
        defferedShader->setVec3("reflectCube0.pos", probe->object->GetComponent<Transform>()->GetPosition());
        defferedShader->setCubeMap("reflectCube1.reflectCube", probe2->GetCubeMap());
        defferedShader->setBool("reflectCube1.exist", true);
        defferedShader->setVec3("reflectCube1.pos", probe2->object->GetComponent<Transform>()->GetPosition());
        defferedShader->setCubeMap("reflectCube2.reflectCube", probe3->GetCubeMap());
        defferedShader->setBool("reflectCube2.exist", true);
        defferedShader->setVec3("reflectCube2.pos", probe3->object->GetComponent<Transform>()->GetPosition());
    }
    int first = 0;
    
    /*scene.buildBVH();
    scene.mappingPhotons(l);
    scene.savePhotons();*/

    /*scene.readPhotons();
    scene.genLightMap(sphereObj.get(), 0);*/

    while (!glfwWindowShouldClose(window))
    {
        
        processInput(window);
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            render.openSSDO();
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            render.openSSAO();
        globalTimer.updateTime(glfwGetTime());
        render(&scene);
       
        scene.Update();
        gcController.update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    
    //_CrtDumpMemoryLeaks();
    return 0;
   
    
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, globalTimer.deltaTime*3);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, globalTimer.deltaTime *3);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, globalTimer.deltaTime * 3);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, globalTimer.deltaTime * 3);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera->ProcessMouseMovement(5, 0);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera->ProcessMouseMovement(-5, 0);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        camera->ProcessMouseMovement(0, 5);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        camera->ProcessMouseMovement(0, -5);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera->ProcessMouseScroll(yoffset);
}


