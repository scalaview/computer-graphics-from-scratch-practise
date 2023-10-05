@ECHO OFF
REM Build Everything

ECHO "Building..."


@REM PUSHD raytracer
@REM CALL build.bat
@REM POPD
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

@REM PUSHD raster
@REM CALL build.bat
@REM POPD
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

REM raytracer
make -f "Makefile.raytracer" clean
make -f "Makefile.raytracer" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

REM raster_triangle
make -f "Makefile.raster_triangle" clean
make -f "Makefile.raster_triangle" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "All assemblies built successfully."