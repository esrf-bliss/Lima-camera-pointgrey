find_path(FLYCAPTURE_INCLUDE_DIRS flycapture/FlyCapture2.h)
find_library(FLYCAPTURE_LIBRARIES flycapture)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Flycapture
  DEFAULT_MSG
  FLYCAPTURE_LIBRARIES
  FLYCAPTURE_INCLUDE_DIRS)
