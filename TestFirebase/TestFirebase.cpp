// TestFirebase.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <QCoreApplication>
#include <QDebug>
#include "FirebaseDatabaseManager.h"
#include "HomeObject.h"

void testConnection(FirebaseDatabaseManager& db, const QString& url);
void testAttributes(FirebaseDatabaseManager& db);
void testObjects(FirebaseDatabaseManager& db);

int main(int argc, char* argv[])
{
    // Set Utf8 to handle emoji
    SetConsoleOutputCP(CP_UTF8);

    QCoreApplication app(argc, argv);

    qDebug() << "=================================";
    std::cout << u8"🔥 Firebase Test Application\n";
    qDebug() << "=================================\n";

    // Personal URL Firebase!
    QString firebaseUrl = "https://homeinventory-b858b-default-rtdb.europe-west1.firebasedatabase.app/";

    FirebaseDatabaseManager db;

    // Test 1: Connection
    std::cout << u8"\n📡 TEST 1: Connection\n";
    qDebug() << "Firebase URL:" << firebaseUrl;
    testConnection(db, firebaseUrl);

    if (!db.isConnected()) {
        std::cout << u8"\n❌ Cannot proceed without connection. Exiting.\n";
        return 1;
    }

    // Test 2: Attributes (Colors, Materials, Types)
    std::cout << u8"\n📚 TEST 2: Attributes\n";
    testAttributes(db);

    // Test 3: Objects (CRUD)
    std::cout << u8"\n📦 TEST 3: Objects CRUD\n";
    testObjects(db);

    qDebug() << "\n=================================";
    std::cout << u8"✅ All tests completed!\n";
    qDebug() << "=================================\n";

    return 0;
}

void testConnection(FirebaseDatabaseManager& db, const QString& url)
{
    bool connected = db.connect(url);

    if (connected) {
        std::cout << u8"✅ Connected successfully!\n";
    }
    else {
        std::cout << u8"❌ Connection failed!\n";
        qDebug() << "Error:" << db.lastError();
    }
}

void testAttributes(FirebaseDatabaseManager& db)
{
    // Test Colors
    qDebug() << "\n🎨 Testing Colors...";
    QStringList colors = db.getColors();
    qDebug() << "Colors in database:" << colors;

    // Test Materials
    qDebug() << "\n🔨 Testing Materials...";
    QStringList materials = db.getMaterials();
    qDebug() << "Materials in database:" << materials;

    // Test Types
    qDebug() << "\n📋 Testing Types...";
    QStringList types = db.getTypes();
    qDebug() << "Types in database:" << types;

    // Test Add Color
    qDebug() << "\n➕ Testing Add Color...";
    bool added = db.addColor("Arancione");
    if (added) {
        qDebug() << "✅ Color 'Arancione' added!";
        QStringList updatedColors = db.getColors();
        qDebug() << "Updated colors:" << updatedColors;
    }
    else {
        qDebug() << "⚠️ Could not add color:" << db.lastError();
    }
}

void testObjects(FirebaseDatabaseManager& db)
{
    // Test Create Object
    qDebug() << "\n➕ Testing Create Object...";

    HomeObject testObj;
    testObj.setName("Libro di Test");
    testObj.setColor("Rosso");
    testObj.setMaterial("Carta");
    testObj.setType("Libro");
    testObj.setLocationId(1);
    testObj.setSublocationId(1);
    testObj.setNotes("Questo è un oggetto di test");

    bool created = db.createObject(testObj);
    if (created) {
        qDebug() << "✅ Object created successfully!";
    }
    else {
        qDebug() << "❌ Failed to create object:" << db.lastError();
    }

    // Test Get All Objects
    qDebug() << "\n📥 Testing Get All Objects...";
    QList<HomeObject> allObjects = db.getAllObjects();
    qDebug() << "Total objects in database:" << allObjects.size();

    for (const HomeObject& obj : allObjects) {
        qDebug() << "  -" << obj.name() << "|" << obj.color() << "|" << obj.material();
    }

    // Test Update Object
    qDebug() << "\n✏️ Testing Update Object...";
    testObj.setNotes("Note aggiornate!");
    testObj.setColor("Blu");

    bool updated = db.updateObject("Libro di Test", testObj);
    if (updated) {
        qDebug() << "✅ Object updated successfully!";
    }
    else {
        qDebug() << "❌ Failed to update object:" << db.lastError();
    }

    // Test Search Objects
    qDebug() << "\n🔍 Testing Search Objects...";
    QVariantMap filters;
    filters["colors"] = QStringList{ "Blu" };

    QList<HomeObject> searchResults = db.searchObjects(filters);
    qDebug() << "Objects with color 'Blu':" << searchResults.size();

    // Test Delete Object
    qDebug() << "\n🗑️ Testing Delete Object...";
    bool deleted = db.deleteObject("Libro di Test");
    if (deleted) {
        qDebug() << "✅ Object deleted successfully!";
    }
    else {
        qDebug() << "❌ Failed to delete object:" << db.lastError();
    }

    // Verify deletion
    QList<HomeObject> afterDelete = db.getAllObjects();
    qDebug() << "Objects after deletion:" << afterDelete.size();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
