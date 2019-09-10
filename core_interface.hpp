#ifndef __CORE_INTERFACE__
#define __CORE_INTERFACE__
#include <string_view>
#include "ocgapi.h"

namespace YGOpen
{

class CoreInterface
{
public:
	CoreInterface(std::string_view absFilePath);
	~CoreInterface();
	
#define OCGFUNC(ret, f, params) ret (*f) params;
#include "ocgapi_funcs.h"
#undef OCGFUNC
private:
	void* handle{nullptr};
};

} // namespace YGOpen

#endif // __CORE_INTERFACE__
