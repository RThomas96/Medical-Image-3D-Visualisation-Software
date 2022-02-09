#include <vector>
#include <glm/glm.hpp>
#include <math.h>

void fromGlmToRaw(const glm::mat3& mat, float res[3][3]) {
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            res[i][j] = mat[i][j];
}

void fromRawToGlm(const float mat[3][3], glm::mat3& res) {
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            res[i][j] = mat[i][j];
}

template<typename Real>
void Invert(Real M_inv[3][3],const Real M[3][3])
{
    Real det = M[0][0] * ( M[1][1]*M[2][2] - M[2][1]*M[1][2] )- M[0][1] * ( M[1][0]*M[2][2] - M[2][0]*M[1][2] ) + M[0][2] * ( M[1][0]*M[2][1] - M[2][0]*M[1][1] );
    if(det==0) {M_inv[0][0]=1;	M_inv[0][1]=0;	M_inv[0][2]=0; M_inv[1][0]=0;	M_inv[1][1]=1;	M_inv[1][2]=0; M_inv[2][0]=0;	M_inv[2][1]=0;	M_inv[2][2]=1; return;}
    M_inv[0][0] =  ( M[1][1]*M[2][2] - M[1][2]*M[2][1] ) / det;	M_inv[0][1] = -( M[0][1]*M[2][2] - M[2][1]*M[0][2] ) / det;	M_inv[0][2] =  ( M[0][1]*M[1][2] - M[1][1]*M[0][2] ) / det;
    M_inv[1][0] = -( M[1][0]*M[2][2] - M[1][2]*M[2][0] ) / det;	M_inv[1][1] =  ( M[0][0]*M[2][2] - M[2][0]*M[0][2] ) / det;	M_inv[1][2] = -( M[0][0]*M[1][2] - M[1][0]*M[0][2] ) / det;
    M_inv[2][0] =  ( M[1][0]*M[2][1] - M[2][0]*M[1][1] ) / det;	M_inv[2][1] = -( M[0][0]*M[2][1] - M[2][0]*M[0][1] ) / det;	M_inv[2][2] =  ( M[0][0]*M[1][1] - M[0][1]*M[1][0] ) / det;
}

template<typename Real>
void Mult(Real p[3],const Real A[3][3],const Real p0[3])
{
    for(unsigned int j=0;j<3;j++) p[j]=A[j][0]*p0[0]+A[j][1]*p0[1]+A[j][2]*p0[2];
}


template<typename Real>
void Mult(Real C[3][3],const Real A[3][3],const Real B[3][3])
{
    for(unsigned int j=0;j<3;j++) for(unsigned int k=0;k<3;k++) C[j][k]=A[j][0]*B[0][k]+A[j][1]*B[1][k]+A[j][2]*B[2][k];
}


// compute M=E.e.E^T where e is diagonal
template<typename Real>
int Jacobi(Real M[3][3],Real e[3],Real E[3][3])
{
    const int MAX_ROTATIONS=20,n=3;
    const Real EPSILON=.0001;

    int i, j, iq, ip, numPos,NB_ROTATIONS=0;
    Real tresh, theta, tau, t, sm, s, h, g, c;
    Real b[3], z[3];

    // initialize
    for (ip=0; ip<n; ip++) { for (iq=0; iq<n; iq++) E[ip][iq] = 0.0;    E[ip][ip] = 1.0;}
    for (ip=0; ip<n; ip++) { b[ip] = e[ip] = M[ip][ip];    z[ip] = 0.0;   }

    // begin rotation sequence
    for (i=0; i<MAX_ROTATIONS; i++)
    {
        sm = 0.0;
        for (ip=0; ip<n-1; ip++) for (iq=ip+1; iq<n; iq++) sm += fabs(M[ip][iq]);
        if (abs(sm) < EPSILON) {NB_ROTATIONS=i; break;}

        if (i < 3) tresh = 0.2*sm/(n*n); else tresh = 0.0;

        for (ip=0; ip<n-1; ip++)
        {
            for (iq=ip+1; iq<n; iq++)
            {
                g = 100.0*fabs(M[ip][iq]);

                if (i > 3 && (fabs(e[ip])+g) == fabs(e[ip]) && (fabs(e[iq])+g) == fabs(e[iq])) M[ip][iq] = 0.0;
                else if (fabs(M[ip][iq]) > tresh)
                {
                    h = e[iq] - e[ip];
                    if ( (fabs(h)+g) == fabs(h)) t = (M[ip][iq]) / h;
                    else { theta = 0.5*h / (M[ip][iq]);  t = 1.0 / (fabs(theta)+sqrt(1.0+theta*theta)); if (theta < 0.0) t = -t; }
                    c = 1.0 / sqrt(1+t*t); s = t*c; tau = s/(1.0+c); h = t*M[ip][iq];
                    z[ip] -= h; z[iq] += h; e[ip] -= h; e[iq] += h;
                    M[ip][iq]=0.0;

                    for (j = 0;j <= ip-1;j++)		{g=M[j][ip];h=M[j][iq];M[j][ip]=g-s*(h+g*tau); M[j][iq]=h+s*(g-h*tau);}
                    for (j = ip+1;j <= iq-1;j++)	{g=M[ip][j];h=M[j][iq];M[ip][j]=g-s*(h+g*tau); M[j][iq]=h+s*(g-h*tau);}
                    for (j=iq+1; j<n; j++)		{g=M[ip][j];h=M[iq][j];M[ip][j]=g-s*(h+g*tau); M[iq][j]=h+s*(g-h*tau);}
                    for (j=0; j<n; j++)			{g=E[j][ip];h=E[j][iq];E[j][ip]=g-s*(h+g*tau); E[j][iq]=h+s*(g-h*tau);}
                }
            }
        }

        for (ip=0; ip<n; ip++) { b[ip] += z[ip]; e[ip] = b[ip]; z[ip] = 0.0; }
    }

    // insure eigenvector consistency (i.e., Jacobi can compute vectors that are negative of one another (.707,.707,0) and (-.707,-.707,0). This can reek havoc in hyperstreamline/other stuff. We will select the most positive eigenvector.
    int ceil_half_n = (n >> 1) + (n & 1);
    for (j=0; j<n; j++)
    {
        for (numPos=0, i=0; i<n; i++) if ( E[i][j] >= 0.0 ) numPos++;
        if ( numPos < ceil_half_n) for(i=0; i<n; i++) E[i][j] *= -1.0;
    }

    return NB_ROTATIONS;
}


template<typename Real>
int ClosestRigid(Real K[3][3],Real R[3][3])
{
    Real KTK[3][3];
    KTK[0][0]=K[0][0]*K[0][0]+K[1][0]*K[1][0]+K[2][0]*K[2][0];	KTK[0][1]=K[0][0]*K[0][1]+K[1][0]*K[1][1]+K[2][0]*K[2][1];	KTK[0][2]=K[0][0]*K[0][2]+K[1][0]*K[1][2]+K[2][0]*K[2][2];
    KTK[1][0]=KTK[0][1];                                        KTK[1][1]=K[0][1]*K[0][1]+K[1][1]*K[1][1]+K[2][1]*K[2][1];	KTK[1][2]=K[0][1]*K[0][2]+K[1][1]*K[1][2]+K[2][1]*K[2][2];
    KTK[2][0]=KTK[0][2];                                        KTK[2][1]=KTK[1][2];                                        KTK[2][2]=K[0][2]*K[0][2]+K[1][2]*K[1][2]+K[2][2]*K[2][2];

    Real e[3],T[3][3];
    int nbit=Jacobi(KTK,e,T);
    for(unsigned int i=0;i<3;i++) if(e[i]>0) e[i]=1/sqrt(e[i]); else e[i]=1E10;

    Real eT[3][3] = {{e[0]*T[0][0],	e[0]*T[1][0],	e[0]*T[2][0] }  ,{e[1]*T[0][1],	e[1]*T[1][1],	e[1]*T[2][1]}, { e[2]*T[0][2],e[2]*T[1][2],e[2]*T[2][2]}};
    Real KTKm12[3][3]; Mult(KTKm12,T,eT);
    Mult(R,K,KTKm12);

    return nbit;
}

