#pragma once

#include "NetMsg.h"
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr {
	class NetMsgTextWO : public NetMsg {
		public:
			NetMsgMacroDeclaration(NetMsgTextWO);

			NetMsgTextWO(const std::string message = "");
			virtual ~NetMsgTextWO();
			virtual bool toStream(NetMessengerStreamBuffer& os) const;
			virtual bool fromStream(NetMessengerStreamBuffer& is);
			virtual void onMessageArrived();
			virtual std::string toString() const;

			std::string text;

	};
}

#endif