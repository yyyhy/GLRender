
#include"opengl.hpp"
#include <iostream>
#include"shader.hpp"
#include"object.hpp"
#include"transform.hpp"
#include"camera.hpp"
#include"directLight.hpp"
#include"pointLight.hpp"
#include"render.hpp"
#include"scene.hpp"
#include"CameraController.hpp"
#include"test.hpp"
#include"reflectProbe.hpp"
#include"debugTool.hpp"
#include <crtdbg.h>
#include"spotLight.hpp"
#include"gc.hpp"
#include"lightFlicker.hpp"
#include"fitLTC.hpp"
#include"sphereLight.hpp"
#include"SceneManager.hpp"
#include"input.hpp"
#include"SDF.hpp"
#include"computeShader.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

float r = 5;

Timer globalTimer;
GCController gcController;
SceneManager sceneManager;

int main()
{
   // _CrtSetBreakAlloc(1067);
    //-----------------------

    GL_INIT;
    #ifdef __APPLE__ 
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	#endif 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLRender", NULL, NULL);
    GL_CHECK_CREATE_WINDOW(window);
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    GL_CHECK_GLAD;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_3D);
    sceneManager.init();

    //ComputeShader cs("shaders/cs/test.csr");
    //return 0;

    //glPatchParameteri(GL_PATCH_VERTICES, 3);
    /*auto sphereObjt = CreateObject("objs/sphere.obj", false);
    glfwTerminate();
    _CrtDumpMemoryLeaks();
    return 0;*/

    Render render;
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
    
    auto sponzaObj= CreateObject("objs/sponza/sponza.obj");
    auto sphereObj = CreateObject("objs/sphere.obj");
    render.SetDefferedShader(defferedShader);
    //sponzaObj->buildBVH();

    //sponzaObj->GetComponent<Transform>()->Translate(0, 0, 0);
    //sponzaObj->GetComponent<Transform>()->SetScale(glm::vec3(1, 1, 1));
    sphereObj->GetComponent<Transform>()->Translate(glm::vec3(0, 5,2));
    sphereObj->GetComponent<Transform>()->SetScale(glm::vec3(0.3, 0.3, 0.3));
    sphereObj->AddComponent<Test>();

    scene.AddObject(sponzaObj);
    scene.AddObject(sphereObj);

    auto cameraObj=CreateObject();
    auto cameraTrans=cameraObj->GetComponent<Transform>();
    auto camera = cameraObj->AddComponent<Camera>();
    cameraTrans->Translate(glm::vec3(0.f,4.f,0.f));
    scene.AddObject(cameraObj);

    auto objL = CreateObject();
    DirectLight *l=new DirectLight(Spectrum(1.f, 1.f, 1.f),500000.f,glm::normalize(glm::vec3(-0.5f, -1.0f, 0.3f)),true);
    objL->AddComponent(l);
    //l->isStatic = true;
    SpotLight* sl = new SpotLight(Spectrum(1.f, 0, 0), 70, 5, Vector3f(-0.4f,-1.f,-0.3f), true);
    objL->AddComponent(sl);
    sl->pos = { 2,4,1 };
    sl->isStatic = true;
    //scene.AddLight(sl);
    scene.AddLight(l);
    scene.AddObject(objL);
   /* SphereLight* spl1 = new SphereLight(Spectrum(1, 1, 1), 50);
    spl1->position = { 0,1,0 };
    spl1->radius = 0.2;
    scene.AddLight(spl1);*/
    /*SphereLight* spl2 = new SphereLight(Spectrum(1, 0, 1), 50);
    spl2->index = 1;
    spl2->position = { 3,1,0 };
    spl2->radius = 0.2;
    scene.AddLight(spl2);*/
    /*auto o = CreateObject();
    o->GetComponent<Transform>()->SetPosition({0,1,0});
    o->GetComponent<Transform>()->SetScale({ 0.2,0.2,0.2 });
    auto s = std::make_shared<Shader>("shaders/pbr.vs", "shaders/photon.fs");
    s->use();
    s->setVec3("col", {5,5,5});
    o->SetShader(-1, s);
    scene.AddObject(o);*/

    auto o2 = CreateObject();
    o2->GetComponent<Transform>()->SetPosition({ 3,1,0 });
    o2->GetComponent<Transform>()->SetScale({ 0.2,0.2,0.2 });
    auto s2 = std::make_shared<Shader>("shaders/pbr.vs", "shaders/photon.fs");
    s2->use();
    s2->setVec3("col", { 5,0,5 });
    o2->SetShader(-1, s2);
    scene.AddObject(o2);
    

    auto probeObj = CreateObject();
    probeObj->GetComponent<Transform>()->Translate(0, 5.2, 5);
    auto probe = probeObj->AddComponent<ReflectProbe>();
    scene.AddObject(probeObj);
    auto probeObj2 = CreateObject();
    probeObj2->GetComponent<Transform>()->Translate(0, 5.2, -5);
    auto probe2 = probeObj2->AddComponent<ReflectProbe>();
    scene.AddObject(probeObj2);
    auto probeObj3 = CreateObject();
    probeObj3->GetComponent<Transform>()->Translate(0, 5, 0);
    auto probe3 = probeObj3->AddComponent<ReflectProbe>();
    scene.AddObject(probeObj3);

    stbi_set_flip_vertically_on_load(true);
   
    gBufferShader->setTexture("albedoMap", "objs/tex/albedo.png");
    /*auto t = gBufferShader->GetTexture("albedoMap");
    uint8_t* tt = new uint8_t[250 * 250 * 4];
    for (int i = 0; i < 250; i++) {
        for (int j = 0; j < 250; j++) {
            unsigned u = i/250.f * t.w;
            unsigned v = j/250.f * t.h;
            Vector4i pos = { 4 * (t.w * v + u) ,4 * (t.w * v + u) + 1 ,
                             4 * (t.w * v + u) + 2,4 * (t.w * v + u) + 3 };
            Vector4f col = { t.albedo[pos.x],
                            t.albedo[pos.y],
                            t.albedo[pos.z],
                            t.albedo[pos.w]
                            };
            tt[4 * (j * 250 + i)]   = col.x;
            tt[4 * (j * 250 + i)+1] = col.y;
            tt[4 * (j * 250 + i)+2] = col.z;
            tt[4 * (j * 250 + i)+3] = col.w;
        }
    }
    stbi_write_png("0.png", 250, 250, 4, tt, 0);*/
    gBufferShader->setTexture("normalMap", "objs/tex/normal.png");
    gBufferShader->setTexture("roughnessMap", "objs/tex/roughness.png");
    gBufferShader->setTexture("metallicMap", "objs/tex/metallic.png");
    
    gBufferShader2->setTexture("albedoMap", "objs/tex/red/albedo.png");
    gBufferShader2->setTexture("normalMap", "objs/tex/red/normal.png");
    gBufferShader2->setTexture("roughnessMap", "objs/tex/red/roughness.png");

    sponzaObj->SetShader(-1, gBufferShader, Deffered);
    sphereObj->SetShader(-1, gBufferShader2, Deffered);
    //sphereObj2->SetShader(-1, gBufferShader, Deffered);

    defferedShader->setTexture("uEavgLut", "baking/KullaConty/Eavg_LUT.png");
    defferedShader->setTexture("uBRDFLut", "baking/KullaConty/E_LUT.png");
    
    /*scene.buildBVH();
    Texture3D t3D = scene.genSDF();
    defferedShader->use();
    defferedShader->setTexture("uSDF", t3D);*/
   
    {
        defferedShader->setCubeMap("reflectCube[0].reflectCube", 0);
        defferedShader->setCubeMap("reflectCube[1].reflectCube", 0);
        defferedShader->setCubeMap("reflectCube[2].reflectCube", 0);
        defferedShader->setCubeMap("reflectCube[3].reflectCube", 0);
        render.InitReflectProbe(&scene);
        scene.SetSkyBox(probe3->GetCubeMap().id);
        defferedShader->use();
        defferedShader->setCubeMap("reflectCube[0].reflectCube", probe->GetCubeMap().id);
        defferedShader->setBool("reflectCube[0].exist", true);
        defferedShader->setVec3("reflectCube[0].pos", probe->object->GetComponent<Transform>()->GetPosition());
        defferedShader->setCubeMap("reflectCube[1].reflectCube", probe2->GetCubeMap().id);
        defferedShader->setBool("reflectCube[1].exist", true);
        defferedShader->setVec3("reflectCube[1].pos", probe2->object->GetComponent<Transform>()->GetPosition());
        defferedShader->setCubeMap("reflectCube[2].reflectCube", probe3->GetCubeMap().id);
        defferedShader->setBool("reflectCube[2].exist", true);
        defferedShader->setVec3("reflectCube[2].pos", probe3->object->GetComponent<Transform>()->GetPosition());
    }
    /*defferedShader->use();
    defferedShader->setBool("lightMapOff", true);*/

    /*scene.buildBVH();
    scene.mappingPhotons(l,100000);
    scene.savePhotons("baking/photon.txt");*/

    //scene.readPhotons("baking/photon.txt");
    //auto ps = scene.getPhotons();
    //std::vector<Vector3f> pos;
    //std::cout << ps.size() << "\n";
    //for (int i = 0; i < 50; i++) {
    //    auto p = ps[i];
    //    /*if (p.times > 1)
    //        continue;*/
    //    //std::cout << p.positon.x << " " << p.positon.y << " " << p.positon.z << "\n";
    //    auto o = CreateObject("objs/sphere.obj");
    //    o->GetComponent<Transform>()->SetPosition(p.positon);
    //    o->GetComponent<Transform>()->SetScale({ 0.2,0.2,0.2 });
    //    auto s= std::make_shared<Shader>("shaders/pbr.vs", "shaders/photon.fs");
    //    s->use();
    //    s->setVec3("col", p.color);
    //    o->SetShader(-1,s);
    //    scene.AddObject(o);
    //    pos.push_back(p.positon);
    //}
    
    //genLTC_Lut(16);
    //return 0;

    //scene.readPhotons("baking/photon.txt");
    //for (int i = 0; i < 6; i++) {
    //    scene.genLightMapIR(sponzaObj.get(), i, "./baking/lightMap/main_light/bk");
    //}
    //    
    ///*for(int i=22;i<40;++i)
    //    scene.genLightMap(sponzaObj.get(), i, "./baking/lightMap/main_light/bk");*/
    //sponzaObj->LoadLightMapData();

    //return 0;

    RENDER_MAIN_LOOP(window)
    {
        
        processInput(window);
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            render.openSSDO();
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            render.openSSAO();
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
            render.openSSGI();
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
            render.CapturePostProcessOutput(0);
        globalTimer.updateTime(glfwGetTime());
        scene.Update();
        render(&scene);
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
        mainCamera->ProcessKeyboard(FORWARD, globalTimer.deltaTime*3);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mainCamera->ProcessKeyboard(BACKWARD, globalTimer.deltaTime *3);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mainCamera->ProcessKeyboard(LEFT, globalTimer.deltaTime * 3);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mainCamera->ProcessKeyboard(RIGHT, globalTimer.deltaTime * 3);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        mainCamera->ProcessMouseMovement(5, 0);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        mainCamera->ProcessMouseMovement(-5, 0);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        mainCamera->ProcessMouseMovement(0, 5);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        mainCamera->ProcessMouseMovement(0, -5);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    //glViewport(0, 0, width, height);
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

    mainCamera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    mainCamera->ProcessMouseScroll(yoffset);
}


