#include "3DViewer.h"
#include "utils/3DFigure.h"
#include "tinyfiledialogs.h"
#include <iostream>
#include <string>

using namespace std;

int main() 
{
    const char* filterPatterns[] = { "*.obj" };
    
    const char* selectedPath = tinyfd_openFileDialog(
        "Seleccionar modelo 3D", "", 1, filterPatterns, "Archivos Wavefront OBJ", 0                             
    );

    if (!selectedPath) 
    {
        cout << "Operación cancelada por el usuario." << endl;
        return 0;
    }

    string objPath = selectedPath;
    cout << "Cargando archivo: " << objPath << endl;

    string mtlPath = objPath.substr(0, objPath.find_last_of(".")) + ".mtl";
    
    C3DViewer test;
    
    if (!test.setup()) 
    {
        fprintf(stderr, "Failed to setup C3DViewer\n");
        return -1;
    }

    C3DFigure obj;
    if(obj.loadObject(objPath)){
        obj.normalization();
        test.setupModel(&obj);
    }

    test.mainLoop();

    return 0;
}