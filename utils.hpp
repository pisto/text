#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include <cstddef>
#include <type_traits>

//types
#ifndef REAL_TYPE
#define REAL_TYPE double
#endif
typedef REAL_TYPE real;			//type to use for physic values

#ifndef INTEGER_TYPE
#define INTEGER_TYPE int
#endif
typedef std::make_signed<INTEGER_TYPE>::type	intg;
typedef std::make_unsigned<INTEGER_TYPE>::type	u_intg;
typedef std::make_signed<size_t>::type			s_size_t;
typedef unsigned char uchar;

#define PI 3.14159265358979323846L

//looping
#define loopa(v, m)		for(size_t v=0, m__=size_t(m); v<m__; v++)
#define loopai(m)		loopa(i, m)
#define loopaj(m)		loopa(j, m)
#define loopak(m)		loopa(k, m)
#define loop(v, m)		for(u_intg v=0, m__=u_intg(m); v<m__; v++)
#define loopi(m)		loop(i, m)
#define loopj(m)		loop(j, m)
#define loopk(m)		loop(k, m)

#define loopar(v, m)	for(size_t v=m; v; v--)
#define loopari(m)		loopar(i, m)
#define looparj(m)		loopar(j, m)
#define loopark(m)		loopar(k, m)
#define loopr(v, m)		for(u_intg v=m; v; v--)
#define loopri(m)		loopr(i, m)
#define looprj(m)		loopr(j, m)
#define looprk(m)		loopr(k, m)

unsigned int random(unsigned int modulo);

#endif /* RANDOM_HPP_ */
