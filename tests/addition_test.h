#ifndef ADDITION_TEST_H
#define ADDITION_TEST_H

#include <cppunit/extensions/HelperMacros.h>

// Déclaration de la classe de test
class additionTest : public CppUnit::TestFixture
{
  // Utilisation de la Macro permettant de définir cette classe comme
  // étant une 'fixture' : le nom fourni en paramètre permet de nommer
  // le constructeur de la classe.
  CPPUNIT_TEST_SUITE(additionTest);
  // Ajout des méthodes réalisant des tests
  CPPUNIT_TEST(addition_normal);
  CPPUNIT_TEST(addition_max);
  CPPUNIT_TEST(addition_min);
  CPPUNIT_TEST(addition_zero);
  // Utilisation de la Macro de fin de déclaration de la classe de
  // test
  CPPUNIT_TEST_SUITE_END();
 public:
  // Déclaration de la méthode d'initialisation (setUp) de la classe
  // de test (penser à une sorte de constructeur)
  void setUp();
  // Déclaration de la méthode de clôture (tearDown) de la classe de
  // test (penser à une sorte de déstructeur)
  void tearDown();
  // Déclaration des méthodes de test
  void addition_normal();
  void addition_max();
  void addition_min();
  void addition_zero();

private:
  // Déclaration des variables d'usage (optionnel)
  int operandeA;
  int operandeB;
  // Déclaration des méthodes privées à usage interne aux tests
  // (optionnel)
};

#endif // ADDITION_TEST_H
