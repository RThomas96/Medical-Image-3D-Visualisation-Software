#ifndef MESHMANIPINTERFACE_H
#define MESHMANIPINTERFACE_H

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <cmath>

#include <glm/glm.hpp>

#include "PCATools.h"
#include "Manipulator.h"

#include "AsRigidAsPossible.h"

enum MeshModificationMode {INTERACTIVE , REALTIME};

/*
  HOW TO USE IT :
  You don't need to specify anything when constructing the object , but use :
        setMode( int m ) to specify if you want INTERACTIVE or REALTIME

  Then just fill the mesh, call reInitialize(),
  and you're done.

  EITHER
    you use open_OFF_mesh("")  and  THEN  open_OFF_cage("") and it will calculate the cage coordinates ,
  OR
    you fill the mesh and the cage BY ACCESSING  :
      get_mesh_vertices()  ,  get_mesh_triangles()  ,  get_vertices()  ,  get_visu_triangles()  ,  and  get_visu_quads(),
    AND THEN you call reInitialize().
  */

template< class point_t >
class MMInterface
{
	std::vector< point_t > vertices;
	std::vector< std::vector< int > > triangles;

	std::vector< point_t > modified_vertices;

	// MESH MANIP :
	std::vector< bool > selected_vertices;
	std::vector< bool > fixed_vertices;

	// MESH VISU :
	std::vector< std::vector< int > > visu_triangles;
	std::vector< std::vector< int > > visu_quads;

	// Deformation parameters :
	MeshModificationMode deformationMode;

	double average_edge_halfsize;

	AsRigidAsPossible ARAP;

	GLuint sphere_index;

	float sphere_scale ;

public:

	inline double getAverage_edge_halfsize(){return average_edge_halfsize;}
	//  ACCESS the input :

	inline std::vector< point_t > & get_vertices() { return vertices ; }
	inline std::vector< point_t > const & get_vertices() const { return vertices ; }
	inline int get_vertices_nb() { return vertices.size() ; }

	inline std::vector< std::vector< int > > & get_triangles() { return triangles ; }
	inline std::vector< std::vector< int > > const & get_triangles() const { return triangles ; }


	inline std::vector< std::vector< int > > & get_visu_triangles() { return visu_triangles ; }
	inline std::vector< std::vector< int > > const & get_visu_triangles() const { return visu_triangles ; }

	inline std::vector< std::vector< int > > & get_visu_quads() { return visu_quads ; }
	inline std::vector< std::vector< int > > const & get_visu_quads() const { return visu_quads ; }

	// ACCESS the output :
	inline std::vector< point_t > const & get_modified_vertices() const { return modified_vertices ; }

	inline std::vector< bool > & get_selected_vertices () { return selected_vertices; }
	inline const std::vector< bool > & get_selected_vertices () const { return selected_vertices; }

	inline std::vector< bool > & get_fixed_vertices () { return fixed_vertices; }
	inline const std::vector< bool > & get_fixed_vertices () const { return fixed_vertices; }

	std::vector< bool > get_handles_vertices (  ) {
		std::vector< bool > handles_vertices ( vertices.size(), false );
		for( unsigned int i = 0 ; i < vertices.size() ; i++ ){
			if( fixed_vertices[i] || selected_vertices[i] )
				handles_vertices[i] = true;
		}
		return handles_vertices;
	}

	void make_selected_fixed_handles(){
		for( unsigned int i = 0 ; i < vertices.size() ; i++ ){
			if(selected_vertices[i]){
				fixed_vertices[i] = true;
				selected_vertices[i] = false;
			}
		}
	}

	void setIterationNb( unsigned int itNb){
		ARAP.setIterationNb(itNb);
	}

	unsigned int getIterationNb( ){
		return ARAP.getIterationNb();
	}

	MMInterface()
	{
		deformationMode = REALTIME;
		average_edge_halfsize = 1.;
		sphere_scale = 1.;
		ARAP = AsRigidAsPossible();
	}

	~MMInterface()
	{
		// delete it if it is not used any more
		glDeleteLists(sphere_index, 1);
	}

	void set_sphere_scale( float _sphere_scale ){
		sphere_scale = _sphere_scale;
		build_sphere_list();
	}

	void build_sphere_list(){
		glDeleteLists(sphere_index, 1);
		sphere_index = glGenLists(1);
		glEnable(GL_LIGHTING);
		// compile the display list
		glNewList(sphere_index, GL_COMPILE);
		BasicGL::drawSphere( 0., 0., 0., sphere_scale*average_edge_halfsize/2., 15, 15 );
		glEndList();
	}

	void setMode( int m )
	{
		deformationMode = MeshModificationMode(m);
	}

	void drawPoints( std::vector< int > & pts )
	{
		glBegin( GL_POINTS );
		for( unsigned int i = 0 ; i < pts.size() ; ++i )
			glVertex( modified_vertices[ pts[i] ] );
		glEnd();
	}

	void clear()
	{
		modified_vertices.clear();

		vertices.clear();
		triangles.clear();

		selected_vertices.clear();
		fixed_vertices.clear();

		visu_triangles.clear();
		visu_quads.clear();

		ARAP.clear();
		average_edge_halfsize = 1.;
	}

	void compute_max_sphere_radius()
	{
		average_edge_halfsize = 0.;
		for( unsigned int t = 0 ; t < triangles.size(); ++t )
		{
			for( int i = 0 ; i < 3 ; i ++ ){
				double elenght = glm::length(vertices[ triangles[t][(i+1)%3] ] - vertices[ triangles[t][i] ] );
				average_edge_halfsize += elenght;
			}
		}

		average_edge_halfsize = average_edge_halfsize/(2.*triangles.size()*3.);

		build_sphere_list();
	}

	void open_OFF( std::string const & filename )
	{
		std::ifstream myfile;
		myfile.open(filename.c_str());
		if (!myfile.is_open())
		{
			std::cout << filename << " cannot be opened" << std::endl;
			return;
		}

		std::string magic_s;

		myfile >> magic_s;

		if( magic_s != "OFF" )
		{
			std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
			myfile.close();
			exit(1);
		}

		int n_vertices , n_faces , dummy_int;
		myfile >> n_vertices >> n_faces >> dummy_int;

		vertices.clear();

		selected_vertices.clear();
		selected_vertices.resize(n_vertices , false);
		fixed_vertices.clear();
		fixed_vertices.resize(n_vertices , false);

		for( int v = 0 ; v < n_vertices ; ++v )
		{
			float x , y , z;
			myfile >> x >> y >> z;
			vertices.push_back( point_t( x , y , z ) );
		}

		triangles.clear();
		visu_triangles.clear();
		visu_quads.clear();

		for( int f = 0 ; f < n_faces ; ++f )
		{
			int n_vertices_on_face;
			myfile >> n_vertices_on_face;
			if( n_vertices_on_face == 3 )
			{
				int _v1 , _v2 , _v3;
				std::vector< int > _v;
				myfile >> _v1 >> _v2 >> _v3;
				_v.push_back( _v1 );
				_v.push_back( _v2 );
				_v.push_back( _v3 );

				triangles.push_back( _v );

				// VISU :
				visu_triangles.push_back( _v );
			}
			else if( n_vertices_on_face == 4 )
			{
				int _v1 , _v2 , _v3 , _v4;
				std::vector< int > _v;
				myfile >> _v1 >> _v2 >> _v3 >> _v4;
				_v.push_back( _v1 );
				_v.push_back( _v2 );
				_v.push_back( _v3 );

				triangles.push_back( _v );

				_v.clear();
				_v.push_back( _v1 );
				_v.push_back( _v3 );
				_v.push_back( _v4 );

				triangles.push_back( _v );

				// VISU :
				_v.clear();
				_v.push_back( _v1 );
				_v.push_back( _v2 );
				_v.push_back( _v3 );
				_v.push_back( _v4 );
				visu_quads.push_back( _v );
			}
			else
			{
				std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
				myfile.close();
				exit(1);
			}
		}

		compute_max_sphere_radius();

		ARAP.clear();
		ARAP.init( modified_vertices, triangles );
	}

	void loadAndInitialize(const std::vector<point_t> & _vertices , const std::vector<Triangle> & _triangles )
	{
		clear();

		for( unsigned int i = 0 ; i < _vertices.size() ; i++ ){
			vertices.push_back( _vertices[i] );
			modified_vertices.push_back( _vertices[i] );
		}

		selected_vertices.resize(vertices.size() , false);
		fixed_vertices.resize(vertices.size() , false);

		for( unsigned int i = 0 ; i < _triangles.size() ; i ++ ){
			const Triangle & T = _triangles[i] ;
			addFace( T.getVertex(0) , T.getVertex(1) , T.getVertex(2) );
		}

		compute_max_sphere_radius();

		ARAP.clear();
		ARAP.init( modified_vertices, triangles );
	}

	void addFace(int _v1, int _v2, int _v3){
		std::vector< int > _v;
		_v.push_back( _v1 );
		_v.push_back( _v2 );
		_v.push_back( _v3 );
		triangles.push_back( _v );
	}

	void addFace(int _v1, int _v2, int _v3, int _v4){
		addFace( _v1 , _v2, _v3 );
		addFace( _v1 , _v3, _v4 );

		// VISU :
		std::vector< int > _v;
		_v.push_back( _v1 );
		_v.push_back( _v2 );
		_v.push_back( _v3 );
		_v.push_back( _v4 );
		visu_quads.push_back( _v );
	}

	// This function gives you the index for the cage face you clicked on :  TODO
	int clickedFace( int x , int y , bool & isAQuad )
	{
		return -1;
	}

	void drawOrdered()
	{
		float modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

		std::priority_queue< std::pair< float , int > , std::deque< std::pair< float , int > > , std::greater< std::pair< float , int > > > facesQueue;

		for (unsigned int t = 0 ; t < visu_triangles.size() ; ++t )
		{
			point_t _center = (
								modified_vertices[ visu_triangles[t][0] ]+
								modified_vertices[ visu_triangles[t][1] ]+
								modified_vertices[ visu_triangles[t][2] ]) / 3.f;
			facesQueue.push(
			  std::make_pair(
				modelview[2] * _center[0] + modelview[6] * _center[1] + modelview[10] * _center[2] + modelview[14] ,
				t
			  )
			);
		}

		for (unsigned int q = 0 ; q < visu_quads.size() ; ++q )
		{
			point_t _center = (
								modified_vertices[ visu_quads[q][0] ]+
								modified_vertices[ visu_quads[q][1] ]+
								modified_vertices[ visu_quads[q][2] ]+
								modified_vertices[ visu_quads[q][3] ]) / 4.f;
			facesQueue.push(
			  std::make_pair(
				modelview[2] * _center[0] + modelview[6] * _center[1] + modelview[10] * _center[2] + modelview[14] ,
				q + visu_triangles.size()
			  )
			);
		}

		while( ! facesQueue.empty() )
		{
			int face_index = facesQueue.top().second ;
			if( face_index >= (int)(visu_triangles.size()) )
			{
				// Display quad number face_index - visu_triangles.size() :
				face_index -= visu_triangles.size();
				glBegin(GL_QUADS);
				for( unsigned int v = 0 ; v < 4 ; ++v )
					glVertex( modified_vertices[ visu_quads[face_index][v] ] );
				glEnd();
			}
			else
			{
				// Display triangle number face_index :
				glBegin(GL_TRIANGLES);
				for( unsigned int v = 0 ; v < 3 ; ++v ){
					glVertex( modified_vertices[ visu_triangles[face_index][v] ] );
				}
				glEnd();
			}
			facesQueue.pop();
		}
	}

	//  You  don't  really  need  that :
	void draw(  )
	{
		glBegin( GL_TRIANGLES );
		// Draw triangles :
		for (unsigned int t = 0 ; t < visu_triangles.size() ; ++t )
		{
			for( unsigned int v = 0 ; v < 3 ; ++v )
				glVertex( modified_vertices[ visu_triangles[t][v] ] );
		}
		glEnd();
		// Draw quads :
		glBegin( GL_QUADS );
		for (unsigned int q = 0 ; q < visu_quads.size() ; ++q )
		{
			for( unsigned int v = 0 ; v < 4 ; ++v )
				glVertex( modified_vertices[ visu_quads[q][v] ] );
		}
		glEnd();
	}

	void drawSelectedVertices()
	{
		for( unsigned int v = 0 ; v < modified_vertices.size() ; v++ ){
			point_t & center = modified_vertices[v];

			bool draw_sphere = true;
			if( selected_vertices[ v ] ){
				glColor3f(0.8,0.,0.);
			} else if( fixed_vertices[ v ] ) {
				glColor3f(0.,0.8,0.);
			} else {
				draw_sphere = false;
			}

			if(draw_sphere){
				glPushMatrix();
				glTranslatef(center[0], center[1], center[2]);
				// draw the display list
				glCallList(sphere_index);
				glPopMatrix();
			}
		}
	}

	void drawTriangulatedFaceNormals(float scale)
	{
		glBegin( GL_LINES );
		for( unsigned int t = 0 ; t < triangles.size() ; ++t )
		{
			point_t const & center = ( modified_vertices[ triangles[t][0] ] + modified_vertices[ triangles[t][1] ] + modified_vertices[ triangles[t][2] ] ) / 3.f;
			point_t normal = ( modified_vertices[ triangles[t][1] ] - modified_vertices[ triangles[t][0] ] ) % ( modified_vertices[ triangles[t][2] ] - modified_vertices[ triangles[t][0] ] );
			normal = glm::normalize(normal);

			glVertex( center );
			glVertex( center+normal*scale );
		}
		glEnd();
	}

	// You need to have the correct QOpenGLContext activated for that !!!!!!!!
	// This function select the cage vertices drawn inside the QRect "zone"
	void select( QRectF const & zone, float *modelview, float * projection, bool moving = true )
	{
		for( unsigned int v = 0 ; v < modified_vertices.size() ; ++v )
		{
			point_t const & p = modified_vertices[ v ];

			float x = modelview[0] * p[0] + modelview[4] * p[1] + modelview[8] * p[2] + modelview[12];
			float y = modelview[1] * p[0] + modelview[5] * p[1] + modelview[9] * p[2] + modelview[13];
			float z = modelview[2] * p[0] + modelview[6] * p[1] + modelview[10] * p[2] + modelview[14];
			float w = modelview[3] * p[0] + modelview[7] * p[1] + modelview[11] * p[2] + modelview[15];
			x /= w;
			y /= w;
			z /= w;
			w = 1.f;

			float xx = projection[0] * x + projection[4] * y + projection[8] * z + projection[12] * w;
			float yy = projection[1] * x + projection[5] * y + projection[9] * z + projection[13] * w;
			float ww = projection[3] * x + projection[7] * y + projection[11] * z + projection[15] * w;
			xx /= ww;
			yy /= ww;

			xx = ( xx + 1.f ) / 2.f;
			yy = 1.f - ( yy + 1.f ) / 2.f;

			if( zone.contains( xx , yy ) ){
				selected_vertices[ v ] = moving;
				fixed_vertices[ v ] = !moving;
			}
		}
	}

	int index_of_closest_point_in_sphere( const point_t & clicked, float radius ){
		int result = -1;
		float min_dist = FLT_MAX;
		for( unsigned int v = 0 ; v < modified_vertices.size() ; ++v )
		{
			float dist = glm::length(modified_vertices[v] - clicked);
			if( dist <= radius && dist < min_dist ){
				result = static_cast<float>(v);
				min_dist = dist;
			}
		}

		return result;
	}

	void select( const point_t & clicked, float scale )
	{
		int v = index_of_closest_point_in_sphere( clicked, scale*average_edge_halfsize );
		if( v >= 0 ){
			selected_vertices[ v ] = !selected_vertices[ v ];
			fixed_vertices[ v ] = false;
		}
	}

	void make_fixed_handles( const point_t & clicked, float scale )
	{
		int v = index_of_closest_point_in_sphere( clicked, scale*average_edge_halfsize );
		if( v >= 0 ){
			bool fixed  = fixed_vertices[ v ] ;
			fixed_vertices[ v ] = !fixed;
			selected_vertices[ v ] = fixed;
		}
	}

	// You need to have the correct QOpenGLContext activated for that !!!!!!!!
	// This function unselect the cage vertices drawn inside the QRect "zone"
	void unselect( QRectF const & zone, float *modelview, float * projection )
	{
		for( unsigned int v = 0 ; v < modified_vertices.size() ; ++v )
		{
			point_t const & p = modified_vertices[ v ];

			float x = modelview[0] * p[0] + modelview[4] * p[1] + modelview[8] * p[2] + modelview[12];
			float y = modelview[1] * p[0] + modelview[5] * p[1] + modelview[9] * p[2] + modelview[13];
			float z = modelview[2] * p[0] + modelview[6] * p[1] + modelview[10] * p[2] + modelview[14];
			float w = modelview[3] * p[0] + modelview[7] * p[1] + modelview[11] * p[2] + modelview[15];
			x /= w;
			y /= w;
			z /= w;
			w = 1.f;

			float xx = projection[0] * x + projection[4] * y + projection[8] * z + projection[12] * w;
			float yy = projection[1] * x + projection[5] * y + projection[9] * z + projection[13] * w;
			float ww = projection[3] * x + projection[7] * y + projection[11] * z + projection[15] * w;
			xx /= ww;
			yy /= ww;

			xx = ( xx + 1.f ) / 2.f;
			yy = 1.f - ( yy + 1.f ) / 2.f;

			if( zone.contains( xx , yy ) ){
				selected_vertices[ v ] = false;
				fixed_vertices[ v ] = false;
			}
		}
	}

	void clear_selection(){
		for(unsigned int v = 0 ; v < selected_vertices.size() ; v++ ){
			selected_vertices[ v ] = false;
			fixed_vertices[ v ] = false;
		}
	}

	void select_all(){
		for(unsigned int v = 0 ; v < selected_vertices.size() ; v++ ){
			if(!fixed_vertices[ v ]){
				selected_vertices[ v ] = true;
			}
		}
	}

	void unselect_all(){
		for(unsigned int v = 0 ; v < selected_vertices.size() ; v++ ){
			selected_vertices[ v ] = false;
		}
	}

	void fixe_all(){
		for(unsigned int v = 0 ; v < selected_vertices.size() ; v++ ){
			if(!selected_vertices[ v ]){
				fixed_vertices[ v ] = true;
			}
		}
	}

	void unfixe_all(){
		for(unsigned int v = 0 ; v < fixed_vertices.size() ; v++ ){
			fixed_vertices[ v ] = false;
		}
	}

	// Once you're OK with the selection you made , call this function to compute the manipulator , and it will activate it :
	void computeManipulatorForSelection( SimpleManipulator * manipulator )
	{
		manipulator->resetScales();
		manipulator->clear();

		int nb=0;
		glm::vec3 oo( 0.f , 0.f , 0.f );
		for( unsigned int v = 0 ; v < modified_vertices.size() ; ++v )
		{
			if( selected_vertices[v] )
			{
				point_t const & p = modified_vertices[v];
				oo += glm::vec3( p[0] , p[1] , p[2] );
				++nb;
			}
		}
		oo /= nb;

		PCATools::PCASolver3f< glm::vec3 , glm::vec3 > solver;
		std::cerr << "Set solver origin to " << oo.x << ", " << oo.y << ", " << oo.z << "\n";
		solver.setOrigine( oo );

		for( unsigned int v = 0 ; v < modified_vertices.size() ; ++v )
		{
			if( selected_vertices[v] )
			{
				point_t const & p = modified_vertices[v];
				solver.addPoint( glm::vec3( p[0] , p[1] , p[2] ) );
			}
		}

		solver.compute();

		manipulator->setOrigine( oo );
		manipulator->setRepX( solver.RepX() );
		manipulator->setRepY( solver.RepY() );
		manipulator->setRepZ( solver.RepZ() );

		for( unsigned int v = 0 ; v < modified_vertices.size() ; ++v )
		{
			if( selected_vertices[v] )
			{
				/// ! Changed ! ///
				//selected_vertices[v] = false;
				point_t const & p = modified_vertices[v];
				manipulator->addPoint( v , glm::vec3( p[0] , p[1] , p[2] ) );
			}
		}

		manipulator->activate();

		ARAP.setHandles(get_handles_vertices());
	}

	void setToPosition( const std::vector<point_t> & _vertices )
	{
		unsigned int n_points = _vertices.size();

		if(modified_vertices.size() != n_points) return;

		for( unsigned int i = 0 ; i < n_points ; ++i )
		{
			modified_vertices[ i ] = _vertices[i];
		}

		ARAP.clear();
		ARAP.init(modified_vertices, triangles);

	}

	// When you move your manipulator , it sends you a SIGNAL.
	// When it happens, call that function with the manipulator as the parameter, it will update everything :
	void changed( SimpleManipulator * manipulator )
	{
		unsigned int n_points = manipulator->n_points();
		glm::vec3 p;
		int idx;
		for( unsigned int i = 0 ; i < n_points ; ++i )
		{
			manipulator->getTransformedPoint( i , idx , p );

			modified_vertices[ idx ] = point_t( p[0] , p[1] , p[2] );
		}

		if( deformationMode == REALTIME )
		{
			ARAP.compute_deformation(modified_vertices);
		}
	}

	// When you move your manipulator , it sends you a SIGNAL.
	// When it happens, call that function with the manipulator as the parameter, it will update everything :
	void changedConstraints( std::vector<std::pair<int, point_t> > & input_def )
	{
		glm::vec3 p;
		std::vector<bool> handles (false, vertices.size() );

		for( unsigned int i = 0 ; i < input_def.size() ; ++i )
		{
			modified_vertices[ input_def[i].first ] = point_t( input_def[i].second[0] , input_def[i].second[1] , input_def[i].second[2] );
			handles[ input_def[i].first ] = true;
		}

		if( deformationMode == REALTIME )
		{
			ARAP.setHandles( handles );
			ARAP.compute_deformation( modified_vertices );
		}
	}

	// When you release the mouse after moving the manipulator, it sends you a SIGNAL.
	// When it happens, call that function with the manipulator as the parameter, it will update everything :
	void manipulatorReleased()
	{
		if( deformationMode == INTERACTIVE )
		{
			ARAP.compute_deformation(modified_vertices);
		}
	}

};

#endif // MESHMANIPINTERFACE_H
