#pragma once

namespace jni {
	class ICleanable {
	public:
		virtual void Cleanup() = 0;
	};
}