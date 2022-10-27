#ifndef BASICPOINT_H
#define BASICPOINT_H

#include <cmath>
#include <iostream>
#include <cassert>
#include <map>
#include <float.h>
#include <stdlib.h>

using namespace std;

class BasicPoint
{
    float x , y , z;

public:
    BasicPoint() : x(0) , y(0) , z(0) {}
    BasicPoint( float x_ , float y_ , float z_ ) : x(x_) , y(y_) , z(z_) {}

    float operator * ( BasicPoint const & other ) const
    {
        return x * other.x + y * other.y + z * other.z;
    }
    float operator [] (unsigned int c) const
    {
        if( c == 0 )
            return x;
        if( c == 1 )
            return y;
        if( c == 2 )
            return z;

        assert( 0  &&  "Give a index between 0 and 2 as a coordinate for BasicPoint" );
        return 0.;
    }

    float & operator [] (unsigned int c)
    {
        if( c == 0 )
            return x;
        if( c == 1 )
            return y;
        if( c == 2 )
            return z;

        assert( 0  &&  "Give a index between 0 and 2 as a coordinate for BasicPoint" );

    }

    BasicPoint operator % ( BasicPoint const & other ) const
    {
        return BasicPoint( y * other.z - z * other.y , z * other.x - x * other.z , x * other.y - y * other.x );
    }

    BasicPoint operator + ( BasicPoint const & other ) const
    {
        return BasicPoint( x + other.x , y + other.y , z + other.z );
    }
    BasicPoint operator - ( BasicPoint const & other ) const
    {
        return BasicPoint( x - other.x , y - other.y , z - other.z );
    }
    void operator += ( BasicPoint const & other )
                     {
        x += other.x;
        y += other.y;
        z += other.z;
    }
    void operator -= ( BasicPoint const & other )
                     {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }
    void operator /= (int m)
                     {
        x /= m;
        y /= m;
        z /= m;
    }
    void operator /= (unsigned int m)
                     {
        x /= m;
        y /= m;
        z /= m;
    }
    void operator /= (float m)
                     {
        x /= m;
        y /= m;
        z /= m;
    }
    void operator /= (double m)
                     {
        x /= m;
        y /= m;
        z /= m;
    }

    float sqrnorm() const
    {
        return x * x + y * y + z * z;
    }

    float norm() const
    {
        return sqrt( sqrnorm() );
    }


    bool isNull(){
        return ( sqrnorm() < 0.00001 );
    }

    void normalize()
    {
        float n = norm();

//        if( fabs( n ) < FLT_MIN*10 )
//            std::cout << "may be the normalize() is not safe ?" << std::endl;

        x /= n;
        y /= n;
        z /= n;
    }
    
    double normDoublePrecision()
    {
        return sqrt( (double)(x) * (double)(x) + (double)(x) * (double)(y) + (double)(z) * (double)(z) );
    }
    void normalizeDoublePrecision()
    {
        double n = normDoublePrecision();

        x = ( (double)(x) / n );
        y = ( (double)(y) / n );
        z = ( (double)(z) / n );
    }


    void operator << ( BasicPoint const & p )
    {
        x = std::min( x , p[0] );
        y = std::min( y , p[1] );
        z = std::min( z , p[2] );
    }
    void operator >> ( BasicPoint const & p )
    {
        x = std::max( x , p[0] );
        y = std::max( y , p[1] );
        z = std::max( z , p[2] );
    }
};

inline BasicPoint operator * (float m , BasicPoint const & p)
{
    return BasicPoint( m * p[0] , m * p[1] , m * p[2] );
}
inline BasicPoint operator * (int m , BasicPoint const & p)
{
    return BasicPoint( m * p[0] , m * p[1] , m * p[2] );
}
inline BasicPoint operator * (double m , BasicPoint const & p)
{
    return BasicPoint( m * p[0] , m * p[1] , m * p[2] );
}
inline BasicPoint operator * (BasicPoint const & p , float m)
{
    return BasicPoint( m * p[0] , m * p[1] , m * p[2] );
}
inline BasicPoint operator * (BasicPoint const & p , int m)
{
    return BasicPoint( m * p[0] , m * p[1] , m * p[2] );
}
inline BasicPoint operator * (BasicPoint const & p , double m)
{
    return BasicPoint( m * p[0] , m * p[1] , m * p[2] );
}

inline std::ostream & operator << (std::ostream & s , BasicPoint const & p)
{
    s << p[0] << " " << p[1] << " " << p[2];
    return s;
}

inline bool operator< (BasicPoint const & a, BasicPoint const & b) {
    return (a[0] < b[0] && a[1] < b[1] && a[2] < b[2]);
}

inline bool operator== (BasicPoint const & p1, BasicPoint const & p2) {
    return (p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2]);
}

inline bool operator>= (BasicPoint const & a, BasicPoint const & b) {
    return (a[0] >= b[0] || a[1] >= b[1] || a[2] >= b[2]);
}

inline bool operator!= (BasicPoint const & p1, BasicPoint const & p2) {
    return (p1[0] != p2[0] || p1[1] != p2[1] || p1[2] != p2[2]);
}

inline BasicPoint operator / (BasicPoint const & p , float m)
{
    return BasicPoint( p[0] / m , p[1] / m , p[2] / m );
}
inline BasicPoint operator / (BasicPoint const & p , double m)
{
    return BasicPoint( p[0] / m , p[1] / m , p[2] / m );
}

inline BasicPoint cross( BasicPoint const & v1 , BasicPoint const & v2 )
{
    return v1 % v2;
}

inline float dot( BasicPoint const & v1 , BasicPoint const & v2 )
{
    return v1 * v2;
}

inline BasicPoint min ( BasicPoint const & p , BasicPoint const & p2 )
{
    return BasicPoint( std::min( p2[0] , p[0] ),
                       std::min( p2[1] , p[1] ),
                       std::min( p2[2] , p[2] ) );
}
inline BasicPoint max ( BasicPoint const & p , BasicPoint const & p2 )
{
    return BasicPoint( std::max( p2[0] , p[0] ),
                       std::max( p2[1] , p[1] ),
                       std::max( p2[2] , p[2] ) );
}

namespace BasicPointM
{
    inline BasicPoint random( float range )
    {
        return BasicPoint( range * (float)(rand()) / (float)( RAND_MAX ) , range * (float)(rand()) / (float)( RAND_MAX ) , range * (float)(rand()) / (float)( RAND_MAX ) );
    }
}

struct compareBasicPoint {
    inline bool operator()(const BasicPoint p1, const BasicPoint p2) const {
        if( p1[0]<p2[0] ||
           (p1[0]==p2[0] && p1[1]<p2[1]) ||
           (p1[0]==p2[0] && p1[1]==p2[1] && p1[2]<p2[2]))
            return true;
        return false;
    }
};

typedef std::map<BasicPoint, int, compareBasicPoint> BasicPointMapIndex;
#endif // BASICPOINT_H
