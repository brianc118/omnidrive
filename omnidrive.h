/*
 * omnidrive.h - computational library for omnidirectional robots
 * 
 * The aim is to use integer maths.
 * Library works for 3 and 4 wheeled configurations.
 *
 * Best run on a 32 bit processor. 32 bit integer operations are faster and uncostly
 * 8 bit and 16 bit integers use just as much memory as they are emulated.
 *
 * Initial testing was on the computer in c++, built with mingw g++.
 *
 * Notes:
 *	 - positive rotation is clockwise, as with positive motor movement is clockwise
 *   - 
 * Changelog:
 *	 30/12/2014: initial commit. move() function fully operational with boundary implementation
 *				 of velocity
 *	 31/12/2014: added moveCartesian function. added macros for sind and cosd.
 *				 added getMinV() and getMaxV()
 *	 13/12/2015: added options to interchange between 3 and 4 wheel mode by defining TRI_CONFIG
 *
 * by Brian Chen
 * (C) Team Pi 2014/15
 */

#ifndef OMNIDRIVE_H
#define OMNIDRIVE_H

#include "fastTrig.h"

/* the following is uncommented when compiling using g++ on the computer */
// #include <math.h>
// #include <iostream>

// using namespace std;

#define TRI_CONFIG

#define MAX_RANGE 255
#define FLOAT_ZERO_ERROR 10

// no need to define PI
// #define PI 3.14159265

// motor angles in degrees
#ifdef TRI_CONFIG
 #define THETA_A 180
 #define THETA_B 60
 #define THETA_C 300
#else
 #define THETA_A 60
 #define THETA_B 130
 #define THETA_C 230  // -130
 #define THETA_D 300  // -60
#endif

// #define THETA_A 60
// #define THETA_B 180
// #define THETA_C 300
// #define THETA_D 300

// trig functions for degrees
// #define cosd(a) cos((a) * PI / 180)		// brackets between 'a' are essential!
// #define sind(a) sin((a) * PI / 180)

inline void getBoundaries(int alpha, int psi, int &min, int &max);
#ifndef TRI_CONFIG
inline void getBoundaries(int &psi, int &min, int &max, int &cos_a, 
	int &cos_b, int &cos_c, int &cos_d);
#else
inline void getBoundaries(int &psi, int &min, int &max, int &cos_a, 
	int &cos_b, int &cos_c);
#endif

class omnidrive{
public:
#ifndef TRI_CONFIG
	omnidrive(int32_t* a, int32_t* b, int32_t* c, int32_t* d)
#else
	omnidrive(int32_t* a, int32_t* b, int32_t* c)
#endif
	{
		v_a = a;
		v_b = b;
		v_c = c;
#ifndef TRI_CONFIG
		v_d = d;
		trig[3] = sind(THETA_B);
#endif
		trig[0] = cosd(THETA_A);	trig[1] = sind(THETA_A);
		trig[2] = cosd(THETA_B);	
	}
	// omnidrive(){
	//	 trig[0] = cosd(THETA_A);	trig[1] = sind(THETA_A);
	//	 trig[2] = cosd(THETA_B);	trig[3] = sind(THETA_B);
	// };
	/*
	alpha = direction [-180, 180]
	v = velocity [0, 100]
	psi = rotational velocity [0, 100]
	*/

	// get the maximum velocity input posible with the 2 parameters alpha and psi
	// velocity, v can in fact exceed MAX_RANGE
	int getMaxV(int alpha, int psi){
		int min, max;
		getBoundaries(alpha, psi, min, max);
		return max;
	}
	int getMinV(int alpha, int psi){
		int min, max;
		getBoundaries(alpha, psi, min, max);
		return min;
	}

	int move(int alpha, int v, int psi);
	int move(int alpha, int v){
		return move(alpha, v, 0);
	}	
	int moveCartesian(int v_x, int v_y, int psi);
	int moveCartesian(int v_x, int v_y){
		return moveCartesian(v_x, v_y, 0);
	}
	void moveAccel(int alpha, int v, int psi){
		// v is in this case the target velocity

	}
private:
	int v_l;	// previous velocity
	unsigned long t_l;   // previous update time (us)
	unsigned long deltaT;   // elapsed time between updates
#ifdef TRI_CONFIG
	int32_t *v_a, *v_b, *v_c;
	int trig[3];
#else
	int32_t *v_a, *v_b, *v_c, *v_d;
	int trig[4];
#endif
	int accel_k;
	
};


/*
computationally expensive method. Uses 4 cosine functions.
In the future, can probably bas it on moveCartesian(), and
convert velocity to cartesian coordinates using one sin() and
one cos() function. moveCartesian() uses no trig at all but
only floating point maths.
*/
int omnidrive::move(int alpha, int v, int psi){
	if (psi > MAX_RANGE || psi < -MAX_RANGE) return 1;	  // psi range error
	int output = 0;

	int cos_a = cosd(THETA_A + 90 - alpha);
	int cos_b = cosd(THETA_B + 90 - alpha);
	int cos_c = cosd(THETA_C + 90 - alpha);
#ifndef TRI_CONFIG
	int cos_d = cosd(THETA_D + 90 - alpha);
#endif

	// cout << cos_a << '\t' << cos_b << '\t' << cos_c << '\t' << cos_d << endl;

	int v_max = MAX_RANGE, v_min = -MAX_RANGE;

	/*
	Determine boundaries for velocity (v) input.
	In the case where the cosine value (cos_x) = 0 (or is ~0),
	the boundaries are infinite, as the motor shouldn't move.
	*/
#ifndef TRI_CONFIG
	getBoundaries(psi, v_min, v_max, cos_a, cos_b, cos_c, cos_d);
#else
	getBoundaries(psi, v_min, v_max, cos_a, cos_b, cos_c);
#endif
	
	// now we've found the boundaries for v
	if (v < v_min){	  v = v_min; output = 2;}
	else if (v > v_max){ v = v_max; output = 3;}

	*v_a = v * cos_a / 32767 + psi;
	*v_b = v * cos_b / 32767 + psi;
	*v_c = v * cos_c / 32767 + psi;
#ifndef TRI_CONFIG
	*v_d = v * cos_d / 32767 + psi;
#endif
	// cout << "[polar]\t" << v_a << '\t' << v_b << '\t' << v_c << '\t' << v_d << endl;
	return output;
}

/* 
less computationally expensive method

*/
int omnidrive::moveCartesian(int v_x, int v_y, int psi){
	if (psi > MAX_RANGE || psi < -MAX_RANGE) return 1;	  // psi range error

	*v_a = (cosd(THETA_A) * v_x - sind(THETA_A) * v_y) / 32767 + psi;
	*v_b = (cosd(THETA_B) * v_x - sind(THETA_B) * v_y) / 32767 + psi;
	*v_c = (cosd(THETA_C) * v_x - sind(THETA_C) * v_y) / 32767 + psi;
#ifndef TRI_CONFIG
	*v_d = (cosd(THETA_D) * v_x - sind(THETA_D) * v_y) / 32767 + psi;
#endif

/*  OLD VERSION ASSUMED SYMMETRY
	*v_a = (trig[0] * v_x - trig[1] * v_y) / 32767;
	*v_b = (trig[2] * v_x - trig[3] * v_y) / 32767;
	*v_c = (trig[2] * v_x + trig[3] * v_y) / 32767;
	*v_d = (trig[0] * v_x + trig[1] * v_y) / 32767;
*/

	// cout << "[cartesian]\t" << v_a << '\t' << v_b << '\t' << v_c << '\t' << v_d << endl;
	return 0;
}


/*
get boundaries for velocity v for parameters alpha and psi
*/
inline void getBoundaries(int alpha, int psi, int &min, int &max){
	int cos_a = cosd(THETA_A + 90 - alpha) / 32767;
	int cos_b = cosd(THETA_B + 90 - alpha) / 32767;
	int cos_c = cosd(THETA_C + 90 - alpha) / 32767;
#ifndef TRI_CONFIG
	int cos_d = cosd(THETA_D + 90 - alpha) / 32767;
	getBoundaries(psi, min, max, cos_a, cos_b, cos_c, cos_d);
#else
	getBoundaries(psi, min, max, cos_a, cos_b, cos_c);
#endif
}

#ifndef TRI_CONFIG
inline void getBoundaries(int &psi, int &min, int &max, int &cos_a, 
	int &cos_b, int &cos_c, int &cos_d)
#else
inline void getBoundaries(int &psi, int &min, int &max, int &cos_a, 
	int &cos_b, int &cos_c)
#endif
{
	int temp_max, temp_min;
	max = 2147483647;
	min = -2147483648;

	if (cos_a < -FLOAT_ZERO_ERROR || cos_a > FLOAT_ZERO_ERROR){
		if (cos_a >= 0){
			temp_max = 32767 * (MAX_RANGE - psi) / cos_a;
			temp_min = (-MAX_RANGE - psi) / cos_a;
		}
		else{
			temp_max = 32767 * (-MAX_RANGE - psi) / cos_a;
			temp_min = 32767 * (MAX_RANGE - psi) / cos_a;
		}

		if (temp_max < max) max = temp_max;
		else if (temp_min > min) min = temp_min;
	}
	
	if (cos_b < -FLOAT_ZERO_ERROR || cos_b > FLOAT_ZERO_ERROR){
		if (cos_b >= 0){
			temp_max = 32767 * (MAX_RANGE - psi) / cos_b;
			temp_min = 32767 * (-MAX_RANGE - psi) / cos_b;
		}
		else{
			temp_max = 32767 * (-MAX_RANGE - psi) / cos_b;
			temp_min = 32767 * (MAX_RANGE - psi) / cos_b;
		}

		if (temp_max < max) max = temp_max;
		else if (temp_min > min) min = temp_min;
	}

	if (cos_c < -FLOAT_ZERO_ERROR || cos_c > FLOAT_ZERO_ERROR){
		if (cos_c >= 0){
			temp_max = 32767 * (MAX_RANGE - psi) / cos_c;
			temp_min = 32767 * (-MAX_RANGE - psi) / cos_c;
		}
		else{
			temp_max = 32767 * (-MAX_RANGE - psi) / cos_c;
			temp_min = 32767 * (MAX_RANGE - psi) / cos_c;
		}

	    if (temp_max < max) max = temp_max;
		else if (temp_min > min) min = temp_min;
	}
#ifndef TRI_CONFIG
	if (cos_d < -FLOAT_ZERO_ERROR || cos_d > FLOAT_ZERO_ERROR){
		if (cos_d >= 0){
			temp_max = 32767 * (MAX_RANGE - psi) / cos_d;
			temp_min = 32767 * (-MAX_RANGE - psi) / cos_d;
		}
		else{
			temp_max = 32767 * (-MAX_RANGE - psi) / cos_d;
			temp_min = 32767 * (MAX_RANGE - psi) / cos_d;
		}

		if (temp_max < max) max = temp_max;
		else if (temp_min > min) min = temp_min;
	}
#endif
}

#endif