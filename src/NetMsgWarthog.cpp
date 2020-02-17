#include "NetMsgWarthog.h"
#ifdef AFTR_CONFIG_USE_BOOST

#include <iostream>
#include <sstream>
#include <string>
#include "AftrManagers.h"
#include "Vector.h"
#include "WO.h"
#include "GLView.h"
#include "WorldContainer.h"
#include "Model.h"
#include "WOLight.h"
#include "GLViewNewModule.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgWarthog);

NetMsgWarthog::NetMsgWarthog(const Vector& pos, const Vector& dir) {
	this->position = pos;
	this->direction = dir;
}

NetMsgWarthog::~NetMsgWarthog() {}

bool NetMsgWarthog::toStream(NetMessengerStreamBuffer& os) const {
	os << this->position.x << this->position.y << this->position.z;
	os << this->direction.x << this->direction.y << this->direction.z;

	return true;
}

bool NetMsgWarthog::fromStream(NetMessengerStreamBuffer& is) {
	is >> this->position.x >> this->position.y >> this->position.z;
	is >> this->direction.x >> this->direction.y >> this->direction.z;
	return true;
}

void NetMsgWarthog::onMessageArrived() {
	((GLViewNewModule*)ManagerGLView::getGLView())->warthog->setPosition(this->position);
	((GLViewNewModule*)ManagerGLView::getGLView())->warthog->setLookDirection(this->direction);

	std::cout << this->toString() << std::endl;
}

std::string NetMsgWarthog::toString() const {
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "\tPosition:  " << this->position << "...\n";
	ss << "\tDirection: " << this->direction << "...\n";
	return ss.str();
}

#endif