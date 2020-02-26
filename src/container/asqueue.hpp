#include "__enigma.h"
__ENIGMA_BEGIN__
template<class _Ty>
class asqueue {
public:
	using value_type = typename std::decay<_Ty>::type;
};
__ENIGMA_END__