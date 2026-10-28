//#include <QCoreApplication>

//#include "smpbcore.h"

#include <iostream>
#include <format>
#include <cstdint>

// Математика
#include <cmath>

// Ядро и его компоненты
#include <smpb/Config.h>
#include <smpb/Core.h>

#define SMGC_EXAMPLE_DIMENSION 2

#if SMGC_EXAMPLE_DIMENSION == 2
#include <smgc/display2D.h>
#include "Scene2D.h"
#else
#include <smgc/display3D.h>
#include "Scene3D.h"
#endif

int main(int argc, char *argv[]) {
    Core core = Core();
    core.setDebugModeEnabled(true);
    core.appendComponent(std::make_unique<Config>(argc, argv));
#if SMGC_EXAMPLE_DIMENSION == 2
    core.appendComponent(std::make_unique<SMGC2D>());
    core.appendComponent(std::make_unique<Scene2D>());
#else
    core.appendComponent(std::make_unique<SMGC>());
    core.appendComponent(std::make_unique<Scene3D>());
#endif

    core.init();

    int errorCode = core.cycle();

    if (errorCode == 0) {
        std::clog << "Program finished cleanly." << std::endl;
    } else {
        std::clog << "Program finished with error code " << std::to_string(errorCode) << "." << std::endl;
    }

    return errorCode;
}
