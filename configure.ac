AC_INIT(README)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(giraffe, 0.9)
AC_PROG_CC

SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL >= $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, , AC_MSG_ERROR([
*** Unable to find SDL_image libary with PNG support
(http://www.libsdl.org/projects/SDL_image/)
]))

AC_CHECK_LIB(SDL_gfx, pixelColor, , AC_MSG_ERROR([
*** Unable to find SDL_gfx library
]))

AC_CHECK_LIB(SDL_mixer, Mix_Linked_Version, , AC_MSG_ERROR([
*** Unable to find SDL_mixer library
]))


AC_OUTPUT(Makefile src/Makefile)
