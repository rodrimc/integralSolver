/*
 * Authors: Felippe Nagato <fnagato@telemidia.puc-rio.br>
 *          Rodrigo Costa <rodrigocosta@telemidia.puc-rio.br>
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

double tolerance, delta, total_area = 0.0;
int type, working = 0;

struct _Task
{
  double a;
  double b;
  struct _Task* next;

};

typedef struct _Task Task;

typedef struct
{
  Task* first;
  Task* last;
} Bag;

Bag bag;

void insertTask (double a, double b)
{

  Task *t = (Task*) malloc (sizeof (Task));
  t->a = a;
  t->b = b;
  t->next = NULL;

  if (bag.first == NULL)
  {
    bag.first = t;
    bag.last = t;
  }
  else
  {
    bag.last->next = t;  
    bag.last = t;
  }
}

int getTask (double *a, double *b)
{
  if (bag.first == NULL)
    return -1;

  Task *t = bag.first;
  *a = t->a;
  *b = t->b;

  bag.first = t->next;
  if (bag.first == NULL)
    bag.last = NULL;

  free (t);
  return 0;
}

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
        value = x * sin(x) + 10.0;
        break;
      }
    case 3:
      {
        value = pow (x, 2.0) + pow (x, 3.0);
        break;
      }
    case 4:
      {
        value = sin(x) + 1.0;
        break;
      }
  }

  return value; 
}

double area (double a, double b)
{
  return (((f(a) + f(b)) / 2.0) *  (b - a)); 
}

void calcArea ()
{
  double a, b;

  while (1)
  {
    double value = 0.0;
    double diff, largerArea, smallerArea1, smallerArea2;
    double x;
    int flag = 0;
    
    #pragma omp critical (task)
    {
      if (getTask(&a, &b) != 0)
      {
        if (working == 0)
          flag = 1;
        else
          flag = 2;
      }
      else
        working ++;
    }

    if (flag == 1)
      break;
    else if (flag == 2)
      continue;

    x = (b - a)/2.0;
    largerArea = area (a, b);
    smallerArea1 = area (a, a + x);
    smallerArea2 = area (a + x, b);

    value = largerArea;
    diff = largerArea - (smallerArea1 + smallerArea2);
    if (fabs(diff) > tolerance && fabs(b - a) > delta)
    {
      #pragma omp critical (task)
      {
        insertTask (a, a + x);
        insertTask (a + x, b);
      }
    }
    else
    {
      #pragma omp critical (area)
      total_area += value;
    }
    
    #pragma omp critical (task)
    working --;
  }
}


int main (int argc, char* argv[])
{
  clock_t t;
  double min_a, max_b, range, width;
  int i, num_task = omp_get_max_threads();

  if (argc < 4)
  {
    printf ("Available functions: \n");
    printf ("Type 1: 1 - (e^(-x^2))/x\n");
    printf ("Type 2: x * sin(x) + 10\n");
    printf ("Type 3: x^2 + x^)\n");
    printf ("Type 4: sin (x) + 1.0\n");

    printf ("Usage: %s [x min] [x max] [type (1, 2, 3 or 4)]\n", argv[0]);
    printf ("Example: %s 2 5 4\n", argv[0]);
    return -1;
  }

  omp_set_num_threads(num_task);

  min_a = atof (argv[1]);
  max_b = atof (argv[2]);
  type = atoi (argv[3]);

  range = max_b - min_a;

  bag.first = NULL;
  bag.last = NULL;

  if (type < 1 || type > 4)
  {
    printf ("Unknown function type.\n");
    return -1;
  }
  
  tolerance = pow (10.0, -20);
  delta = pow(10.0, -5);
  
  t = clock();

  width = range / num_task;  
  for (i = 0; i < num_task; i++)
  {
    double interval_init = i * width + min_a;
    insertTask (interval_init, interval_init + width);
  }

  #pragma omp parallel
  calcArea ();
  
  t = clock() - t;

  printf ("Area (a=%.2f, b=%.2f): %f\ntime: %fs", min_a, max_b, total_area,
          (double) t/CLOCKS_PER_SEC);

  return 0;
}
