
/// Макрос для самопальных тестовых проверок. 
#define ASSERT_EQUALS(caseName, actual, expected, onFailureText) \
	std::cout << caseName << ": "; \
	if(actual == expected) \
	{ \
		std::cout << "\033[1;32m OK \033[0m" << std::endl; \
	} \
	else \
	{ \
		std::cout << "\033[1;31m FAIL! \033[0m" << onFailureText << std::endl; \
		std::cout << "actual: " << actual << std::endl; \
		std::cout << "expected: " << expected << std::endl; \
	} 