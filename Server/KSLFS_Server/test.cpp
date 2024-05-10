#include <iostream>
#include <string>



const char* strs[] = {
    ""
};

void PrintChildren(const char* str)
{
    printf("\t\t{\n");
    printf("\t\t\t"Name": " % s",\n\n", str);
    printf("\t\t\t"Transform": \n");
    printf("\t\t\t{\n");
    printf("\t\t\t\t"Position": [0.0, 0.0, 0.0], \n\t"Rotate": [0.0, 0.0, 0.0], \n\t"Scale" : [1.0, 1.0, 1.0] \n");
    printf("\t\t\t},\n");
    printf("\t\t\t"Collider": \n");
    printf("\t\t\t{\n");
    printf("\t\t\t\t"Static": true, \n\t"AutoMesh": true\n");
    printf("\t\t\t},\n");
    printf("\t\t\t"Renderer": \n");
    printf("\t\t\t{\n");
    printf("\t\t\t\t"Mesh": " % s. % s", \n\t"Material": "NoMaterial"\n", "map-E(main)3", str);
    printf("\t\t\t}\n");
    printf("\t\t},\n");
}
int main()
{
    for (int i = 0; i < _countof(strs); ++i) {
        PrintChildren(strs[i]);
    }

}