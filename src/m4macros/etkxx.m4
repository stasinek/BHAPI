dnl Configure paths for ETK++
dnl Anthony Lee 2004

dnl AM_PATH_ETKXX([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for ETK++, and define ETKXX_CFLAGS and ETKXX_CXXFLAGS and ETKXX_LIBS
dnl
AC_DEFUN([AM_PATH_ETKXX],
[dnl 
dnl Get the cflags and libraries from etkxx-config
dnl
AC_ARG_ENABLE(etkxx-test, [  --disable-etkxx-test      do not try to compile and run a test ETK++ program],
		    , enable_etkxx_test=yes)

  AC_PATH_PROG(ETKXX_CONFIG, etkxx-config, no)

  no_etkxx=""

  min_etkxx_version=ifelse([$1], ,0.1.15,$1)
  AC_MSG_CHECKING(for ETK++ - version >= $min_etkxx_version)

  if test x$ETKXX_CONFIG != xno ; then
    if $ETKXX_CONFIG --atleast-version $min_etkxx_version; then
	  :
    else
	  no_etkxx=yes
    fi
  fi

  if test x"$no_etkxx" = x ; then
    ETKXX_CPPFLAGS=`$ETKXX_CONFIG --cflags`
    ETKXX_LIBS=`$ETKXX_CONFIG --libs`
    etkxx_config_major_version=`$ETKXX_CONFIG --version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    etkxx_config_minor_version=`$ETKXX_CONFIG --version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    etkxx_config_micro_version=`$ETKXX_CONFIG --version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS

    if test "x$enable_etkxx_test" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $ETKXX_CPPFLAGS"
      LIBS="$ETKXX_LIBS $LIBS"
dnl
dnl Now check if the installed ETK++ is sufficiently new. (Also sanity
dnl checks the results of etkxx-config to some extent)
dnl
      rm -f conf.etkxxtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>

#include <etk/support/String.h>
#include <etk/support/StringArray.h>

int
main()
{
	euint8 major, minor, micro;
	char *tmp_version;

	system("touch conf.etkxxtest");

	EString tmpVersion("$min_etkxx_version");
	EStringArray *array = tmpVersion.Split('.');
	if(array == NULL || array->CountItems() != 3 ||
	   array->ItemAt(0)->GetInteger(&major) == false ||
	   array->ItemAt(1)->GetInteger(&minor) == false ||
	   array->ItemAt(2)->GetInteger(&micro) == false)
	{
		printf("%s, bad version string\n", "$min_etkxx_version");
		exit(1);
	}

	if(etk_major_version != $etkxx_config_major_version ||
	   etk_minor_version != $etkxx_config_minor_version ||
	   etk_micro_version != $etkxx_config_micro_version)
	{
		printf("\n*** 'etkxx-config --version' returned %d.%d.%d, but ETK++ (%d.%d.%d) was found!\n", 
		       $etkxx_config_major_version, $etkxx_config_minor_version, $etkxx_config_micro_version,
		       etk_major_version, etk_minor_version, etk_micro_version);
	}
	else if(etk_major_version != ETK_MAJOR_VERSION ||
		etk_minor_version != ETK_MINOR_VERSION ||
		etk_micro_version != ETK_MICRO_VERSION)
	{
		printf("*** ETK++ header files (version %d.%d.%d) do not match\n",
		       ETK_MAJOR_VERSION, ETK_MINOR_VERSION, ETK_MICRO_VERSION);
		printf("*** library (version %d.%d.%d)\n",
		       etk_major_version, etk_minor_version, etk_micro_version);
	}
	else
	{
		if(etk_major_version > major ||
		   (etk_major_version == major && etk_minor_version > minor) ||
		   (etk_major_version == major && etk_minor_version == minor && etk_micro_version >= micro))
		{
			return 0;
		}
		else
		{
			printf("\n*** An old version of ETK++ (%d.%d.%d) was found.\n",
			       etk_major_version, etk_minor_version, etk_micro_version);
			printf("*** You need a version of ETK++ newer than %d.%d.%d. The latest version of\n",
			       major, minor, micro);
			printf("***\n");
		}
	}

	return 1;
}
],, no_etkxx=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_etkxx" = x ; then
     AC_MSG_RESULT(yes (version $etkxx_config_major_version.$etkxx_config_minor_version.$etkxx_config_micro_version))
     ifelse([$2], , :, [$2])
  else
     AC_MSG_RESULT(no)
     if test "$ETKXX_CONFIG" = "no" ; then
       echo "*** etkxx-config was not found."
     else
       if test -f conf.etkxxtest ; then
        :
       else
          echo "*** Could not run ETK++ test program, checking why..."
          CXXFLAGS="$CXXFLAGS $ETKXX_CPPFLAGS"
          LIBS="$LIBS $ETKXX_LIBS"
          AC_TRY_LINK([
#include <etk/support/SupportDefs.h>
#include <stdio.h>
],      [ return ((etk_major_version) || (etk_minor_version) || (etk_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding ETK++ or finding the wrong"
          echo "*** version of ETK++. If it is not finding ETK++, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means ETK++ was incorrectly installed"
          echo "*** or that you have moved ETK++ since it was installed." ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     ETKXX_CPPFLAGS=""
     ETKXX_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_LANG_RESTORE

  ETKXX_CFLAGS=$ETKXX_CPPFLAGS
  ETKXX_CXXFLAGS=$ETKXX_CPPFLAGS
  AC_SUBST(ETKXX_CFLAGS)
  AC_SUBST(ETKXX_CXXFLAGS)
  AC_SUBST(ETKXX_LIBS)
  rm -f conf.etkxxtest
])

