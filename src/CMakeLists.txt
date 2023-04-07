set(TARGET_FOLDERS base core http log)
foreach(folder ${TARGET_FOLDERS})
  add_subdirectory(${folder})
endforeach()
