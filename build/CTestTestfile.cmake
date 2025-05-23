# CMake generated Testfile for 
# Source directory: D:/Github/IntelligentRecipeManagementSystem
# Build directory: D:/Github/IntelligentRecipeManagementSystem/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestRestaurant "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestRestaurant.exe")
  set_tests_properties(TestRestaurant PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;138;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestRestaurant "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestRestaurant.exe")
  set_tests_properties(TestRestaurant PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;138;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestRestaurant "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestRestaurant.exe")
  set_tests_properties(TestRestaurant PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;138;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestRestaurant "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestRestaurant.exe")
  set_tests_properties(TestRestaurant PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;138;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestRestaurant NOT_AVAILABLE)
endif()
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestRestaurantManager "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestRestaurantManager.exe")
  set_tests_properties(TestRestaurantManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;158;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestRestaurantManager "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestRestaurantManager.exe")
  set_tests_properties(TestRestaurantManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;158;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestRestaurantManager "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestRestaurantManager.exe")
  set_tests_properties(TestRestaurantManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;158;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestRestaurantManager "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestRestaurantManager.exe")
  set_tests_properties(TestRestaurantManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;158;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestRestaurantManager NOT_AVAILABLE)
endif()
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestRecipe "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestRecipe.exe")
  set_tests_properties(TestRecipe PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;196;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestRecipe "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestRecipe.exe")
  set_tests_properties(TestRecipe PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;196;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestRecipe "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestRecipe.exe")
  set_tests_properties(TestRecipe PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;196;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestRecipe "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestRecipe.exe")
  set_tests_properties(TestRecipe PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;196;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestRecipe NOT_AVAILABLE)
endif()
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestJsonRecipeRepository "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestJsonRecipeRepository.exe")
  set_tests_properties(TestJsonRecipeRepository PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;215;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestJsonRecipeRepository "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestJsonRecipeRepository.exe")
  set_tests_properties(TestJsonRecipeRepository PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;215;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestJsonRecipeRepository "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestJsonRecipeRepository.exe")
  set_tests_properties(TestJsonRecipeRepository PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;215;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestJsonRecipeRepository "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestJsonRecipeRepository.exe")
  set_tests_properties(TestJsonRecipeRepository PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;215;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestJsonRecipeRepository NOT_AVAILABLE)
endif()
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestRecipeManager "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestRecipeManager.exe")
  set_tests_properties(TestRecipeManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;233;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestRecipeManager "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestRecipeManager.exe")
  set_tests_properties(TestRecipeManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;233;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestRecipeManager "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestRecipeManager.exe")
  set_tests_properties(TestRecipeManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;233;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestRecipeManager "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestRecipeManager.exe")
  set_tests_properties(TestRecipeManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;233;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestRecipeManager NOT_AVAILABLE)
endif()
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestRecipeEncyclopediaManager "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestRecipeEncyclopediaManager.exe")
  set_tests_properties(TestRecipeEncyclopediaManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;249;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestRecipeEncyclopediaManager "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestRecipeEncyclopediaManager.exe")
  set_tests_properties(TestRecipeEncyclopediaManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;249;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestRecipeEncyclopediaManager "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestRecipeEncyclopediaManager.exe")
  set_tests_properties(TestRecipeEncyclopediaManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;249;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestRecipeEncyclopediaManager "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestRecipeEncyclopediaManager.exe")
  set_tests_properties(TestRecipeEncyclopediaManager PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;249;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestRecipeEncyclopediaManager NOT_AVAILABLE)
endif()
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestRecipeEncyclopediaCommandHandler "D:/Github/IntelligentRecipeManagementSystem/build/Debug/TestRecipeEncyclopediaCommandHandler.exe")
  set_tests_properties(TestRecipeEncyclopediaCommandHandler PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;267;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestRecipeEncyclopediaCommandHandler "D:/Github/IntelligentRecipeManagementSystem/build/Release/TestRecipeEncyclopediaCommandHandler.exe")
  set_tests_properties(TestRecipeEncyclopediaCommandHandler PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;267;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestRecipeEncyclopediaCommandHandler "D:/Github/IntelligentRecipeManagementSystem/build/MinSizeRel/TestRecipeEncyclopediaCommandHandler.exe")
  set_tests_properties(TestRecipeEncyclopediaCommandHandler PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;267;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestRecipeEncyclopediaCommandHandler "D:/Github/IntelligentRecipeManagementSystem/build/RelWithDebInfo/TestRecipeEncyclopediaCommandHandler.exe")
  set_tests_properties(TestRecipeEncyclopediaCommandHandler PROPERTIES  _BACKTRACE_TRIPLES "D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;267;add_test;D:/Github/IntelligentRecipeManagementSystem/CMakeLists.txt;0;")
else()
  add_test(TestRecipeEncyclopediaCommandHandler NOT_AVAILABLE)
endif()
subdirs("_deps/googletest-build")
subdirs("_deps/spdlog-build")
