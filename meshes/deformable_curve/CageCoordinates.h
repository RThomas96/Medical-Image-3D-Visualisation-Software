#ifndef CAGECOORDINATES_H
#define CAGECOORDINATES_H

#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>


namespace URAGO
{
	template< class point_t >
	double __signed_solid_angle(point_t const & a, point_t const & b, point_t const & c)
	{
		double det = point_t::dotProduct(a,point_t::crossProduct(b,c));
		if( fabs(det) < 0.0000000001 ) // then you're on the limit case where you cover half the sphere
			return M_2_PI;

		double al = (a).norm();
		double bl = (b.norm());
		double cl = (c).norm();

		double div = al*bl*cl + point_t::dotProduct(a,b)*cl + point_t::dotProduct(a,c)*bl + point_t::dotProduct(b,c)*al;
		double at = std::atan2( std::abs(det), div);
		if(at < 0)
		{
			at += M_PI; // If det>0 && div<0 atan2 returns < 0, so add pi.
			//    at = -at;
		}
		double omega = 2.0f * at;

		if(det > 0.f)
			return omega;

		return -omega;
	}



	// You can find these expressions in URAGO 2000 :
	// "Analytical integrals of fundamental solution of three-dimensional Laplace equation and their gradients"
	// or in BEN-CHEN 2008 :
	// "Variational Harmonic Maps for Space Deformation" where they appear in Appendix

	template< class point_t >
	void computeCoordinatesOf3dPoint(
			point_t const & eta ,
			std::vector< std::vector< int > > const & cage_triangles ,
			std::vector< point_t > const & cage_vertices ,
			std::vector< point_t > const & cage_normals ,
			std::vector< double > & _VC_phi ,
			std::vector< double > & _FC_psi)
	{
		assert( cage_normals.size() == cage_triangles.size()   &&    "cage_normals.size() != cage_triangles.size()" );

		_VC_phi.clear();
		_VC_phi.resize( cage_vertices.size() , 0.f );
		_FC_psi.clear();
		_FC_psi.resize( cage_triangles.size() , 0.f );

		// iterate over the triangles:
		for( unsigned int t = 0 ; t < cage_triangles.size() ; ++t )
		{
			std::vector<int> const & tri = cage_triangles[t];
			point_t const & Nt = cage_normals[t];

			point_t e[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				e[v] = cage_vertices[tri[v]] - eta;

			double e_norm[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				e_norm[v] =  e[v].norm();

			point_t e_normalized[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				e_normalized[v] = e[v] / e_norm[v];

			double signed_solid_angle = __signed_solid_angle
												(e_normalized[0],
												 e_normalized[1],
												 e_normalized[2]) / (4.f * M_PI);

			double signed_volume = point_t::dotProduct( point_t::crossProduct(e[0],e[1]) , e[2] ) / 6.f;

			double At = point_t::crossProduct(
					cage_vertices[tri[1]]-cage_vertices[tri[0]],
					cage_vertices[tri[2]]-cage_vertices[tri[0]]).norm() / 2.f;

			double R[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				R[v] = e_norm[(v+1)%3] + e_norm[(v+2)%3];

			point_t d[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				d[v] = cage_vertices[tri[(v+1)%3]] - cage_vertices[tri[(v+2)%3]];

			double d_norm[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				d_norm[v] =  d[v].norm();

			double C[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				C[v] = std::log( (R[v] + d_norm[v]) / (R[v] - d_norm[v]) ) / (4.f * M_PI * d_norm[v]);

			point_t Pt( - signed_solid_angle * Nt );
			for( unsigned int v = 0 ; v < 3 ; ++v )
				Pt += point_t::crossProduct( Nt , C[v]*d[v] );

			point_t J[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
			{
				J[v] = point_t::crossProduct( e[(v+2)%3] , e[(v+1)%3] );
			}

			_FC_psi[t] = - 3.f * signed_solid_angle * signed_volume / At ;
			for( unsigned int v = 0 ; v < 3 ; ++v )
				_FC_psi[t] -= C[v]* point_t::dotProduct(J[v],Nt);

			for( unsigned int v = 0 ; v < 3 ; ++v )
			{
				_VC_phi[ tri[v] ] += point_t::dotProduct(Pt , J[v]) / (2.f * At);
			}
		}
	}




	template< class point_t >
	void computeCoordinatesOf3dPoint(
			point_t const & eta ,
			std::vector< std::vector< int > > const & cage_triangles ,
			std::vector< point_t > const & cage_vertices ,
			std::vector< point_t > const & cage_normals ,
			std::vector< float > & _VC_phi ,
			std::vector< float > & _FC_psi ,
			std::vector< point_t > & _VC_phi_gradient ,
			std::vector< point_t > & _FC_psi_gradient )
	{
		assert( cage_normals.size() == cage_triangles.size()   &&    "cage_normals.size() != cage_triangles.size()" );

		_VC_phi.clear();
		_VC_phi.resize( cage_vertices.size() , 0.f );
		_FC_psi.clear();
		_FC_psi.resize( cage_triangles.size() , 0.f );

		_VC_phi_gradient.clear();
		_VC_phi_gradient.resize( cage_vertices.size() , point_t(0.f,0.f,0.f) );
		_FC_psi_gradient.clear();
		_FC_psi_gradient.resize( cage_triangles.size() , point_t(0.f,0.f,0.f) );
		// iterate over the triangles:
		for( unsigned int t = 0 ; t < cage_triangles.size() ; ++t )
		{
			std::vector<int> const & tri = cage_triangles[t];
			point_t const & Nt = cage_normals[t];

			point_t e[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				e[v] = cage_vertices[tri[v]] - eta;

			float e_norm[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				e_norm[v] =  e[v].norm();

			point_t e_normalized[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				e_normalized[v] = e[v] / e_norm[v];

			float signed_solid_angle = __signed_solid_angle
											   (e_normalized[0],
												e_normalized[1],
												e_normalized[2]) / (4.f * M_PI);

			float signed_volume = point_t::dotProduct( point_t::crossProduct(e[0],e[1]) , e[2] ) / 6.f;

			float At = point_t::crossProduct(
					cage_vertices[tri[1]]-cage_vertices[tri[0]],
					cage_vertices[tri[2]]-cage_vertices[tri[0]]).norm() / 2.f;

			float R[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				R[v] = e_norm[(v+1)%3] + e_norm[(v+2)%3];

			point_t d[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				d[v] = cage_vertices[tri[(v+1)%3]] - cage_vertices[tri[(v+2)%3]];

			float d_norm[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				d_norm[v] =  d[v].norm();

			float C[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				C[v] = std::log( (R[v] + d_norm[v]) / (R[v] - d_norm[v]) ) / (4.f * M_PI * d_norm[v]);


			point_t Pt( - signed_solid_angle * Nt );
			for( unsigned int v = 0 ; v < 3 ; ++v )
				Pt += point_t::crossProduct( Nt , C[v]*d[v] );

			point_t J[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
			{
				J[v] = point_t::crossProduct( e[(v+2)%3] , e[(v+1)%3] );
			}

			_FC_psi[t] = - 3.f * signed_solid_angle * signed_volume / At ;
			for( unsigned int v = 0 ; v < 3 ; ++v )
			{
				_FC_psi[t] -= C[v]* point_t::point_t::dotProduct(J[v],Nt);
				_FC_psi_gradient[t] = -Pt;
			}

			for( unsigned int v = 0 ; v < 3 ; ++v )
			{
				_VC_phi[ tri[v] ] += point_t::point_t::dotProduct(Pt , J[v]) / (2.f * At);
				_VC_phi_gradient[ tri[v] ] += point_t::point_t::crossProduct(Pt,d[v]) / (2.f * At);
			}


			float C_tilde[3];
			for( unsigned int v = 0 ; v < 3 ; ++v )
				C_tilde[v] = 1.f / (M_2_PI * (R[v] + d_norm[v]) * (R[v] - d_norm[v]));

		}
	}
}

namespace GreenCoords
{
	template< class normal_t >
	class GreenScalingFactor
	{
		normal_t _u;
		normal_t _v;
		// u is the first halfedge of the triangle (v1 - v0) , and v is the second one (v2 - v1).

		float _s;

	public:
		GreenScalingFactor( normal_t const & u , normal_t const & v ) : _u( u ) , _v( v ) , _s( 1.f ) {}

		void computeScalingFactor( normal_t const & u , normal_t const & v )
		{
			_s = sqrt(
					( (_u.sqrnorm()) * (v.sqrnorm())  -  2.f*( _u * _v )*( u * v ) + (u.sqrnorm()) * (_v.sqrnorm()) ) /
					( 2.f * ( _u % _v ).sqrnorm() )
			);
		}
		float computeScalingFactorWithoutUpdatingIt( normal_t const & u , normal_t const & v )
		{
			return sqrt(
					( (_u.sqrnorm()) * (v.sqrnorm())  -  2.f*( _u * _v )*( u * v ) + (u.sqrnorm()) * (_v.sqrnorm()) ) /
					( 2.f * ( _u % _v ).sqrnorm() )
			);
		}

		float scalingFactor()
		{
			return _s;
		}
	};


	template< class point_t >
	float GCTriInt( point_t const & p , point_t const & v1 , point_t const & v2 , point_t const & eta )
	{
		float c_alpha = std::max( std::min( point_t::dotProduct( v2 - v1 , p - v1 ) / ( (v2 - v1).norm() * (p - v1).norm() ) , 1.f ) , -1.f );
		float alpha = std::acos( c_alpha );
		float s_alpha = std::sin( alpha );

		float surfaceSqr = ( point_t::crossProduct( v2 - v1 , p - v1 ) ).getSquaredLength();
		if( surfaceSqr < 0.00000001f )
		{
			//  surface of the triangle is too low : return 0;
			return 0.f;
		}

		float c_beta = std::max( std::min( point_t::dotProduct( v1 - p , v2 - p ) / ( (v1 - p).norm() * (v2 - p).norm() ) , 1.f ) , -1.f );
		float beta = std::acos( c_beta );

		float lambda = ( p - v1 ).getSquaredLength() * s_alpha * s_alpha;
		float sqrt_lambda = std::sqrt( lambda );

		float c = ( p - eta ).getSquaredLength();
		float sqrt_c = std::sqrt(c);

		double theta[2] = { M_PI - alpha , M_PI - alpha - beta };

		float I_theta[2];
		for( int i = 0 ; i < 2 ; ++i )
		{
			float S = std::sin( theta[i] ) , C = std::cos( theta[i] );

			if( std::abs( C ) >= 0.99999999f )
				return 0.f;

			if( std::abs( S ) <= 0.00000001f )
				return 0.f;

			if( std::abs( theta[i] ) < 0.001f )
				return 0.f;

			float sign_S_div_by_2 = (S < 0.f) ? -0.5f : 0.5f;
			I_theta[ i ] = - sign_S_div_by_2 *
						   ( 2.f * sqrt_c * std::atan( sqrt_c * C / std::sqrt( lambda + S * S * c ) )
							 + sqrt_lambda * std::log( ( 2.f*sqrt_lambda*S*S / ( (1.f-C)*(1.f-C) ) )   *  (1.f - 2.f*c*C / ( c*(1.f+C) + lambda + std::sqrt( lambda * lambda + lambda * c * S*S ) ) ) )
						   );

			if( std::isinf( I_theta[ i ] ) || std::isnan( I_theta[ i ] ) )
			{
				// RAHHHHH !!!!!!!!
				return 0.f;
			}
		}
		return (-0.25f / M_PI) * std::abs( I_theta[0] - I_theta[1] - sqrt_c*beta );
	}



	template< class point_t >
	void computeGreenCoordinatesOf3dPoint(
			point_t const & eta ,
			std::vector< std::vector< int > > const & cage_triangles ,
			std::vector< point_t > const & cage_vertices ,
			std::vector< point_t > const & cage_normals ,
			std::vector< float > & _VC_phi ,
			std::vector< float > & _FC_psi)
	{
		assert( cage_normals.size() == cage_triangles.size()   &&    "cage_normals.size() != cage_triangles.size()" );

		float epsilon = 0.01f;
		float epsilonSqr = epsilon * epsilon;

		_VC_phi.clear();
		_FC_psi.clear();
		_VC_phi.resize( cage_vertices.size() , 0.f );
		_FC_psi.resize( cage_triangles.size() , 0.f );

		for( unsigned int fi = 0 ; fi < cage_triangles.size() ; ++fi )
		{
			point_t vj[ 3 ];
			int v[3];
			std::vector< int > const & fh = cage_triangles[ fi ];
			for( int l = 0 ; l < 3 ; ++l )
			{
				v[l] = fh[ l ];
				vj[ l ] = cage_vertices[ v[l] ] - eta;
			}

			point_t const & nj = cage_normals[ fi ];
			point_t const & p = ( vj[0] * nj ) * nj;

			float s[3];
			float I[3];
			float II[3];
			point_t q[3];
			point_t N[3];
			for( int l = 0 ; l < 3 ; ++l )
			{
				s[l] = ( point_t::dotProduct( point_t::crossProduct( vj[ l ] - p , vj[ (l+1) % 3 ] - p ) , nj ) ) < 0.f ? -1.f : 1.f;
				I[ l ] = GCTriInt( p , vj[l] , vj[ (l+1) % 3 ] , point_t( 0.f , 0.f , 0.f ) );
				II[ l ] = GCTriInt( point_t( 0.f , 0.f , 0.f ) , vj[ (l+1) % 3 ] , vj[l] , point_t( 0.f , 0.f , 0.f ) );
				q[ l ] = point_t::crossProduct( vj[ (l+1) % 3 ] , vj[l] );
				N[ l ] = q[ l ];
				N[ l ].normalize();
			}

			float Itotal = - fabs( s[0] * I[0] + s[1] * I[1] + s[2] * I[2] );

			_FC_psi[ fi ] = -Itotal;

			point_t w = Itotal * nj + ( II[0] * N[0] + II[1] * N[1] + II[2] * N[2] );

			if( w.getSquaredLength() >= epsilonSqr )
			{
				for( int l = 0 ; l < 3 ; ++l )
					_VC_phi[ v[l] ] += point_t::dotProduct( N[ (l+1) % 3 ] , w ) / point_t::dotProduct( N[ (l+1) % 3 ] , vj[l] );
			}
		}
	}
}

namespace MVCCoords
{
	template< class point_t, typename iterator_t >
	void computeMVCCoordinatesOf3dPoint(
			point_t const & eta ,
			std::vector< std::vector< iterator_t > > const & cage_triangles ,
			std::vector< point_t > const & cage_vertices ,
			std::vector< point_t > const & cage_normals ,
			std::vector< std::pair< iterator_t , float > > & _phi)
	{
		assert( cage_normals.size() == cage_triangles.size()   &&    "cage_normals.size() != cage_triangles.size()" );
		float epsilon = 0.000001f;

		_phi.clear();
		float totalWeight = 0.f;

		std::vector< float > d( cage_vertices.size() , 0.f );
		std::vector< point_t > u( cage_vertices.size() );

		for( iterator_t v = 0 ; v < cage_vertices.size() ; ++v )
		{
			d[ v ] = glm::length( eta - cage_vertices[ v ] );
			if( d[ v ] < epsilon )
			{
				_phi.push_back( std::make_pair( v , 1.f ) );
				return;
			}
			u[ v ] = ( cage_vertices[v] - eta ) / d[v];
		}

		std::vector< float > complete_phi(  cage_vertices.size() , 0.f );

		iterator_t vid[3];
		float l[3];
		float theta[3] ;
		float w[3];
		float c[3];
		float s[3];

		for( iterator_t t = 0 ; t < cage_triangles.size() ; ++t )
		{
			// the Norm is CCW :
			for( iterator_t i = 0 ; i <= 2 ; ++i )
			{
				vid[i] =  cage_triangles[t][i];
			}
			for( iterator_t i = 1 ; i <= 3 ; ++i )
			{
				l[ i % 3 ] = glm::length( u[ vid[ ( i + 1 ) % 3 ] ] - u[ vid[ ( i - 1 ) % 3 ] ] );
			}

			for( iterator_t i = 0 ; i <= 2 ; ++i )
			{
				theta[i] = 2.f * std::asin( l[i] / 2.f );
			}

			float h = ( theta[0] + theta[1] + theta[2] ) / 2.f;

			if( M_PI - h < epsilon )
			{
				// eta is on the triangle t , use 2d barycentric coordinates :
				_phi.clear();
				for( iterator_t i = 1 ; i <= 3 ; ++i )
				{
					w[ i % 3 ] = std::sin( theta[ i % 3 ] ) * l[ (i-1) % 3 ] * l[ (i+1) % 3 ];
				}
				totalWeight = w[0] + w[1] + w[2];

				_phi.push_back( std::make_pair( vid[0] , w[0] / totalWeight ) );
				_phi.push_back( std::make_pair( vid[1] , w[1] / totalWeight ) );
				_phi.push_back( std::make_pair( vid[2] , w[2] / totalWeight ) );
				return;
			}

			for( iterator_t i = 1 ; i <= 3 ; ++i )
				c[ i % 3 ] = ( 2.f * std::sin(h) * std::sin(h - theta[ i % 3 ]) ) / ( std::sin(theta[ (i+1) % 3 ]) * std::sin(theta[ (i-1) % 3 ]) ) - 1.f;

			float sign_Basis_u0u1u2 = 1.f;
			//if( (u[vid[0]] % u[vid[1]])*(u[vid[2]]) < 0.f )
			//    sign_Basis_u0u1u2 = -1.f;

			for( iterator_t i = 1 ; i <= 3 ; ++i )
				s[ i % 3 ] = sign_Basis_u0u1u2 * std::sqrt( std::max( 0.0 , 1.0 - c[ i % 3 ] * c[ i % 3 ] ) );

			if( fabs( s[0] ) < epsilon   ||   fabs( s[1] ) < epsilon   ||   fabs( s[2] ) < epsilon )
			{
				// eta is on the same plane, outside t  ->  ignore triangle t :
				continue;
			}

			float w[3];
			for( iterator_t i = 1 ; i <= 3 ; ++i )
				w[ i % 3 ] = ( theta[ i % 3 ] - c[ (i+1)% 3 ]*theta[ (i-1) % 3 ] - c[ (i-1) % 3 ]*theta[ (i+1) % 3 ] ) / ( d[ vid[i % 3] ] * std::sin( theta[ (i+1) % 3 ] ) * s[ (i-1) % 3 ] );

			totalWeight += ( w[0] + w[1] + w[2] );
			complete_phi[ vid[0] ] += w[0];
			complete_phi[ vid[1] ] += w[1];
			complete_phi[ vid[2] ] += w[2];
		}

		for( iterator_t v = 0 ; v < cage_vertices.size() ; ++v )
		{
			if( complete_phi[ v ] != 0.f )
				_phi.push_back( std::make_pair( v , complete_phi[v] / totalWeight ) );
		}
	}
}

#endif // CAGECOORDINATES_H
