# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
    OF_ROOT=../../.local/lib/of_v0.10.1_osx_release/../../.local/lib/of_v0.10.1_osx_release/../workspace/of_v0.10.1_osx_release/../../.local/lib/of_v0.10.1_osx_release/../../.local/lib/of_v0.10.1_osx_release/../workspace/of_v0.10.1_osx_release/../workspace/of_v0.10.1_osx_release
endif

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

# embed assets so the .app is self-contained (runs from /Applications)
# macOS-only (cp -R bundle layout + plutil) — a no-op elsewhere: the
# Windows package is a plain exe + data/ + runtime DLLs (see the
# build-win64 job in .github/workflows/release.yml).
ifeq ($(shell uname -s),Darwin)
after:
	rm -rf bin/Starships.app/Contents/Resources/data
	cp -R bin/data bin/Starships.app/Contents/Resources/data
	cp Starships.icns bin/Starships.app/Contents/Resources/of.icns
	plutil -replace CFBundleIconFile -string of.icns bin/Starships.app/Contents/Info.plist
endif
