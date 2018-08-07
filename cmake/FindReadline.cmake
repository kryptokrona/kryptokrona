# Search for the path containing library's headers
find_path(Readline_ROOT_DIR
    NAMES include/readline/readline.h
)

# Search for include directory
find_path(Readline_INCLUDE_DIR
    NAMES readline/readline.h
    HINTS ${Readline_ROOT_DIR}/include
)

# Search for library
find_library(Readline_LIBRARY
    NAMES readline
    HINTS ${Readline_ROOT_DIR}/lib
)

# Conditionally set READLINE_FOUND value
if(Readline_INCLUDE_DIR AND Readline_LIBRARY AND Ncurses_LIBRARY)
  set(READLINE_FOUND TRUE)
else()
  FIND_LIBRARY(Readline_LIBRARY NAMES readline)
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Readline DEFAULT_MSG 
    Readline_INCLUDE_DIR Readline_LIBRARY )
  MARK_AS_ADVANCED(Readline_INCLUDE_DIR Readline_LIBRARY)
endif()

# Hide these variables in cmake GUIs
mark_as_advanced(
    Readline_ROOT_DIR
    Readline_INCLUDE_DIR
    Readline_LIBRARY
)

# Apple uses the system readline library rather than GNU Readline, which doesn't
# support the stuff we need.
if (APPLE AND READLINE_FOUND AND Readline_INCLUDE_DIR EQUAL "/usr/include")
    message(FATAL_ERROR "Readline library found, but it is using the apple version of readline rather than GNU Readline.\nTo fix this, run:\nbrew install readline; brew link --force readline\nAlternatively, run:\ncmake .. -DFORCE_READLINE=FALSE\nTo disable readline support")
endif()
