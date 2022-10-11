#pragma once
#ifndef KABLUNK_TEST_KABLUNK_UNIT_TEST_API_H
#define KABLUNK_TEST_KABLUNK_UNIT_TEST_API_H

#include "Kablunk/Core/Core.h"

namespace Kablunk::test
{

}

#ifdef KB_UNIT_TEST
#		define KB_BEGIN_UNIT_TEST_GROUP(x)	\
		{ \
			int unit_test_tests_passed = 0; \
			int unit_test_total_tests = 0; \
			const char* unit_test_test_name = x; \
			KB_CORE_TRACE("[Test] Starting tests for {}", unit_test_test_name);

#		define KB_UNIT_TEST(x) \
			{ \ 
				unit_test_total_tests++; \
				if (!(x)) \
				{ \
					KB_CORE_ERROR("[Test #{}] Failed!", unit_test_total_tests); \
				} \
				else \
				{ \
					KB_CORE_INFO("[Test #{}] Passed!", unit_test_total_tests) \
					unit_test_tests_passed++; \
				}
			}
#		define KB_END_UNIT_TEST_GROUP()	\
			KB_CORE_TRACE("[Test] Finished tests for " unit_test_test_name); \
			KB_CORE_TRACE("[Test]  {}/{}", unit_test_tests_passed, unit_test_total_tests); \
		}
#	else
#		define KB_BEGIN_UNIT_TEST(x) {}
#		define KB_UNIT_TEST(x)       {}
#		define KB_END_UNIT_TEST()    {}
#	endif

#endif
