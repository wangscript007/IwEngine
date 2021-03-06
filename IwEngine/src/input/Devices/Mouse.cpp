#include "iw/input/Devices/Mouse.h"

#ifdef IW_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace iw {
namespace Input {
	Translation MouseBase::translation = CreateTranslation();

	InputName MouseBase::Translate(
		unsigned int oskey)
	{
		return translation.to(oskey);
	}

	unsigned int MouseBase::Translate(
		InputName key)
	{
		return translation.from(key);
	}

	Translation MouseBase::CreateTranslation() {
		Translation translation;

#ifdef IW_PLATFORM_WINDOWS
		translation.insert(VK_LBUTTON,  LMOUSE);
		translation.insert(VK_RBUTTON,  RMOUSE);
		translation.insert(VK_MBUTTON,  MMOUSE);
		translation.insert(VK_XBUTTON1, XMOUSE1);
		translation.insert(VK_XBUTTON2, XMOUSE2);
#endif

		return translation;
	}

}
}
