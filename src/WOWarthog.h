#pragma once

#include "WO.h"
#include "Model.h"
#include <irrKlang.h>

#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr {

	class WOWarthog {
		public:
			static WOWarthog* New(
				const std::string modelFileName = "../mm/models/warthog/warthog.obj", 
				Vector scale = Vector(4, 4, 4),
				MESH_SHADING_TYPE shadingType = MESH_SHADING_TYPE::mstSMOOTH,
				std::string label = "Warthog",
				Vector pos = Vector(13, -17, -4)
			);
			~WOWarthog();

			//User Keyboard Input Specific
			//-----------------------------------
			void onKeyDown(const SDL_KeyboardEvent& key);
			void onKeyUp(const SDL_KeyboardEvent& key);
			void onMouseWheelScroll(const SDL_MouseWheelEvent& e);
			void onMouseDown(const SDL_MouseButtonEvent& e);
			void onMouseUp(const SDL_MouseButtonEvent& e);
			void onMouseMove(const SDL_MouseMotionEvent& e);

			void setSpeed(float newSpeed) { this->speed = newSpeed; }
			float getSpeed() { return this->speed; }

			Vector getPosition() { return warthog->getPosition(); }
			Vector getLookDirection() { return warthog->getLookDirection(); };
			Vector getNormalDirection() { return warthog->getNormalDirection(); };
			void setPosition(const Vector& newXYZ) { warthog->setPosition(newXYZ); };
			void setPosition(float x, float y, float z) { warthog->setPosition(Vector(x, y, z)); };
			void setLookDirection(const Vector& lookDirection) { warthog->getModel()->setLookDirection(lookDirection); }
			void setNormalDirection(const Vector& normalDirection) { warthog->getModel()->setNormalDirection(normalDirection); }
			void moveRelative(const Vector& dxdydz) { warthog->moveRelative(dxdydz); };
			// Moves the warthog forward or backwards by the distance
			void drive(float distance = 0.1f);
			void reverse(float distance = 0.1f);
			// Proccess the keys pressed
			void update();
			bool hasDriver() { return driver != nullptr; };
			void setDriver(Camera* newDriver);
			Camera* getDriver() { return driver; }
			WO* getWarthogWO() { return warthog; };

			bool isMoving();
			float lookAngle = 0;
			float yaw = 0;

			irrklang::ISound* drivingSound;
			irrklang::ISound* horn;

			float toRads(float deg);
			float toDeg(float rad);

		protected:
			WO* warthog;
			Camera* driver;
			std::set<SDL_Keycode> keysPressed;
			float speed, roll;
			float distanceFromWarthog = 32.28f;
			float angleAround = 0;
			float pitch = 16.1892f;
			bool honking;
			void calculateZoom(const SDL_MouseWheelEvent& e);
			float calculateHorizontal();
			float calculateVertical();
			void calculateCameraPos(float horizontal, float vertical);
			void calculateLookAngle();

			WOWarthog(
				const std::string modelFileName = "../mm/models/warthog/warthog.obj",
				Vector scale = Vector(4, 4, 4),
				MESH_SHADING_TYPE shadingType = MESH_SHADING_TYPE::mstSMOOTH,
				std::string label = "Warthog",
				Vector pos = Vector(13, -17, -4)
			);

		private:
			bool isMovementKey(SDL_Keycode key);
	};

}

#endif

