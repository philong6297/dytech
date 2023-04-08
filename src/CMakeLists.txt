set(TARGET_FOLDERS base core http log third_party)
foreach(folder ${TARGET_FOLDERS})
  add_subdirectory(${folder})
endforeach()
