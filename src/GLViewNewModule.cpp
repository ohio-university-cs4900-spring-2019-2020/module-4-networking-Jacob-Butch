#include "GLViewNewModule.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "Camera.h"
#include "NewModuleWayPoints.h"
#include "Model.h"
#include "AftrGLRendererBase.h"
#include <irrKlang.h>
#include "ISoundManager.h"
#include "WOFTGLString.h"
#include "NetMessengerClient.h"
#include "NetMsgTextWO.h"
#include "NetMsgWarthog.h"


using namespace Aftr;
using namespace irrklang;

SoundEndListener* soundEndListener;
const std::string sharedMM = ManagerEnvironmentConfiguration::getSMM();

GLViewNewModule* GLViewNewModule::New( const std::vector< std::string >& args ){   
    GLViewNewModule* glv = new GLViewNewModule( args );
    glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
    glv->onCreate();
    return glv;
}

void GLViewNewModule::init(float gravityScalar, Vector gravityNormalizedVector, std::string configFileName, const PHYSICS_ENGINE_TYPE& physicsEngineType) {
    ISoundManager::init();
    GLView::init(gravityScalar, gravityNormalizedVector, configFileName, physicsEngineType);
}

GLViewNewModule::GLViewNewModule( const std::vector< std::string >& args ) : GLView( args ){
    //Initialize any member variables that need to be used inside of LoadMap() here.
    //Note: At this point, the Managers are not yet initialized. The Engine initialization occurs immediately after this method returns 
    //(see GLViewNewModule::New() for reference). Then the engine invoke's GLView::loadMap() for this module.
    //After loadMap() returns, GLView::onCreate is finally invoked.
    //The order of execution of a module startup: GLView::New() is invoked: calls GLView::init() -> GLView::loadMap() -> GLView::onCreate()
    //GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
}

void GLViewNewModule::onCreate(){
    //GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
    if( this->pe != NULL ){
        //optionally, change gravity direction and magnitude here. The user could load these values from the module's aftr.conf
        this->pe->setGravityNormalizedVector( Vector( 0, 0,-1.0f ) );
        this->pe->setGravityScalar( Aftr::GRAVITY );
    }
    this->setActorChaseType(cctNUM_CAM_CHASE_TYPE); //Default is STANDARDEZNAV mode

    this->warthog = WOWarthog::New();
    worldLst->push_back(this->warthog->getWarthogWO());
    soundEndListener = new SoundEndListener();
    this->citySound = ISoundManager::getEngine()->play2D("../mm/sounds/citysounds.ogg", true);  

    this->startingText = "Hit the '/' key to type a message here";
    this->worldText = this->getInitialWorldText();
    worldLst->push_back(this->worldText);

    if (ManagerEnvironmentConfiguration::getVariableValue("NetServerListenPort") == "12683") {
        this->client = NetMessengerClient::New("127.0.0.1", "12682");
    } else {
        this->client = NetMessengerClient::New("127.0.0.1", "12683");
    }
    
}

GLViewNewModule::~GLViewNewModule(){ ISoundManager::drop(); }

void GLViewNewModule::updateWorld(){
    GLView::updateWorld();  
    ISoundManager::setListenerPosition(this->cam->getPosition(), this->cam->getLookDirection(), Vector(0, 0, 0), this->cam->getNormalDirection());
    if (this->isDriving()) {
        this->warthog->update();
    }
    if (this->client->isTCPSocketOpen()) {
        if (this->worldText->getText() != this->startingText) {
            this->client->sendNetMsgSynchronousTCP(NetMsgTextWO(this->worldText->getText()));
        }
        if (this->isDriving()) {
            this->client->sendNetMsgSynchronousTCP(NetMsgWarthog(this->warthog->getPosition(), this->warthog->getLookDirection()));
        }
    }
}

void GLViewNewModule::onKeyDown( const SDL_KeyboardEvent& key ){
    if (!this->typing) {
        GLView::onKeyDown(key);
        if (this->isDriving()) { this->warthog->onKeyDown(key); }
    }
    this->processKeyPress(key.keysym.sym);
}

void GLViewNewModule::onKeyUp(const SDL_KeyboardEvent& key) {
    GLView::onKeyUp(key);
    if (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_RSHIFT) { this->shift = false; }
    if (this->isDriving()) { this->warthog->onKeyUp(key); }   
}

void GLViewNewModule::onResizeWindow(GLsizei width, GLsizei height) { GLView::onResizeWindow(width, height); }
void GLViewNewModule::onMouseDown(const SDL_MouseButtonEvent& e) {
    GLView::onMouseDown(e);
    if (this->isDriving()) { this->warthog->onMouseDown(e); }
}

void GLViewNewModule::onMouseUp(const SDL_MouseButtonEvent& e) {
    GLView::onMouseUp(e);
    if (this->isDriving()) { this->warthog->onMouseUp(e); }
}

void GLViewNewModule::onMouseMove(const SDL_MouseMotionEvent& e) {
    GLView::onMouseMove(e);
    if (this->isDriving()) { this->warthog->onMouseMove(e); }
}

void GLViewNewModule::onMouseWheelScroll(const SDL_MouseWheelEvent& e) {
    GLView::onMouseWheelScroll(e);
    if (this->isDriving()) { this->warthog->onMouseWheelScroll(e); }
}

void Aftr::GLViewNewModule::loadMap(){
    

    this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
    this->actorLst = new WorldList();
    this->netLst = new WorldList();

    ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
    ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
    ManagerOpenGLState::enableFrustumCulling = false;
    Axes::isVisible = false;
    this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+
    this->cam->setPosition( -25, 0, 15 );
    
    std::string grass(sharedMM + "/models/grassFloor400x400_pp.wrl" );
   
    //SkyBox Textures readily available
    std::vector< std::string > skyBoxImageNames; //vector to store texture paths

    skyBoxImageNames.push_back(sharedMM + "/images/skyboxes/space_thick_rb+6.jpg");
    {
        float ga = 0.1f; //Global Ambient Light level for this module
        ManagerLight::setGlobalAmbientLight(aftrColor4f(ga, ga, ga, 1.0f));
        WOLight* light = WOLight::New();
        light->isDirectionalLight(true);
        light->setPosition(Vector(0, 0, 100));
        //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
        //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
        light->getModel()->setDisplayMatrix(Mat4::rotateIdentityMat({ 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD));
        light->setLabel("Light");
        worldLst->push_back(light);
    }
    
    //Create the SkyBox
    WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
    wo->setPosition(Vector(0, 0, 0));
    wo->setLabel("Sky Box");
    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo);
    
    //wo = WO::New(sharedMM + "/models/citytexday_3ds/city_tex_day.3DS", Vector(1, 1, 1), MESH_SHADING_TYPE::mstAUTO);
    //wo->setPosition(Vector(0, 0, 1350));
    //wo->renderOrderType = RENDER_ORDER_TYPE::roOVERLAY;
    //wo->setLabel("City");
    //worldLst->push_back(wo);
    
    //wo = WO::New("../mm/models/toilet/model.dae", Vector(10, 10, 10), MESH_SHADING_TYPE::mstAUTO);
    //wo->setPosition(10, 10, 10);
    //wo->setLabel("Toilet");
    //worldLst->push_back(wo);
    
    createNewModuleWayPoints();
}

void GLViewNewModule::createNewModuleWayPoints() {
    // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
    WayPointParametersBase params(this);
    params.frequency = 5000;
    params.useCamera = true;
    params.visible = false;
    WOWayPointSpherical* wayPt = WOWP1::New(params, 3);
    wayPt->setPosition(Vector(50, 0, 3));
    worldLst->push_back(wayPt);
}

void GLViewNewModule::processKeyPress(const SDL_Keycode& key) {
    if (this->typing) {
        switch (key) {
            case SDLK_SLASH:
                this->typing = false;    
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                if (!this->shift) {
                    this->shift = true;
                }
                break;
            case SDLK_RETURN:
                // send message
                client->sendNetMsgSynchronousTCP(NetMsgTextWO(this->worldText->getText()));
                break;
            case SDLK_ESCAPE:
                this->typing = false;
                break;
            default:
                this->type(key);
        }
        return;
    }

    switch (key) {
        case SDLK_SLASH:
            this->typing = true;        
            if (this->worldText->getText() == this->startingText) {
                this->worldText->setText("");
            }
            return;
        case SDLK_f:
            if (this->warthog != nullptr) {
                if (this->cam != nullptr && !this->warthog->hasDriver()) {
                    this->warthog->setDriver(this->cam);
                    this->cam->attachCameraToWO(this->warthog->getWarthogWO(), this->warthog->getPosition());
                } else {
                    this->warthog->setDriver(nullptr);
                    this->cam->detachCameraFromWO();
                }
            }
            break;
        case SDLK_w:
        case SDLK_UP:
            if (!this->isDriving() && this->cam != nullptr) {
                this->cam->moveInLookDirection(10);
            }
            break;
        case SDLK_a:
        case SDLK_LEFT:
            if (!this->isDriving() && this->cam != nullptr) {
                for (int i = 0; i < 10; i++) { this->cam->moveLeft(); }
            }
            break;
        case SDLK_s:
        case SDLK_DOWN:
            if (!this->isDriving() && this->cam != nullptr) {
                this->cam->moveOppositeLookDirection(10);
            }
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            if (!this->isDriving() && this->cam != nullptr) {
                for (int i = 0; i < 10; i++) { this->cam->moveRight(); }
            }
            break;
        case SDLK_l:
            if(this->cam != nullptr) {
                std::cout << "Camera:  " << this->cam->getPosition() << std::endl;
                if (this->warthog != nullptr) {
                    std::cout << "Warthog: " << this->warthog->getPosition() << std::endl << std::endl;
                    std::cout << "Warthog Look Rad: " << this->warthog->getLookDirection() << std::endl;
                    std::cout << "Warthog Look Deg: " << this->warthog->lookAngle << std::endl;
                    std::cout << "Warthog Look Yaw: " << this->warthog->yaw << std::endl;
                }
            }
            break;
        case SDLK_0:
            if(this->cam != nullptr) this->cam->setPosition(Vector(0, 0, 0));
            break;
        case SDLK_1:
            if (NetMessengerClient::New("127.0.0.1", "12682")->isTCPSocketOpen()) {
                std::cout << "12682 is OPEN" << std::endl;
            } else {
                std::cout << "12682 is CLOSED" << std::endl;
            }
            break;
        case SDLK_2:
            if (NetMessengerClient::New("127.0.0.1", "12683")->isTCPSocketOpen()) {
                std::cout << "12683 is OPEN" << std::endl;
            } else {
                std::cout << "12683 is CLOSED" << std::endl;
            }
            break;
        case SDLK_c:
            this->cam->attachCameraToWO(this->warthog->getWarthogWO(), this->warthog->getPosition());
            break;
        case SDLK_o:
            ISoundManager::getEngine()->play2D("../mm/sounds/oof.mp3");
            break;
        default: break;
    }
}

void GLViewNewModule::type(const SDL_Keycode& key) {
    std::string text = this->worldText->getText();
    if (key == SDLK_BACKSPACE) {
        if (text.size() < 2) {
            text = "";
        }
        else {
            text.pop_back();
        }
    } else if (this->shift) {
        text += this->getShiftedChar(key);
    }
    else {
        text += key;
    }
    this->worldText->setText(text);
}

char GLViewNewModule::getShiftedChar(char c) {
    if (c < 123 && c > 96) {
        return (c - 32);
    }
    switch (c) {
        case 39: return 34; // ' -> "
        case 44: return 60; // , -> <
        case 45: return 95; // - -> _
        case 46: return 62; // . -> >
        case 47: return 63; // / -> ?
        case 48: return 41; // 0 -> )
        case 49: return 33; // 1 -> !
        case 50: return 64; // 2 -> @
        case 51: return 35; // 3 -> #
        case 52: return 36; // 4 -> $
        case 53: return 37; // 5 -> %
        case 54: return 94; // 6 -> ^
        case 55: return 38; // 7 -> &
        case 56: return 42; // 8 -> *
        case 57: return 40; // 9 -> (
        case 59: return 58; // ; -> :
        case 61: return 43; // = -> +
        case 91: return 123;// [ -> {
        case 92: return 124;// \ -> |
        case 93: return 125;// ] -> }
        case 96: return 126;// ` -> ~
        default: return c;
    }
}

bool GLViewNewModule::isDriving() {
    return this->warthog != nullptr && this->warthog->hasDriver();
}

WOFTGLString* GLViewNewModule::getInitialWorldText() {
    WOFTGLString* worldString = WOFTGLString::New(sharedMM + "/fonts/arial.ttf", 72);
    worldString->setPosition(750, 0, 100);
    worldString->setLabel("World Text");
    worldString->setText(this->startingText);
    worldString->getModel()->setLookDirection(Vector(0, -1, 0));
    worldString->getModel()->setNormalDirection(Vector(-1, 0, 0));
    worldString->getModel()->setScale(Vector(1000, 1000, 1000));
    return worldString;
}

/*
std::string GLViewNewModule::getBackground() {
    switch (background) {
    case 0: return "/images/skyboxes/sky_water+6.jpg";
    case 1: return "/images/skyboxes/sky_dust+6.jpg";
    case 2: return "/images/skyboxes/sky_mountains+6.jpg";
    case 3: return "/images/skyboxes/sky_winter+6.jpg";
    case 4: return "/images/skyboxes/early_morning+6.jpg";
    case 5: return "/images/skyboxes/sky_afternoon+6.jpg";
    case 6: return "/images/skyboxes/sky_cloudy+6.jpg";
    case 7: return "/images/skyboxes/sky_cloudy3+6.jpg";
    case 8: return "/images/skyboxes/sky_day+6.jpg";
    case 9: return "/images/skyboxes/sky_day2+6.jpg";
    case 10: return "/images/skyboxes/sky_deepsun+6.jpg";
    case 11: return "/images/skyboxes/sky_evening+6.jpg";
    case 12: return "/images/skyboxes/sky_morning+6.jpg";
    case 13: return "/images/skyboxes/sky_morning2+6.jpg";
    case 14: return "/images/skyboxes/sky_noon+6.jpg";
    case 15: return "/images/skyboxes/sky_warp+6.jpg";
    case 16: return "/images/skyboxes/space_Hubble_Nebula+6.jpg";
    case 17: return "/images/skyboxes/space_gray_matter+6.jpg";
    case 18: return "/images/skyboxes/space_easter+6.jpg";
    case 19: return "/images/skyboxes/space_hot_nebula+6.jpg";
    case 20: return "/images/skyboxes/space_ice_field+6.jpg";
    case 21: return "/images/skyboxes/space_lemon_lime+6.jpg";
    case 22: return "/images/skyboxes/space_milk_chocolate+6.jpg";
    case 23: return "/images/skyboxes/space_solar_bloom+6.jpg";
    case 24: return "/images/skyboxes/space_thick_rb+6.jpg";
    default:
        background = 0;
        return getBackground();
    }
}
*/

////Create the infinite grass plane (the floor)
/*
wo = WO::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
wo->setPosition( Vector( 0, 0, 0 ) );
wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
grassSkin.getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
wo->setLabel( "Grass" );
worldLst->push_back( wo );
*/