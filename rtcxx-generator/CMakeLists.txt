set(LLVM_LINK_COMPONENTS support)

add_clang_executable(rtcxx-generator
  Main.cpp
  ReflMatchFinder.h
  )

set_property(TARGET rtcxx-generator PROPERTY CXX_STANDARD 20)

target_link_libraries(rtcxx-generator
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )
 

target_compile_definitions(rtcxx-generator PRIVATE REFL_GENERATOR)
#target_compile_definitions(rtcxx-generator PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

#_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
