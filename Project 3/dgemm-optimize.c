#include <x86intrin.h>

//collaborators: An Pham, Franz Anthony Varela, Jonathan Srinivasan

//unrolling
/*
void dgemm(int m, int n, float *A, float *C) {
    int i,j,k,l;

    for(i=0; i < m; i++)
	{
        for(k=0; k < n-3; k += 4)
		{
            for(j=0; j < m; j++)
			{

                C[i+j*m] += A[i+k*m] * A[j+k*m];
                C[i+j*m] += A[i+(k+1)*m] * A[j+(k+1)*m];
                C[i+j*m] += A[i+(k+2)*m] * A[j+(k+2)*m];
                C[i+j*m] += A[i+(k+3)*m] * A[j+(k+3)*m];
            }
        }

		for(l=k; l<n; l++)
		{
			for (j=0; j < m; j++)
			{
            C[i+j*m] += A[i+l*m] * A[j+l*m];
			}
		}
    }
}
*/

//Register Blocking
/*
void dgemm(int m, int n, float *A, float *C) {
    float x = 0;

	for(int i = 0; i < m; i ++){
	    for(int k = 0; k < n; k++){
          x = A[i+k*m];

	  	    for(int j = 0; j<m;j ++)
	  		    C[j+i*m]+= A[j+k*m]*x;

	  }
	}
}
*/

//SSE
//we can vectorize these operations using this library
//the methods usually act upon 4 floats a time
//so need to come up with a scheme that can exploit this
//however, also need to account for the left over rows and columns we may not account for
//basically, grab the appropriate elements in groups of 4, dot product, and add them to the proper elements in C as you go along

void dgemm( int m, int n, float *A, float *C ){
  int i, j, k;
  const int m_4 = m/4, m_rem = m%4; // determine how many we will need to account for
  similar scheme to naive implementation
  for (i = 0; i < m; i++) { // for EVERY row
    for (j = 0; j < n; j++) { // for EVERY column
      __m128 b = _mm_set_ps1(A[i+j*m]); // i'm just copying the same value 4 times
      for (k = 0; k < m_4*4; k+=4) { // go through the rows in groups of 4
        __m128 a = _mm_loadu_ps(&A[k+j*m]); // grabs the 4 elements in one column, but different rows
        __m128 r = _mm_mul_ps(a, b); // dot product
        __m128 c = _mm_loadu_ps(&C[k+i*m]); // get the current C element (along the column, different rows)
        c = _mm_add_ps(c, r); // +=
        _mm_storeu_ps(&C[k+i*m], c); // store it back
      }
      for(k = m_4*4; k < m_4*4+m_rem;k++){ // go through the row elements we haven't seen yet (still on the i'th iteration)
        C[k+i*m]+=A[k+j*m]*A[i+j*m];
      }
    }
  }
}
