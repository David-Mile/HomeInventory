// TestFirebase.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <QCoreApplication>
#include <QDebug>
#include "FirebaseDatabaseManager.h"
#include "HomeObject.h"

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
    QString apiKey = "AIzaSyAB1LW2A7wSOGMKIzlyHd7MYjKsnuwy88w";

    FirebaseDatabaseManager db;

    // Firebase Configuration
    std::cout << u8"\n Configuring Firebase\n";
    db.connect(firebaseUrl);
    db.setApiKey(apiKey);

    // Prova auto-login con credenziali salvate
    std::cout << u8"\n🔐 Attempting auto-login...\n";
    bool autoLoginSuccess = db.tryAutoLogin();

    // Se auto-login fallisce, chiedi credenziali manualmente
    if (!autoLoginSuccess) {
        std::cout << u8"\n📝 Manual login required\n";
        // USA QTextStream invece di std::cin
        QTextStream cin(stdin);
        QString email, password;

        std::cout << u8"Enter your email: ";
        std::cout.flush();
        email = cin.readLine().trimmed();

        std::cout << u8"Enter your password: ";
        std::cout.flush();
        password = cin.readLine().trimmed();

        std::cout << u8"Remember credentials for next time? (y/n): ";
        std::cout.flush();
        QString rememberInput = cin.readLine().trimmed().toLower();
        bool rememberMe = (rememberInput == "y" || rememberInput == "yes");

        // Autentica con email e password
        bool loginSuccess = db.authenticateWithEmail(email, password, rememberMe);

        if (loginSuccess) {
            // TEST: Verifica subito dopo il login
            std::cout << "\n🧪 Testing immediate request after login...\n";
            QStringList colors = db.getColors();
            if (colors.isEmpty()) {
                std::cout << "❌ Request failed even after fresh login!\n";
                std::cout << "Error: ";
                qDebug() << db.lastError();
            }
            else {
                std::cout << "✅ Request succeeded!\n";
                qDebug() << "Colors:" << colors;
            }
        }

        if (!loginSuccess) {
            std::cout << u8"\n❌ Login failed: ";
            qDebug() << db.lastError();
            std::cout << u8"\nCannot proceed without authentication. Exiting.\n";
            return 1;
        }
    }

    if (!db.isAuthenticated()) {
        std::cout << u8"\n❌ Not authenticated. Exiting.\n";
        return 1;
    }

    //Esegui i test

    // Test Attributes (Colors, Materials, Types)
    std::cout << u8"📚 Test Attributes\n";
    testAttributes(db);

    // Test Objects (CRUD)
    std::cout << u8"📦 Test Objects CRUD\n";
    testObjects(db);

    // Step 5: Chiedi se fare logout e cancellare le credenziali
    std::cout << u8"👋 STEP 6: Logout options\n";
    std::cout << u8"Do you want to clear saved credentials? (y/n): ";
    std::string clearInput;
    std::getline(std::cin, clearInput);
    bool clearCredentials = (clearInput == "y" || clearInput == "Y");

    db.logout(clearCredentials);

    qDebug() << "\n=================================";
    std::cout << u8"✅ All tests completed!\n";
    qDebug() << "=================================\n";

    return 0;
}

void testAttributes(FirebaseDatabaseManager& db)
{
    // Test Colors
    std::cout << u8"\n🎨 Testing Colors...\n";
    QStringList colors = db.getColors();
    qDebug() << "Colors in database:" << colors;

    // Test Materials
    std::cout << u8"\n🔨 Testing Materials...\n";
    QStringList materials = db.getMaterials();
    qDebug() << "Materials in database:" << materials;

    // Test Types
    std::cout << u8"\n📋 Testing Types...\n";
    QStringList types = db.getTypes();
    qDebug() << "Types in database:" << types;

    // Test Add Color
    std::cout << u8"\n➕ Testing Add Color...\n";
    bool added = db.addColor("Arancione");
    if (added) {
        std::cout << u8"✅ Color 'Arancione' added!\n";
        QStringList updatedColors = db.getColors();
        qDebug() << "Updated colors:" << updatedColors;
    }
    else {
        std::cout << u8"⚠️ Could not add color: ";
        qDebug() << db.lastError();
    }
}

void testObjects(FirebaseDatabaseManager& db)
{
    // Test Create Object
    std::cout << "\n➕ Testing Create Object...\n";

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
        std::cout << u8"✅ Object created successfully!\n";
    }
    else {
        std::cout << u8"❌ Failed to create object:";
        qDebug() << db.lastError();
    }

    // Test Get All Objects
    std::cout << u8"\n📥 Testing Get All Objects...\n";
    QList<HomeObject> allObjects = db.getAllObjects();
    qDebug() << "Total objects in database:" << allObjects.size();

    for (const HomeObject& obj : allObjects) {
        qDebug() << "  -" << obj.name() << "|" << obj.color() << "|" << obj.material();
    }

    // Test Update Object
    std::cout << u8"\n✏️ Testing Update Object...\n";
    testObj.setNotes("Note aggiornate!");
    testObj.setColor("Blu");

    bool updated = db.updateObject("Libro di Test", testObj);
    if (updated) {
        std::cout << u8"✅ Object updated successfully!\n";
    }
    else {
        std::cout << u8"❌ Failed to update object:\n";
        qDebug() << db.lastError();
    }

    // Test Search Objects
    std::cout << u8"\n🔍 Testing Search Objects...\n";
    QVariantMap filters;
    filters["colors"] = QStringList{ "Blu" };

    QList<HomeObject> searchResults = db.searchObjects(filters);
    qDebug() << "Objects with color 'Blu':" << searchResults.size();

    // Test Delete Object
    std::cout << u8"\n🗑️ Testing Delete Object...\n";
    bool deleted = db.deleteObject("Libro di Test");
    if (deleted) {
        std::cout << u8"✅ Object deleted successfully!\n";
    }
    else {
        std::cout << u8"❌ Failed to delete object: ";
        qDebug() << db.lastError();
    }

    // Verify deletion
    QList<HomeObject> afterDelete = db.getAllObjects();
    qDebug() << "Objects after deletion: " << afterDelete.size();
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
