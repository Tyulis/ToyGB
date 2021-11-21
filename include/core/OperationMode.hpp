#ifndef _CORE_OPERATIONMODE_HPP
#define _CORE_OPERATIONMODE_HPP

namespace toygb {
	enum class OperationMode {
		DMG, CGB, AGB, SGB /* Unsupported */,
		Auto, /* Set based on cartridge header */
	};
}

#endif
