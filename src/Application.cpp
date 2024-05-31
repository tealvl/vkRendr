#include "Application.hpp"

void Application::run(){
    init();
    mainLoop();
}

void Application::init(){
    rendr::RendererConfig renderConfig;
    renderConfig.deviceConfig.deviceEnableFeatures.setSamplerAnisotropy(true);
    renderer.init(renderConfig, window);
    
    renderer.initMaterial(material);
    MeshWithTextureObj walls(material);
    MeshWithTextureObj details(material);
    rendr::STBImageRaii wallsTex("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_walls_baked.png");  
    rendr::STBImageRaii detailsTex("C:/Dev/cpp-projects/engine/resources/zen-studio/textures/t_details_Baked.png");  
    walls.loadTexture(std::move(wallsTex), renderer);    
    details.loadTexture(std::move(detailsTex), renderer);
    rendr::UfbxSceneRaii fbxScene("C:/Dev/cpp-projects/engine/resources/zen-studio/source/room.fbx");
    auto meshesAndMatInd = rendr::ufbxLoadMeshesPartsSepByMaterial(fbxScene.get());  
    auto fbxMatToMesh = rendr::mergeMeshesByMaterial(meshesAndMatInd);
    walls.loadMesh(fbxMatToMesh[0], renderer);
    details.loadMesh(fbxMatToMesh[2], renderer);

    objsToDraw.push_back(std::move(walls));
    objsToDraw.push_back(std::move(details));

    inputManager.setUpWindowCallbacks(window);
    camManip.setCamera(camera);
    camManip.setInputManager(inputManager);
}

void Application::mainLoop(){
    while (!window.shouldClose()) {
        window.pollEvents();
        timer.update();
        
        //TODO вынести 
        if(inputManager.isKeyPressed('Q')){
            camManip.enableMouseCameraControl();
            window.disableCursor();  
        }
        if(inputManager.isKeyPressed('E')){
            camManip.disableMouseCameraControl();
            window.enableCursor();
        }

        camManip.update(timer.getDeltaTime());
        inputManager.resetInputOffsets();

        rendr::MVPUniformBufferObject ubo = camera.getMVPubo(0.01f, renderer.getSwapChainAspect());
        renderer.updateUniformBuffer(ubo);
        
        std::vector<rendr::IDrawableObj*> objToDrawOnThisFrame;
        for(auto& obj : objsToDraw){
            objToDrawOnThisFrame.push_back(&obj);
        }

        renderer.setDrawableObjects(objToDrawOnThisFrame);
        renderer.drawFrame();
    }
    renderer.waitIdle();
}
