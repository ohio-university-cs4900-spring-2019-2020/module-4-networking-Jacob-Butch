#include "NetMsgTextWO.h"
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
#include "WOFTGLString.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgTextWO);

NetMsgTextWO::NetMsgTextWO(const std::string message){ this->text = message; }

NetMsgTextWO::~NetMsgTextWO() {}

bool NetMsgTextWO::toStream(NetMessengerStreamBuffer& os) const {
	os << this->text;

	return true;
}

bool NetMsgTextWO::fromStream(NetMessengerStreamBuffer& is) {
	is >> this->text;
	return true;
}

void NetMsgTextWO::onMessageArrived() {
	((GLViewNewModule*)ManagerGLView::getGLView())->worldText->setText(this->text);

	std::cout << this->toString() << std::endl;
}

std::string NetMsgTextWO::toString() const {
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "   Message: " << this->text << "...\n";

	return ss.str();
}

#endif