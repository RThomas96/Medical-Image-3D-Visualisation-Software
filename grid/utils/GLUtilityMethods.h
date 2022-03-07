#ifndef GLUTILITYMETHODS_H
#define GLUTILITYMETHODS_H

#include <QGLViewer/qglviewer.h>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>
#include <math.h>
using std::isspace;



namespace RGB
{
    extern float color[30][3];
    extern float color4[30][4];
    extern int nColor;


    void calc_RGB( float val , float val_min , float val_max , float & r , float & g , float & b );
    void get_random_RGB_from_HSV( float & r , float & g , float & b );
    void get_random_RGB_from_HSV( float & r , float & g , float & b , float H);
}


namespace BasicGL
{
    template< class point_t >
    inline void glVertex( point_t const & p )
    {
        glVertex3f( p[0] , p[1] , p[2] );
    }
    template< class point_t >
    inline void glNormal( point_t const & p )
    {
        glNormal3f( p[0] , p[1] , p[2] );
    }
    void drawSphere(float x,float y,float z,float radius,int slices,int stacks);

    template< class point_t >
    void glBox( point_t const & b0 ,
                point_t const & b1 ,
                point_t const & b2 ,
                point_t const & b3 ,
                point_t const & b4 ,
                point_t const & b5 ,
                point_t const & b6 ,
                point_t const & b7 )
    {
        // bottom :
        glVertex( b0 );  glVertex( b1 );  glVertex( b2 );  glVertex( b3 );

        // top:
        glVertex( b4 );  glVertex( b5 );  glVertex( b6 );  glVertex( b7 );

        // front:
        glVertex( b0 );  glVertex( b1 );  glVertex( b5 );  glVertex( b4 );

        // back:
        glVertex( b2 );  glVertex( b3 );  glVertex( b7 );  glVertex( b6 );

        // left:
        glVertex( b0 );  glVertex( b4 );  glVertex( b7 );  glVertex( b3 );

        // right:
        glVertex( b1 );  glVertex( b2 );  glVertex( b6 );  glVertex( b5 );
    }
    template< class point_t >
    void drawCube( point_t const & _Min, point_t const & _Max ){

        point_t points[8] = {
            _Min,
            BasicPoint (_Max[0], _Min[1], _Min[2]),
            BasicPoint (_Max[0], _Max[1], _Min[2]),
            BasicPoint (_Min[0], _Max[1], _Min[2]),

            BasicPoint (_Min[0], _Min[1], _Max[2]),
            BasicPoint (_Max[0], _Min[1], _Max[2]),
            _Max,
            BasicPoint (_Min[0], _Max[1], _Max[2])  };

        //float s = 1.0f;

        glBegin(GL_QUADS);

        glNormal3f(0.f,0.f,-1.f);
        glVertex(points[3]); glVertex(points[2]); glVertex(points[1]); glVertex(points[0]);
        glNormal3f(1.f,0.f,0.f);
        glVertex(points[1]); glVertex(points[2]); glVertex(points[6]); glVertex(points[5]);
        glNormal3f(0.f,1.f,0.f);
        glVertex(points[2]); glVertex(points[3]); glVertex(points[7]); glVertex(points[6]);
        glNormal3f(-1.f,0.f,0.f);
        glVertex(points[3]); glVertex(points[0]); glVertex(points[4]); glVertex(points[7]);
        glNormal3f(0.f,-1.f,0.f);
        glVertex(points[0]); glVertex(points[1]); glVertex(points[5]); glVertex(points[4]);
        glNormal3f(0.f,0.f,1.f);
        glVertex(points[5]); glVertex(points[6]); glVertex(points[7]); glVertex(points[4]);

        glEnd();
    }

}



namespace GLTools{
    void initLights ();
    void setSunsetLight ();
    void setSunriseLight ();
    void setSingleSpotLight ();
    void setMovingSpotLight ();
    void setDefaultMaterial ();
    void setInverseDefaultMaterial ();


    void SetAmbient(float r, float g , float b,float alpha = 1.f);
    void SetDiffuse(float r, float g , float b,float alpha = 1.f);
    void SetSpecular(float r, float g , float b,float alpha = 1.f);
    void SetEmissive(float r, float g , float b,float alpha = 1.f);
    void SetShininess(float sh);

    void MoveSpot(int tetaStep, int phiStep, int tr);


    enum MaterialStandard {
        MY_MATERIAL_01 ,
        TAMY_MATERIAL_01 ,
        TAMY_MATERIAL ,
        EMERALD ,
        JADE ,
        OBSIDIAN ,
        PEARL ,
        RUBY ,
        TURQUOISE ,
        BRASS ,
        BRONZE ,
        CHROME ,
        COPPER ,
        GOLD ,
        POLISHED_GOLD ,
        SILVER ,
        POLISHED_SILVER ,
        BLACK_PLASTIC ,
        CYAN_PLASTIC ,
        GREEN_PLASTIC ,
        RED_PLASTIC ,
        WHITE_PLASTIC ,
        YELLOW_PLASTIC ,
        BLACK_RUBBER ,
        CYAN_RUBBER ,
        GREEN_RUBBER ,
        RED_RUBBER ,
        WHITE_RUBBER ,
        YELLOW_RUBBER ,
        PEWETER
    };

    void MLoadStandard( MaterialStandard typ );
}


namespace GLCheck{
    int checkErrors( std::string const & szFile , int iLine );
}

namespace FileIO{
    template <typename Point, typename Face>
            bool read(std::istream& _in, std::vector<Point> & vertices, std::vector<Face> & triangles )
    {
        std::cout << "[OBJReader] : read file\n";


        std::string line;
        std::string keyWrd;

        float                  x, y, z;

        std::vector<int> vhandles;

        while( _in && !_in.eof() )
        {
            std::getline(_in,line);
            if ( _in.bad() ){
                std::cout << "  Warning! Could not read file properly!\n";
                return false;
            }

            size_t start = line.find_first_not_of(" \t\r\n");
            size_t end   = line.find_last_not_of(" \t\r\n");

            if(( std::string::npos == start ) || ( std::string::npos == end))
                line = "";
            else
                line = line.substr( start, end-start+1 );

            // comment
            if ( line.size() == 0 || line[0] == '#' || isspace(line[0]) ) {
                continue;
            }

            std::stringstream stream(line);

            stream >> keyWrd;

            // material file
            if (keyWrd == "mtllib" || keyWrd == "usemtl")
            {

            }
            // vertex
            else if (keyWrd == "v")
            {
                stream >> x; stream >> y; stream >> z;

                if ( !stream.fail() )
                    vertices.push_back(Point(x,y,z));
            }

            // texture coord
            else if (keyWrd == "vt" || keyWrd == "vn")
            {

            }
            // face
            else if (keyWrd == "f")
            {
                int component(0), nV(0);
                int value;

                vhandles.clear();

                // read full line after detecting a face
                std::string faceLine;
                std::getline(stream,faceLine);
                std::stringstream lineData( faceLine );

                // work on the line until nothing left to read
                while ( !lineData.eof() )
                {
                    // read one block from the line ( vertex/texCoord/normal )
                    std::string vertex;
                    lineData >> vertex;

                    do{

                        //get the component (vertex/texCoord/normal)
                        size_t found=vertex.find("/");

                        // parts are seperated by '/' So if no '/' found its the last component
                        if( found != std::string::npos ){

                            // read the index value
                            std::stringstream tmp( vertex.substr(0,found) );

                            // If we get an empty string this property is undefined in the file
                            if ( vertex.substr(0,found).empty() ) {
                                // Switch to next field
                                vertex = vertex.substr(found+1);

                                // Now we are at the next component
                                ++component;

                                // Skip further processing of this component
                                continue;
                            }

                            // Read current value
                            tmp >> value;

                            // remove the read part from the string
                            vertex = vertex.substr(found+1);

                        } else {

                            // last component of the vertex, read it.
                            std::stringstream tmp( vertex );
                            tmp >> value;

                            // Clear vertex after finished reading the line
                            vertex="";

                            // Nothing to read here ( garbage at end of line )
                            if ( tmp.fail() ) {
                                continue;
                            }
                        }

                        // store the component ( each component is referenced by the index here! )
                        switch (component)
                        {
                        case 0: // vertex
                            if ( value < 0 ) {
                                // Calculation of index :
                                // -1 is the last vertex in the list
                                // As obj counts from 1 and not zero add +1
                                value = vertices.size() + value + 1;
                            }
                            // Obj counts from 1 and not zero .. array counts from zero therefore -1
                            vhandles.push_back(value-1);
                            break;

                case 1: // texture coord
                    break;

                case 2: // normal
                    break;
                }

                        // Prepare for reading next component
                        ++component;

                        // Read until line does not contain any other info
                    } while ( !vertex.empty() );

                    component = 0;
                    nV++;
                }

                if (vhandles.size()>3)
                {
                    //model is not triangulated, so let us do this on the fly...
                    //to have a more uniform mesh, we add randomization
                    unsigned int k=(false)?(rand()%vhandles.size()):0;
                    for (unsigned int i=0;i<vhandles.size()-2;++i)
                    {
                        triangles.push_back(Face(vhandles[(k+0)%vhandles.size()],vhandles[(k+i+1)%vhandles.size()],vhandles[(k+i+2)%vhandles.size()]));
                    }
                }
                else if (vhandles.size()==3)
                {
                    triangles.push_back(Face(vhandles[0],vhandles[1],vhandles[2]));
                }
                else
                {
                    printf("TriMesh::LOAD: Unexpected number of face vertices (<3). Ignoring face");
                }
            }

        }

        return true;
    }

    template <typename Point, typename Face>
            bool objLoader(const std::string& _filename, std::vector<Point> & vertices, std::vector<Face> & triangles)
    {

        std::fstream in( _filename.c_str(), std::ios_base::in );

        if (!in.is_open() || !in.good())
        {
            std::cout << "[OBJReader] : cannot not open file "
                    << _filename
                    << std::endl;
            return false;
        }

        {
#if defined(WIN32)
            std::string::size_type dot = _filename.find_last_of("\\/");
#else
            std::string::size_type dot = _filename.rfind("/");
#endif
            std::string path_ = (dot == std::string::npos)
                                ? "./"
                                    : std::string(_filename.substr(0,dot+1));
        }

        bool result = FileIO::read(in, vertices, triangles);

        in.close();
        return result;
    }

    template <typename Point, typename Face>
   bool saveOFF(const std::string& filename, std::vector<Point> & vertices, std::vector<Face> & triangles)
    {

            std::ofstream myfile;
            myfile.open(filename.c_str());
            if (!myfile.is_open())
            {
                std::cout << filename << " cannot be opened" << std::endl;
                return false;
            }

            myfile << "OFF" << std::endl;
            myfile << (vertices.size()) << " " << triangles.size() << " 0" << std::endl;

            for( unsigned int v = 0 ; v < vertices.size() ; ++v )
            {
                myfile << (vertices[v]) << std::endl;
            }

            for( unsigned int t = 0 ; t < triangles.size() ; ++t )
            {
                myfile << "3 " << (triangles[t][0]) << " " << (triangles[t][1]) << " " << (triangles[t][2]) << std::endl;
            }


            myfile.close();
        return true;
    }

}

namespace MeshTools{
    template <typename Point>
            void computeAveragePosAndRadius ( const std::vector<Point> & points, Point & center, double & radius){
        center = Point (0.0, 0.0, 0.0);
        for (unsigned int i = 0; i < points.size (); i++)
            center += points[i];
        center /= float (points.size ());
        radius = 0.0;
        for (unsigned int i = 0; i < points.size (); i++) {
            float vDistance = (points[i] - center).norm();
            if (vDistance > radius)
                radius = vDistance;
        }
    }

    template <typename Point>
            void scaleAndCenterToUnitBox ( std::vector<Point> & points, Point & center, double & scale ){
        computeAveragePosAndRadius (points, center, scale);
        for (unsigned int i = 0; i < points.size (); i++){
            points[i] = (points[i] - center)/ scale;
        }

    }

    template <typename Point>
            bool isVisiblePoint( const Point & p, const Point & clippingNormal, const Point & pointOnClipping ){

        Point pos = p - pointOnClipping;
        float dotProduct = 0.;
        for(int v = 0 ; v < 3; v ++)
            dotProduct += clippingNormal[v]*pos[v];

        if( dotProduct < 0.) return false;

        return true;
    }

}

namespace Math{
    // ---------------------------------------------------------------------------------
    // get a random number (int)
    // ---------------------------------------------------------------------------------
    int	getRandomIntBetween(int iMin, int iMax);

    // ---------------------------------------------------------------------------------
    // get a random number (float)
    // ---------------------------------------------------------------------------------
    float getRandomFloatBetween(float fMin, float fMax);
}
#endif // GLUTILITYMETHODS_H
