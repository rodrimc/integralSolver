/*
 * Authors: Felippe Nagato <fnagato@telemidia.puc-rio.br>
 *          Rodrigo Costa <rodrigocosta@telemidia.puc-rio.br>
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>

double tolerance;
double delta;
int type;

double f (double x)
{
  double value = 0.0;
  switch (type)
  {
    case 1:
      {
        value = (1 - exp(-(pow(-x,2))))/x;
        break;
      }
    case 2:
      {
        value = x * sin(x) + 10;
      }
    case 3:
      {
        value = pow (x, 2.0) + pow (x, 3.0);
      }
  }

  return value; 
}

double area (double a, double b)
{
  return (((f(a) + f(b)) / 2.0) *  (b - a)); 
}

double calcArea (double a, double b)
{
  double value = 0.0;
  double diff, largerArea, smallerArea1, smallerArea2;
  double x = (b - a)/2.0;

  largerArea = area (a, b);
  smallerArea1 = area (a, a + x);
  smallerArea2 = area (a + x, b);
 
  value = largerArea;
  diff = largerArea - (smallerArea1 + smallerArea2);
  if (fabs(diff) > tolerance && fabs(b - a) > delta)
  {
    double area1, area2;
    area1 = calcArea (a, a + x);
    area2 = calcArea (a + x, b);

    value = area1 + area2;
  }

  return value;
}


int main (int argc, char* argv[])
{
  double totalArea = 0.0, minA, maxB;
  if (argc < 4)
  {
    printf ("Available functions: \n");
    printf ("Type 1: (1 - (e^(-x^2)))/x\n");
    printf ("Type 2: (x * sin(x) + 10\n");
    printf ("Type 3: (x^2 + x^3\n");

    printf ("Usage: %s [x min] [x max] [type (1, 2 or 3)]\n", argv[0]);
    return -1;
  }

  minA = atof (argv[1]);
  maxB = atof (argv[2]);
  type = atoi (argv[3]);

  if (type < 1 || type > 3)
  {
    printf ("Unknown function type.\n");
    return -1;
  }
  
  tolerance = pow (10.0, -20);
  delta = pow(10.0, -4);

  #pragma omp parallel
  {
    double a, b, area;
    double range = maxB - minA;
    int threadId = omp_get_thread_num ();
    int numThreads = omp_get_num_threads ();

    a = threadId * (range/numThreads) + minA;
    b = a + range/numThreads;
    
    area = calcArea (a, b);
    
    #pragma omp critical
    totalArea += area;
  }

  printf ("Area (a=%.2f, b=%.2f): %f\n", minA, maxB, totalArea);

  return 0;
}
