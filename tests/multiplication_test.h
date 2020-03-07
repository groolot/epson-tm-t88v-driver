#ifndef MULTIPLICATION_TEST_H
#define MULTIPLICATION_TEST_H

#include <cppunit/extensions/HelperMacros.h>

// Déclaration de la classe de test
class multiplicationTest : public CppUnit::TestFixture
{
  // Utilisation de la Macro permettant de définir cette classe comme
  // étant une 'fixture' : le nom fourni en paramètre permet de nommer
  // le constructeur de la classe.
  CPPUNIT_TEST_SUITE(multiplicationTest);
  // Ajout des méthodes réalisant des tests
  /* TODO: ajouter les instructions pour chaque méthode de test
  */
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
  /* TODO: ajouter les instructions de déclaration de chaque méthode
   * de test
  */

private:
  // Déclaration des variables d'usage (optionnel)
  /* TODO: ajouter les déclarations des variables d'usage si besoin
   */
  // Déclaration des méthodes privées à usage interne aux tests
  // (optionnel)
};

#endif // MULTIPLICATION_TEST_H
