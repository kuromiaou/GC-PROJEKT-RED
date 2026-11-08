#include "ecs/ComponentStorage.h"
#include "ecs/EntityManager.h"

#include <cstdio>

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

static int failures = 0;

static void check(bool condition, const char* description) {
    if (condition) {
        std::printf("[OK]    %s\n", description);
    }
    else {
        std::printf("[ECHEC] %s\n", description);
        failures++;
    }
}

static void testComponentStorage() {
    std::printf("\n--- ComponentStorage ---\n");
    ComponentStorage<Position> storage;

    storage.add(4, Position{ 4.0f, 40.0f });
    storage.add(1, Position{ 1.0f, 10.0f });
    storage.add(6, Position{ 6.0f, 60.0f });
    check(storage.has(4) && storage.has(1) && storage.has(6), "4, 1 et 6 ajoutees");
    check(!storage.has(0) && !storage.has(5), "0 et 5 absentes");
    check(!storage.has(1000), "1000 absente (hors de sparse)");

    storage.remove(4);
    check(!storage.has(4), "4 retiree");
    check(storage.has(6), "6 toujours presente apres le deplacement");
    check(storage.get(6).x == 6.0f && storage.get(6).y == 60.0f, "6 a garde sa valeur");
    check(storage.get(1).x == 1.0f, "1 a garde sa valeur");

    storage.remove(1);
    check(!storage.has(1), "1 retiree (elle etait en derniere position)");
    check(storage.has(6) && storage.get(6).x == 6.0f, "6 toujours correcte");

    storage.remove(42);
    check(storage.has(6), "retirer une entite absente ne casse rien");

    storage.add(6, Position{ 99.0f, 99.0f });
    check(storage.get(6).x == 6.0f, "ajouter un doublon ne remplace pas la valeur");
}

static void testEntityCreation() {
    std::printf("\n--- Creation d'entites ---\n");
    EntityManager manager;

    EntityId a = manager.createEntity();
    EntityId b = manager.createEntity();
    EntityId c = manager.createEntity();
    check(a == 0 && b == 1 && c == 2, "les IDs sont 0, 1, 2");
    check(manager.isAlive(a) && manager.isAlive(b) && manager.isAlive(c), "les trois sont vivantes");

    manager.destroyEntity(b);
    check(!manager.isAlive(b), "1 n'est plus vivante");

    EntityId d = manager.createEntity();
    check(d == 1, "l'ID 1 est reutilise");
    EntityId e = manager.createEntity();
    check(e == 3, "l'entite suivante recoit un ID neuf (3)");
}

static void testComponents() {
    std::printf("\n--- Composants ---\n");
    EntityManager manager;
    EntityId e = manager.createEntity();

    manager.addComponent(e, Position{ 1.0f, 2.0f });
    check(manager.hasComponent<Position>(e), "e a une Position");
    check(!manager.hasComponent<Velocity>(e), "e n'a pas de Velocity");
    check(manager.getComponent<Position>(e).x == 1.0f, "la valeur est la bonne");

    manager.getComponent<Position>(e).x = 5.0f;
    check(manager.getComponent<Position>(e).x == 5.0f, "modifier via la reference fonctionne");

    manager.addComponent(e, Velocity{ 3.0f, 4.0f });
    check(manager.hasComponent<Velocity>(e), "e a aussi une Velocity");

    manager.removeComponent<Position>(e);
    check(!manager.hasComponent<Position>(e), "Position retiree");
    check(manager.hasComponent<Velocity>(e), "Velocity toujours la");
}

static void testDestroyRemovesComponents() {
    std::printf("\n--- Destruction ---\n");
    EntityManager manager;
    EntityId e = manager.createEntity();
    manager.addComponent(e, Position{ 1.0f, 2.0f });
    manager.addComponent(e, Velocity{ 3.0f, 4.0f });

    manager.destroyEntity(e);
    EntityId reused = manager.createEntity();
    check(reused == e, "l'ID est reutilise");
    check(!manager.hasComponent<Position>(reused), "la nouvelle entite n'a pas l'ancienne Position");
    check(!manager.hasComponent<Velocity>(reused), "la nouvelle entite n'a pas l'ancienne Velocity");
}

int main() {
    testComponentStorage();
    testEntityCreation();
    testComponents();
    testDestroyRemovesComponents();

    std::printf("\n%s (%d echec(s))\n", failures == 0 ? "TOUT EST BON" : "IL Y A DES ERREURS", failures);
    return failures == 0 ? 0 : 1;
}