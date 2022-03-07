#include "GLUtilityMethods.h"

#if 1
#define GetOpenGLError() __GetOpenGLError( ( char* )__FILE__, ( int )__LINE__ )
#else
#define GetOpenGLError()
#endif

#include <cassert>

namespace GLCheck{
    int checkErrors( std::string const & szFile , int iLine )
    {
        int iRetCode;
        GLenum glErr = glGetError();
        while ( glErr != GL_NO_ERROR ) {
       //     std::cout << "GLError in file << " << szFile << " @ line " << iLine << " : " << gluErrorString( glErr ) << std::endl;
            iRetCode = 1;
            glErr = glGetError();
        }
        return iRetCode;
    }
}



namespace RGB
{
    int nColor = 30;

    float color[ 30 ][3] = {
        { 0.52459 , 0.370169 , 0.818006 } ,
        { 0.22797 , 0.736751 , 0.578866 } ,
        { 0.615946 , 0.840482 , 0.941527 } ,
        { 0.820386 , 0.886748 , 0.611765 } ,
        { 0.523095 , 0.277974 , 0.277974 } ,
        { 0.160586 , 0.399985 , 0.656397 } ,
        { 0.563622 , 0.205234 , 0.501854 } ,
        { 0.948363 , 0.724559 , 0.358358 } ,
        { 0.377066 , 0.299641 , 0.860731 } ,
        { 0.29955 , 0.770504 , 0.824872 } ,
        { 0.659052 , 0.409354 , 0.861982 } ,
        { 0.559197 , 0.574395 , 0.133989 } ,
        { 0.513832 , 0.308644 , 0.163775 } ,
        { 0.825589 , 0.496513 , 0.564584 } ,
        { 0.800473 , 0.423407 , 0.657435 } ,
        { 0.347524 , 0.813748 , 0.765499 } ,
        { 0.501244 , 0.87425 , 0.44152 } ,
        { 0.493843 , 0.905364 , 0.692546 } ,
        { 0.304311 , 0.506661 , 0.197818 } ,
        { 0.520516 , 0.124697 , 0.534661 } ,
        { 0.551827 , 0.655329 , 0.927062 } ,
        { 0.64477 , 0.907103 , 0.321981 } ,
        { 0.924224 , 0.835676 , 0.410391 } ,
        { 0.271122 , 0.30605 , 0.777478 } ,
        { 0.720943 , 0.366674 , 0.274281 } ,
        { 0.596414 , 0.334386 , 0.334386 } ,
        { 0.293492 , 0.801221 , 0.328527 } ,
        { 0.409415 , 0.607416 , 0.464027 } ,
        { 0.861845 , 0.425895 , 0.606302 } ,
        { 0.49691 , 0.196338 , 0.592523 }
    };

    float color4[ 30 ][4] = {
        { 0.52459 , 0.370169 , 0.818006 , 1.f } ,
        { 0.22797 , 0.736751 , 0.578866 , 1.f } ,
        { 0.615946 , 0.840482 , 0.941527 , 1.f } ,
        { 0.820386 , 0.886748 , 0.611765 , 1.f } ,
        { 0.523095 , 0.277974 , 0.277974 , 1.f } ,
        { 0.160586 , 0.399985 , 0.656397 , 1.f } ,
        { 0.563622 , 0.205234 , 0.501854 , 1.f } ,
        { 0.948363 , 0.724559 , 0.358358 , 1.f } ,
        { 0.377066 , 0.299641 , 0.860731 , 1.f } ,
        { 0.29955 , 0.770504 , 0.824872 , 1.f } ,
        { 0.659052 , 0.409354 , 0.861982 , 1.f } ,
        { 0.559197 , 0.574395 , 0.133989 , 1.f } ,
        { 0.513832 , 0.308644 , 0.163775 , 1.f } ,
        { 0.825589 , 0.496513 , 0.564584 , 1.f } ,
        { 0.800473 , 0.423407 , 0.657435 , 1.f } ,
        { 0.347524 , 0.813748 , 0.765499 , 1.f } ,
        { 0.501244 , 0.87425 , 0.44152 , 1.f } ,
        { 0.493843 , 0.905364 , 0.692546 , 1.f } ,
        { 0.304311 , 0.506661 , 0.197818 , 1.f } ,
        { 0.520516 , 0.124697 , 0.534661 , 1.f } ,
        { 0.551827 , 0.655329 , 0.927062 , 1.f } ,
        { 0.64477 , 0.907103 , 0.321981 , 1.f } ,
        { 0.924224 , 0.835676 , 0.410391 , 1.f } ,
        { 0.271122 , 0.30605 , 0.777478 , 1.f } ,
        { 0.720943 , 0.366674 , 0.274281 , 1.f } ,
        { 0.596414 , 0.334386 , 0.334386 , 1.f } ,
        { 0.293492 , 0.801221 , 0.328527 , 1.f } ,
        { 0.409415 , 0.607416 , 0.464027 , 1.f } ,
        { 0.861845 , 0.425895 , 0.606302 , 1.f } ,
        { 0.49691 , 0.196338 , 0.592523 , 1.f }
    };


    void get_random_RGB( float & r , float & g , float & b )
    {
        r = (float)(rand()) / (float)(RAND_MAX);
        g = (float)(rand()) / (float)(RAND_MAX);
        b = (float)(rand()) / (float)(RAND_MAX);
    }

    void get_random_RGB_from_HSV( float & r , float & g , float & b )
    {
        QColor c = QColor::fromHsvF( (float)(rand()) / (float)(RAND_MAX) , 0.3f + 0.5f * (float)(rand()) / (float)(RAND_MAX) , 0.5f + 0.5f * (float)(rand()) / (float)(RAND_MAX) );

        r = c.redF();
        g = c.greenF();
        b = c.blueF();
    }

    void get_random_RGB_from_HSV( float & r , float & g , float & b , float H )
    {
        QColor c = QColor::fromHsvF( H , 0.3f + 0.5f * (float)(rand()) / (float)(RAND_MAX) , 0.5f + 0.5f * (float)(rand()) / (float)(RAND_MAX) );

        r = c.redF();
        g = c.greenF();
        b = c.blueF();
    }

    void calc_RGB( float val , float val_min , float val_max , float & r , float & g , float & b )
    {
        // define uniform color intervalls [v0,v1,v2,v3,v4]
        float v0, v1, v2, v3, v4 ;

        v0 = val_min ;
        v1 = val_min + 1.0/4.0 * (val_max - val_min);
        v2 = val_min + 2.0/4.0 * (val_max - val_min);
        v3 = val_min + 3.0/4.0 * (val_max - val_min);
        v4 = val_max ;



        if (val < v0)
        {
            r = 0.f;
            g = 0.f;
            b = 1.f;
            return;
        }
        else if (val > v4)
        {
            r = 1.f;
            g = 0.f;
            b = 0.f;
            return;
        }
        else if (val <= v2)
        {
            if (val <= v1) // [v0, v1]
            {
                r = 0.f;
                g = (val - v0) / (v1 - v0);
                b = 1.f;
                return;
            }
            else // ]v1, v2]
            {
                r = 0.f;
                g = 1.f;
                b = 1.f - (val - v1) / (v2 - v1);
                return;
            }
        }
        else
        {
            if (val <= v3) // ]v2, v3]
            {
                r = (val - v2) / (v3 - v2);
                g = 1.f;
                b = 0.f;
                return;
            }
            else // ]v3, v4]
            {
                r = 1.f;
                g = 1.f - (val - v3) / (v4 - v3);
                b = 0.f;
                return;
            }
        }
    }
}



namespace BasicGL
{
    void drawSphere(float x,float y,float z,float radius,int slices,int stacks)
    {
        if(stacks < 2){stacks = 2;}
        if(stacks > 20){stacks = 20;}
        if(slices < 3){slices = 3;}
        if(slices > 30){slices = 30;}
        //Pas essentiel ...

        int Nb = slices*stacks +2;
        std::vector< qglviewer::Vec > points(Nb);

        qglviewer::Vec centre(x,y,z);

        float sinP , cosP , sinT , cosT , Phi , Theta;
        points[0] = qglviewer::Vec(0,0,1);
        points[Nb-1] = qglviewer::Vec(0,0,-1);

        for(int i=1; i<=stacks; i++)
        {
            Phi = 90 - (float)(i*180)/(float)(stacks+1);
            sinP = sinf(Phi*3.14159265/180);
            cosP = cosf(Phi*3.14159265/180);

            for(int j=1; j<=slices; j++)
            {
                Theta = (float)(j*360)/(float)(slices);
                sinT = sinf(Theta*3.14159265/180);
                cosT = cosf(Theta*3.14159265/180);

                points[ j + (i-1)*slices ] = qglviewer::Vec(cosT*cosP,sinT*cosP,sinP);
            }
        }

        int k1,k2;
        glBegin(GL_TRIANGLES);
        for(int i=1; i<=slices; i++)
        {
            k1 = i;
            k2 = (i%slices+1);
            glNormal3fv(points[0]);
            glVertex3fv((centre + radius*points[0]));
            glNormal3fv(points[k1]);
            glVertex3fv((centre + radius*points[k1]));
            glNormal3fv(points[k2]);
            glVertex3fv((centre + radius*points[k2]));

            k1 = (stacks-1)*slices+i;
            k2 = (stacks-1)*slices+(i%slices+1);
            glNormal3fv(points[k1]);
            glVertex3fv((centre + radius*points[k1]));
            glNormal3fv(points[Nb-1]);
            glVertex3fv((centre + radius*points[Nb-1]));
            glNormal3fv(points[k2]);
            glVertex3fv((centre + radius*points[k2]));
        }
        glEnd();

        glBegin(GL_QUADS);
        for(int j=1; j<stacks; j++)
        {
            for(int i=1; i<=slices; i++)
            {
                k1 = i + (j-1)*slices;
                k2 = (i%slices+1) + (j-1)*slices;
                glNormal3fv(points[k2]);
                glVertex3fv((centre + radius*points[k2]));
                glNormal3fv(points[k1]);
                glVertex3fv((centre + radius*points[k1]));

                k1 = i + (j)*slices;
                k2 = (i%slices+1) + (j)*slices;
                glNormal3fv(points[k1]);
                glVertex3fv((centre + radius*points[k1]));
                glNormal3fv(points[k2]);
                glVertex3fv((centre + radius*points[k2]));
            }
        }
        glEnd();
    }
}









namespace GLTools{
    void initLights () {
        GLfloat light_position0[4] = {0, 0, 10, 0};
        GLfloat light_position1[4] = {52, 16, 50, 0};
        GLfloat light_position2[4] = {26, 48, 50, 0};
        GLfloat light_position3[4] = {-16, 52, 50, 0};
        GLfloat light_position4[4] = {42, 374, 161, 0};
        GLfloat light_position5[4] = {473, -351, -259, 0};
        GLfloat light_position6[4] = {-438, 167, -48, 0};

        GLfloat direction1[3] = {-52,-16,-50};
        GLfloat direction2[3] = {-26,-48,-50};
        GLfloat direction3[3] = {16,-52,-50};
        GLfloat direction4[3] = {-42, -374, -161};
        GLfloat direction5[3] = {-473, 351, 259};
        GLfloat direction6[3] = {438, -167, 48};


        GLfloat color1[4] = {1,0, 0, 1};
        GLfloat color2[4] = {0, 1, 0, 1};
        GLfloat color3[4] = {0, 0, 1, 1};
        GLfloat color4[4] = {1, 1, 1, 1};
        GLfloat color5[4] = {0.28, 0.39, 1.0, 1};
        GLfloat color6[4] = {1.0, 0.69, 0.23, 1};

        GLfloat specularColor4[4] = {0.8, 0.8, 0.8, 1};
        GLfloat specularColor5[4] = {0.8, 0.8, 0.8, 1};
        GLfloat specularColor6[4] = {0.8, 0.8, 0.8, 1};

        GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

        glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

        glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
        glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
        glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
        glLightfv (GL_LIGHT1, GL_SPECULAR, color1);

        glLightfv (GL_LIGHT2, GL_POSITION, light_position2);
        glLightfv (GL_LIGHT2, GL_SPOT_DIRECTION, direction2);
        glLightfv (GL_LIGHT2, GL_DIFFUSE, color2);
        glLightfv (GL_LIGHT2, GL_SPECULAR, color2);

        glLightfv (GL_LIGHT3, GL_POSITION, light_position3);
        glLightfv (GL_LIGHT3, GL_SPOT_DIRECTION, direction3);
        glLightfv (GL_LIGHT3, GL_DIFFUSE, color3);
        glLightfv (GL_LIGHT3, GL_SPECULAR, color3);

        glLightfv (GL_LIGHT4, GL_POSITION, light_position4);
        glLightfv (GL_LIGHT4, GL_SPOT_DIRECTION, direction4);
        glLightfv (GL_LIGHT4, GL_DIFFUSE, color4);
        glLightfv (GL_LIGHT4, GL_SPECULAR, specularColor4);

        glLightfv (GL_LIGHT5, GL_POSITION, light_position5);
        glLightfv (GL_LIGHT5, GL_SPOT_DIRECTION, direction5);
        glLightfv (GL_LIGHT5, GL_DIFFUSE, color5);
        glLightfv (GL_LIGHT5, GL_SPECULAR, specularColor5);

        glLightfv (GL_LIGHT6, GL_POSITION, light_position6);
        glLightfv (GL_LIGHT6, GL_SPOT_DIRECTION, direction6);
        glLightfv (GL_LIGHT6, GL_DIFFUSE, color6);
        glLightfv (GL_LIGHT6, GL_SPECULAR, specularColor6);

        glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);

        glEnable (GL_LIGHTING);
    }

    void setSunsetLight () {
        glDisable (GL_LIGHT0);
        glEnable (GL_LIGHT1);
        glEnable (GL_LIGHT2);
        glEnable (GL_LIGHT3);
        glDisable (GL_LIGHT4);
        glDisable (GL_LIGHT5);
        glDisable (GL_LIGHT6);
    }

    void setSunriseLight () {
        glDisable (GL_LIGHT0);
        glDisable (GL_LIGHT1);
        glDisable (GL_LIGHT2);
        glDisable (GL_LIGHT3);
        glEnable (GL_LIGHT4);
        glEnable (GL_LIGHT5);
        glEnable (GL_LIGHT6);
    }

    void setSingleSpotLight () {
        glEnable (GL_LIGHT0);
        glDisable (GL_LIGHT1);
        glDisable (GL_LIGHT2);
        glDisable (GL_LIGHT3);
        glDisable (GL_LIGHT4);
        glDisable (GL_LIGHT5);
        glDisable (GL_LIGHT6);
    }

    void setDefaultMaterial () {
        GLfloat material_color[4] = {1,1,1,1.0f};
        GLfloat material_specular[4] = {0.00,0.00,0.00,.00};
        GLfloat material_ambient[4] = {0.05,0.05,0.05,.05};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);
    }

    void setInverseDefaultMaterial () {
        GLfloat material_color[4] = {0,0,0,1.0f};
        GLfloat material_specular[4] = {0.5,0.5,0.5,1.0};
        GLfloat material_ambient[4] = {0.5,0.5,0.5,0.0};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);
    }

    void setMovingSpotLight() {
        glDisable (GL_LIGHT0);
        glDisable (GL_LIGHT1);
        glDisable (GL_LIGHT2);
        glDisable (GL_LIGHT3);
        glEnable (GL_LIGHT4);
        glDisable (GL_LIGHT5);
        glDisable (GL_LIGHT6);
    }


    void MoveSpot(int tetaStep, int phiStep, int trStep){
        int r = std::max(409+trStep, 1);

        float teta = (tetaStep % 360) * 2*M_PI/360.;
        float phi = (phiStep % 360)* 2*M_PI/360.;

        GLfloat light_position4[4] = {r*sin(teta)*cos(phi), r*cos(teta)*cos(phi), r*sin(phi), 1};

        glLightfv (GL_LIGHT4, GL_POSITION, light_position4);
    }

    // FROM MATTHIAS :

    void SetAmbient(float r, float g , float b,float alpha)
    {
        float mat[4] = {r,g,b,alpha};
        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, mat );
    }
    void SetDiffuse(float r, float g , float b,float alpha)
    {
        float mat[4] = {r,g,b,alpha};
        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, mat );
    }
    void SetSpecular(float r, float g , float b,float alpha)
    {
        float mat[4] = {r,g,b,alpha};
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat );
    }
    void SetShininess(float sh)
    {
        glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 128.f * sh );
    }
    void SetEmissive(float r, float g , float b,float alpha)
    {
        float mat[4] = {r,g,b,alpha};
        glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat );
    }



    void MLoadStandard( GLTools::MaterialStandard typ )
    {
        switch( typ ) {
            // My special materials
        case MY_MATERIAL_01:
            SetAmbient  ( 0.0f,			0.058823530f,	0.32549021f );
            SetDiffuse  ( 0.54509807f,	0.76078433f,	0.81960785f );
            SetSpecular ( 1.f,			1.f,			1.f );
            SetShininess( 1.f );
            break;

            // Tamy material
        case TAMY_MATERIAL_01:
            SetAmbient  ( 0.f,			0.f,			0.f );
            SetDiffuse  ( 1.f,			1.f,			1.f );
            SetSpecular ( 0.5f,			0.5f,			0.5f );
            SetShininess( 1.f );
            break;


            // Tamy material
        case TAMY_MATERIAL:
            SetAmbient  ( 0.f,			0.f,			0.f );
            SetDiffuse  ( 1.f,			1.f,			1.f );
            SetSpecular ( 0.01f,		.01f,			0.01f );
            SetShininess( 0.01f );
            break;

        case EMERALD:
            SetAmbient  ( 0.0215f,		0.1745f,		0.0215f );
            SetDiffuse  ( 0.07568f,		0.61424f,		0.07568f );
            SetSpecular ( 0.633f,		0.727811f,		0.0633f );
            SetShininess( 0.6f );
            break;
        case JADE:
            SetAmbient  ( 0.135f,       0.2225f,		0.1575f );
            SetDiffuse  ( 0.54f,		0.89f,			0.063f );
            SetSpecular ( 0.316228f,	0.0316228f,		0.316228f );
            SetShininess( 0.1f );
            break;
        case OBSIDIAN:
            SetAmbient  ( 0.05375f,     0.05f,			0.0625f );
            SetDiffuse  ( 0.18275f,     0.17f,			0.22525f );
            SetSpecular ( 0.332741f,	0.328634f,		0.346435f );
            SetShininess( 0.3f );
            break;
        case PEARL:
            SetAmbient  ( 0.25f,		0.20725f,		0.20725f );
            SetDiffuse  ( 1.0f,         0.829f,			0.829f );
            SetSpecular ( 0.296648f,	0.296648f,		0.296648f );
            SetShininess( 0.088f );
            break;
        case RUBY:
            SetAmbient  ( 0.1745f,      0.01175f,		0.01175f );
            SetDiffuse  ( 0.61424f,     0.04136f,		0.04136f );
            SetSpecular ( 0.727811f,	0.626959f,		0.626959f );
            SetShininess( 0.6f );
            break;
        case TURQUOISE:
            SetAmbient  ( 0.1f,         0.18725f,		0.1745f );
            SetDiffuse  ( 0.396f,       0.74151f,		0.69102f );
            SetSpecular ( 0.297254f,	0.30829f,		0.306678f );
            SetShininess( 0.1f );
            break;
        case BRASS:
            SetAmbient  ( 0.329412f,	0.223529f,		0.027451f );
            SetDiffuse  ( 0.780392f,	0.568627f,		0.113725f );
            SetSpecular ( 0.992157f,	0.941176f,		0.807843f );
            SetShininess( 0.21794872f );
            break;
        case BRONZE:
            SetAmbient  ( 0.2125f,      0.1275f,		0.054f );
            SetDiffuse  ( 0.714f,       0.4284f,		0.18144f );
            SetSpecular ( 0.393548f,	0.271906f,		0.166721f );
            SetShininess( 0.2f );
            break;
        case CHROME:
            SetAmbient  ( 0.25f,		0.25f,			0.25f );
            SetDiffuse  ( 0.4f,         0.4f,			0.4f );
            SetSpecular ( 0.774597f,	0.774597f,		0.774597f );
            SetShininess( 0.6f );
            break;
        case COPPER:
            SetAmbient  ( 0.19125f,     0.0735f,		0.0225f );
            SetDiffuse  ( 0.7038f,  	0.27048f,		0.0828f );
            SetSpecular ( 0.256777f,	0.137622f,		0.086014f );
            SetShininess( 0.1f );
            break;
        case GOLD:
            SetAmbient  ( 0.24725f,     0.1995f,		0.0745f );
            SetDiffuse  ( 0.75164f,     0.60648f,		0.22648f );
            SetSpecular ( 0.628281f,	0.555802f,		0.366065f );
            SetShininess( 0.4f );
            break;
        case POLISHED_GOLD:
            SetAmbient  ( 0.24725f,     0.2245f,		0.0645f );
            SetDiffuse  ( 0.34615f,     0.3143f,		0.0903f );
            SetSpecular ( 0.797357f,	0.723991f,		0.208006f );
            SetShininess( 83.2f / 128.f );
            break;
        case SILVER:
            SetAmbient  ( 0.19225f,     0.19225f,		0.19225f );
            SetDiffuse  ( 0.50754f,     0.50754f,		0.50754f );
            SetSpecular ( 0.508273f,	0.508273f,		0.508273f );
            SetShininess( 0.4f );
            break;
        case POLISHED_SILVER:
            SetAmbient  ( 0.23125f,     0.23125f,		0.23125f );
            SetDiffuse  ( 0.2775f,     0.2775f,		0.2775f );
            SetSpecular ( 0.773911f,	0.773911f,		0.773911f );
            SetShininess( 89.6f / 128.f );
            break;
        case BLACK_PLASTIC:
            SetAmbient  ( 0.0f,			0.0f,			0.0f );
            SetDiffuse  ( 0.01f,		0.01f,			0.01f );
            SetSpecular ( 0.50f,		0.50f,			0.50f );
            SetShininess( 0.25f );
            break;
        case CYAN_PLASTIC:
            SetAmbient  ( 0.0f,			0.1f,			0.06f );
            SetDiffuse  ( 0.0f,			0.50980392f,	0.50980392f );
            SetSpecular ( 0.50196078f,	0.50196078f,	0.50196078f );
            SetShininess( 0.25f );
            break;
        case GREEN_PLASTIC:
            SetAmbient  ( 0.0f,			0.0f,			0.0f );
            SetDiffuse  ( 0.1f,			0.35f,			0.1f );
            SetSpecular ( 0.45f,		0.55f,			0.45f );
            SetShininess( 0.25f );
            break;
        case RED_PLASTIC:
            SetAmbient  ( 0.0f,			0.0f,			0.0f );
            SetDiffuse  ( 0.5f,			0.0f,			0.0f );
            SetSpecular ( 0.7f,			0.6f,			0.6f );
            SetShininess( 0.25f );
            break;
        case WHITE_PLASTIC:
            SetAmbient  ( 0.0f,			0.0f,			0.0f );
            SetDiffuse  ( 0.55f,		0.55f,			0.55f );
            SetSpecular ( 0.70f,		0.70f,			0.70f );
            SetShininess( 0.25f );
            break;
        case YELLOW_PLASTIC:
            SetAmbient  ( 0.0f,			0.0f,			0.0f );
            SetDiffuse  ( 0.5f,			0.5f,			0.0f );
            SetSpecular ( 0.6f,			0.6f,			0.5f );
            SetShininess( 0.25f );
            break;
        case BLACK_RUBBER:
            SetAmbient  ( 0.02f,		0.02f,			0.02f );
            SetDiffuse  ( 0.01f,		0.01f,			0.01f );
            SetSpecular ( 0.4f,			0.4f,			0.4f );
            SetShininess( 0.078125f );
            break;
        case CYAN_RUBBER:
            SetAmbient  ( 0.0f,			0.05f,			0.05f );
            SetDiffuse  ( 0.4f,			0.5f,			0.5f );
            SetSpecular ( 0.04f,		0.7f,			0.7f );
            SetShininess( 0.078125f );
            break;
        case GREEN_RUBBER:
            SetAmbient  ( 0.0f,			0.05f,			0.0f );
            SetDiffuse  ( 0.4f,			0.5f,			0.4f );
            SetSpecular ( 0.04f,		0.7f,			0.04f );
            SetShininess( 0.078125f );
            break;
        case RED_RUBBER:
            SetAmbient  ( 0.05f,        0.0f,			0.0f );
            SetDiffuse  ( 0.5f,			0.4f,			0.4f );
            SetSpecular ( 0.7f,			0.04f,			0.04f );
            SetShininess( 0.078125f );
            break;
        case WHITE_RUBBER:
            SetAmbient  ( 0.05f,        0.05f,			0.05f );
            SetDiffuse  ( 0.5f,			0.5f,			0.5f );
            SetSpecular ( 0.7f,			0.7f,			0.7f );
            SetShininess( 0.078125f );
            break;
        case YELLOW_RUBBER:
            SetAmbient  ( 0.05f,        0.05f,			0.0f );
            SetDiffuse  ( 0.5f,			0.5f,			0.4f );
            SetSpecular ( 0.7f,			0.7f,			0.04f );
            SetShininess( 0.078125f );
            break;
        case PEWETER:
            SetAmbient  ( 0.10588f, 0.05824f, 0.113725f );
            SetDiffuse  ( 0.427451f, 0.470588f, 0.541176f );
            SetSpecular ( 0.3333f, 0.3333f,	0.521569f );
            SetShininess( 51.2f / 128.f );
            break;
        default:
            assert( !"Invalid Standard-Material Index!" );
            break;
        }
        SetEmissive( 0.f, 0.f, 0.f );
    }
}

namespace Math{
    // ---------------------------------------------------------------------------------
    // get a random number (int)
    // ---------------------------------------------------------------------------------
    int	getRandomIntBetween(int iMin, int iMax)
    {
            const float fDiv = 1.0f / (RAND_MAX + 1.0f);
            int iRand = rand();
            int iRange = iMax - iMin + 1;
            return iMin + (int) ( iRange * iRand * fDiv );
    }

    // ---------------------------------------------------------------------------------
    // get a random number (float)
    // ---------------------------------------------------------------------------------
    float getRandomFloatBetween(float fMin, float fMax)
    {
            const float fDiv = 1.0f / (RAND_MAX + 1.0f);
            int iRand = rand();
            float fRange = fMax - fMin + 1.0f;
            return fMin + ( fRange * iRand * fDiv );
    }
}
