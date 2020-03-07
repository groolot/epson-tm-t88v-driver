#include "arithmetique.h"

long int arithmetique::addition(int operandeA, int operandeB)
{
  return static_cast<long int>(operandeA) + operandeB;
}

int arithmetique::soustraction(int operandeA, int operandeB)
{
  return operandeA - operandeB;
}

int arithmetique::multiplication(int operandeA, int operandeB)
{
  return operandeA * operandeB;
}

int arithmetique::division(int dividende, int diviseur)
{
  return dividende / diviseur;
}
