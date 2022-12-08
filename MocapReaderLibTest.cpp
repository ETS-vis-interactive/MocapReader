#include <iostream>
#include <string>
#include "BVHFormat.h"

void PrintVector( float vec[3] )
{
    std::cout << "(" << vec[0] << "," << vec[1] << "," << vec[2] << ")";
}

int main( int argc, char ** argv )
{
    if( argc != 2 )
    {
        std::cout << "USAGE: " << argv[0] << " <input bvh filename>" << std::endl;
        exit(1);
    }

    std::string mocapFilename( argv[1] );

    MOCAPHEADER header;
    MocapData * mocapfile = new BVHFormat(&header);

    if(!mocapfile->ImportData(mocapFilename.c_str()))
    {
        std::cout << "Problem reading input file " << mocapFilename << " " << mocapfile->GetError() << std::endl;
        exit(1);
    }
    
    // Print the nodes
    int nbNodes = header.noofsegments;
    NODE ** nl = mocapfile->GetNodeList();
    std::cout << "Nodes" << std::endl;
    for( int i = 0; i < nbNodes; ++i )
    {
        std::cout << nl[i]->name << "; ";
        std::cout << "len: ";
        PrintVector( nl[i]->length );
        std::cout << "; offset: ";
        PrintVector( nl[i]->offset );
        std::cout << "; euler: ";
        PrintVector( nl[i]->euler );
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Print the frames
    std::cout << "Frames" << std::endl;
    for( int i = 0; i < header.noofframes; ++i )
    {
        for( int n = 0; n < header.noofsegments; ++n )
        {
            std::cout << nl[n]->name << ": t";
            PrintVector(nl[n]->froset[i]);
            std::cout << ", r";
            PrintVector(nl[n]->freuler[i]);
            std::cout << "; ";
        }
        std::cout << std::endl;
    }
}
