#include "WOWarthog.h"
#ifdef AFTR_CONFIG_USE_BOOST

#include "Camera.h"
#include "Model.h"
#include <complex> 
#include "ISoundManager.h"
#include "Mat4Fwd.h"

using namespace Aftr;

WOWarthog* WOWarthog::New(const std::string modelFileName, Vector scale, MESH_SHADING_TYPE shadingType, std::string label, Vector pos) {
	//WOWarthog* wow = new WOWarthog(modelFileName, scale, shadingType, label, pos);
	return new WOWarthog(modelFileName, scale, shadingType, label, pos);
}

WOWarthog::WOWarthog(const std::string modelFileName, Vector scale, MESH_SHADING_TYPE shadingType, std::string label, Vector pos) {
	this->speed = 1;
	this->roll = 0;
	this->driver = nullptr;
	this->warthog = WO::New(modelFileName, scale, shadingType);
	this->warthog->setLabel(label);
	this->warthog->setPosition(pos);
	this->drivingSound = ISoundManager::getEngine()->play3D("../mm/sounds/warthog/drive.wav", ISoundManager::toVec3df(Vector(0, -17, -4)), true, true, true);
	this->drivingSound->setMinDistance(30);
	this->horn = ISoundManager::getEngine()->play3D("../mm/sounds/warthog/horn.wav", ISoundManager::toVec3df(Vector(0, -17, -4)), true, true, true);
	this->horn->setMinDistance(30);
	this->honking = false;
}

WOWarthog::~WOWarthog() {
	if (this->drivingSound != nullptr) {
		this->drivingSound->drop();
	}
	if (this->horn != nullptr) {
		this->horn->drop();
	}
}

void WOWarthog::setDriver(Camera* driver) {
	this->driver = driver;
	if (driver != nullptr) {
		this->driver->setPosition(this->getPosition() + Vector(-31, 0, this->calculateVertical()));
		this->driver->setParentWorldObject(this->warthog);
	} else {
		this->keysPressed.clear();
		if (this->drivingSound != nullptr && !this->drivingSound->getIsPaused()) {
			this->drivingSound->setIsPaused(true);
		}
		if (this->horn != nullptr && !this->horn->getIsPaused()) {
			this->horn->setIsPaused(true);
		}
	}
}

void WOWarthog::onKeyDown(const SDL_KeyboardEvent& key){
	SDL_Keycode keyDown = key.keysym.sym;
	if (keyDown == SDLK_PLUS || keyDown == SDLK_EQUALS) {
		if (this->speed < 10) {
			++this->speed;
		} else if (this->speed < 0 || this->speed > 10) {
			//reset speed in case it becomes too large or negative
			this->speed = 0;
		}
		//else do nothing for now
	}
	if (keyDown == SDLK_MINUS) {
		if (this->speed > 0) {
			--this->speed;
		} else if (this->speed < 0 || this->speed > 10) {
			//reset speed in case it becomes too large or negative
			this->speed = 0;
		}
		//else do nothing for now
	}
	if(isMovementKey(keyDown)) {
		std::set<SDL_Keycode>::iterator found = keysPressed.find(keyDown);
		if (found == keysPressed.end()) {
			keysPressed.insert(keyDown);
		}
	}
	if (keyDown == SDLK_SPACE && this->horn != nullptr) {
		if (!honking) {
			this->horn->setIsPaused(false);
			honking = true;
		}
	}
	if (keyDown == SDLK_p) {
		float vertical = this->calculateVertical();
		float horizontal = this->calculateHorizontal();
		float theta = this->toDeg(this->getLookDirection().y) + angleAround;
		float offsetX = horizontal * std::sin(this->toRads(theta));
		float offsetY = horizontal * std::cos(this->toRads(theta));
		Vector pos = this->getPosition();
		std::cout << "Vertical:\t" << vertical << std::endl;
		std::cout << "Horizontal:\t" << horizontal << std::endl;
		std::cout << "Theta:\t\t" << theta << std::endl;
		std::cout << "Offset X:\t" << offsetX << std::endl;
		std::cout << "Offset Y:\t" << offsetY << std::endl;
	}
}

void WOWarthog::onKeyUp(const SDL_KeyboardEvent& key) {
	SDL_Keycode keyUp = key.keysym.sym;
	if (isMovementKey(keyUp)) {
		std::set<SDL_Keycode>::iterator found = keysPressed.find(keyUp);
		if (found != keysPressed.end()) {
			keysPressed.erase(found);
		}
	} 
	if (keyUp == SDLK_SPACE && this->horn != nullptr) {
		if (honking) {
			this->horn->setIsPaused(true);
			honking = false;
		}
	}
}

void WOWarthog::onMouseDown(const SDL_MouseButtonEvent& e) { 
	//if (e.button == 0) { this->pitch -= (e.y * 0.1f); } 
	//else if (e.button == 1) { this->angleAround = (e.x * 0.3f); }
}

void WOWarthog::onMouseWheelScroll(const SDL_MouseWheelEvent& e) { this->calculateZoom(e); }
void WOWarthog::onMouseUp(const SDL_MouseButtonEvent& e) {}
void WOWarthog::onMouseMove(const SDL_MouseMotionEvent& e) {}

void WOWarthog::calculateZoom(const SDL_MouseWheelEvent& e) {
	float zoomLevel = e.direction * 0.1f;
	this->distanceFromWarthog -= zoomLevel;
}

float WOWarthog::calculateHorizontal() {
	return this->distanceFromWarthog * std::cos(this->toRads(this->pitch));
}

float WOWarthog::calculateVertical() {
	return this->distanceFromWarthog * std::sin(this->toRads(this->pitch));
}

void WOWarthog::calculateCameraPos(float horizontal, float vertical) {
	float theta = this->toDeg(this->getLookDirection().y) + angleAround;
	float offsetX = horizontal * std::sin(this->toRads(theta));
	float offsetY = horizontal * std::cos(this->toRads(theta));
	Vector pos = this->getPosition();
	if (this->hasDriver()) {
		//this->driver->setPosition(Vector(pos.x - offsetX, pos.y - offsetY, pos.z + vertical));
		this->driver->setPosition(pos.x - 31, pos.y + 0, pos.z + vertical);
	}
	this->yaw = 180 - (this->toDeg(this->getLookDirection().y) + angleAround);
}

void WOWarthog::update() {
	for (std::set<SDL_Keycode>::iterator it = this->keysPressed.begin(); it != this->keysPressed.end(); ++it) {
		if (*it == SDLK_UP || *it == SDLK_w) {
			this->drive(this->speed);
		}
		if (*it == SDLK_LEFT || *it == SDLK_a) {
			this->warthog->rotateAboutRelZ(0.1f);
		}
		if (*it == SDLK_DOWN || *it == SDLK_s) {
			this->reverse(this->speed);
		}
		if (*it == SDLK_RIGHT || *it == SDLK_d) {
			this->warthog->rotateAboutRelZ(-0.1f);
		}
	}
	if (this->drivingSound != nullptr) {
		if (this->isMoving()) {
			if (this->drivingSound->getIsPaused()) {
				this->drivingSound->setIsPaused(false);
			}
		}
		else {
			if (!this->drivingSound->getIsPaused()) {
				this->drivingSound->setIsPaused(true);
			}
		}
		this->drivingSound->setPosition(ISoundManager::toVec3df(this->getPosition()));
	}
	if (this->horn != nullptr) {
		this->horn->setPosition(ISoundManager::toVec3df(this->getPosition()));
	}
	this->driver->attachCameraToWO(this->warthog, this->warthog->getPosition() + Vector(0, 0, 7));
	//this->calculateCameraPos(this->calculateHorizontal(), this->calculateVertical());
	//this->calculateLookAngle();
}

void WOWarthog::drive(float distance) {
	for (int i = 0; i < distance; ++i) {
		this->moveRelative(this->getLookDirection());
	}
}

void WOWarthog::reverse(float distance) {
	for (int i = 0; i < distance; ++i) {
		this->moveRelative(this->getLookDirection() * -1);
	}
}

bool WOWarthog::isMovementKey(SDL_Keycode key) {
	return (key == SDLK_UP || key == SDLK_LEFT || key == SDLK_DOWN || key == SDLK_RIGHT ||
		key == SDLK_w || key == SDLK_a || key == SDLK_s || key == SDLK_d);
}

void WOWarthog::calculateLookAngle() {
	Vector look = this->getLookDirection();
	float angle = std::atan2(look.y, look.x) * (180 / Aftr::PI);
	//Adjust negative angles to positve
	if (angle < 0) {
		angle += 360;
	}
	//Reverse angle to find angle pointing opposite front of warthog
	if (angle < 180) {
		angle += 180;
	}
	else {
		angle -= 180;
	}
	this->lookAngle = angle;
}

bool WOWarthog::isMoving() {
	return this->keysPressed.find(SDLK_UP) != this->keysPressed.end()
		|| this->keysPressed.find(SDLK_w) != this->keysPressed.end()
		|| this->keysPressed.find(SDLK_DOWN) != this->keysPressed.end()
		|| this->keysPressed.find(SDLK_s) != this->keysPressed.end();
}

float WOWarthog::toRads(float deg) {
	return std::tan((deg * Aftr::PI) / 180);
}

float WOWarthog::toDeg(float rad) {
	return rad * (180 / Aftr::PI);
}

#endif