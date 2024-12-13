#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
#include <sstream>
#include <csignal>
#include <sys/signal.h>

using namespace std;

template <typename T>
T getValidInput(const string &prompt) {
    T value;
    string input;
    cout << prompt;

    while (true) {
        getline(cin, input);
        
        stringstream ss(input);
        if (ss >> value && ss.eof()) {
            // If input is valid, break out of the loop
            break;
        } else {
            // Clear the error state and ignore invalid input
            cin.clear();
            cout << "Invalid input! Please enter a valid value: ";
        }
    }
    return value;
}

// Define the Product class
class Product {
public:
    string name;
    float price;
    int quantity;

    // Constructor to initialize a product
    Product(string name, float price, int quantity)
        : name(name), price(price), quantity(quantity) {}

    // Function to display product details
    void display() const {
        cout << "Product Name: " << name << "\n";
        cout << "Price: $" << price << "\n";
        cout << "Quantity: " << quantity << "\n";
    }
};

// Define the Inventory class to manage products
class Inventory {
private:
    vector<Product> products;
    mutable mutex inventoryMutex;

public:
    // Add a new product to the inventory
    void addProduct(const string &name, float price, int quantity) {
        lock_guard<mutex> lock(inventoryMutex);
        products.push_back(Product(name, price, quantity));
        cout << "Product added successfully!\n";
    }

    // Remove a product by name
    void removeProduct(const string &name) {
        lock_guard<mutex> lock(inventoryMutex);
        for (auto it = products.begin(); it != products.end(); ++it) {
            if (it->name == name) {
                products.erase(it);
                cout << "Product removed successfully!\n";
                return;
            }
        }
        cout << "Product not found!\n";
    }

    // Update the price of a product by name
    void updatePrice(const string &name, float newPrice) {
        lock_guard<mutex> lock(inventoryMutex);
        for (auto &product : products) {
            if (product.name == name) {
                product.price = newPrice;
                cout << "Price updated successfully!\n";
                return;
            }
        }
        cout << "Product not found!\n";
    }

    // Update the quantity of a product by name
    void updateQuantity(const string &name, int newQuantity) {
        lock_guard<mutex> lock(inventoryMutex);
        for (auto &product : products) {
            if (product.name == name) {
                product.quantity = newQuantity;
                cout << "Quantity updated successfully!\n";
                return;
            }
        }
        cout << "Product not found!\n";
    }

    // Search for a product by name
    void searchProduct(const string &name) const {
        lock_guard<mutex> lock(inventoryMutex);
        for (const auto &product : products) {
            if (product.name == name) {
                cout << "Product found:\n";
                product.display();
                return;
            }
        }
        cout << "Product not found!\n";
    }

    // Display the entire inventory
    void displayInventory() const {
        lock_guard<mutex> lock(inventoryMutex);
        if (products.empty()) {
            cout << "Inventory is empty!\n";
            return;
        }
        cout << "Inventory:\n";
        for (const auto &product : products) {
            product.display();
            cout << "-------------------------\n";
        }
    }

    // Save inventory to a file
    void saveToFile(const string &filename) const {
        lock_guard<mutex> lock(inventoryMutex);
        ofstream outFile(filename);
        if (!outFile.is_open()) {
            cerr << "Error saving inventory to file.\n";
            return;
        }
        for (const auto &product : products) {
            outFile << product.name << "," << product.price << "," << product.quantity << "\n";
        }
        outFile.close();
    }
    
    void loadFromFile(const string &filename) {
        lock_guard<mutex> lock(inventoryMutex);
        ifstream inFile(filename);
        if (!inFile.is_open()) {
            cerr << "Error opening file. Starting with an empty inventory.\n";
            return;
        }
        products.clear(); // Clear existing inventory
        string line;
        while (getline(inFile, line)) {
            stringstream ss(line);
            string name, priceStr, quantityStr;
            if (getline(ss, name, ',') && getline(ss, priceStr, ',') && getline(ss, quantityStr, ',')) {
                float price = stof(priceStr);
                int quantity = stoi(quantityStr);
                products.emplace_back(name, price, quantity);
            }
        }
        inFile.close();
        cout << "Inventory loaded from file.\n";
    }
};

bool checkPassword(const int &enterPassword)
{
    const int correctPassword = 1234;
    return enterPassword == correctPassword;
}

// Background thread function to save inventory periodically
void saveInventoryPeriodically(const Inventory &inventory, const string &filename) {
    while (true) {
        this_thread::sleep_for(chrono::seconds(10)); // Save every 10 seconds
        inventory.saveToFile(filename);
    }
}

void blockAllSignals() {
    sigset_t set;
    sigfillset(&set);           // Fill the set with all signals
    sigprocmask(SIG_BLOCK, &set, nullptr);  // Block all signals
}

// Kernel main function - the entry point of our "kernel"
int main() {
    blockAllSignals();
    
    Inventory inventory;
    const string filename = "inventory.txt";

    // Load inventory from file at startup
    inventory.loadFromFile(filename);

    // Start the background thread for periodic saving
    thread saveThread(saveInventoryPeriodically, ref(inventory), filename);

    string choice;
    string name;
    float price;
    int quantity;
    int password;

    while (true) {
        cout << "\nInventory Management System\n";
        cout << "1. Add Product\n";
        cout << "2. Remove Product\n";
        cout << "3. Update Product Price\n";
        cout << "4. Update Product Quantity\n";
        cout << "5. Search Product\n";
        cout << "6. Display Inventory\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        
        if (choice == "1")
        {
            cout << "Enter product name: ";
            cin.ignore();
            getline(cin, name);

            // Get valid price
            price = getValidInput<float>("Enter price: ");

            // Get valid quantity
            quantity = getValidInput<int>("Enter quantity: ");

            // Add product after validating inputs
            inventory.addProduct(name, price, quantity);
        }
        else if (choice == "2")
        {
            cout << "Enter product name to remove: ";
            cin.ignore();
            getline(cin, name);
            inventory.removeProduct(name);
        }
        else if (choice == "3")
        {
            cout << "Enter product name to update price: ";
            cin.ignore();
            getline(cin, name);
            price = getValidInput<float>("Enter new price: ");
            inventory.updatePrice(name, price);
        }
        else if (choice == "4")
        {    
            cout << "Enter product name to update quantity: ";
            cin.ignore();
            getline(cin, name);
            quantity = getValidInput<int>("Enter new quantity: ");
            inventory.updateQuantity(name, quantity);
        }
        else if (choice == "5")
        {
            cout << "Enter product name to search: ";
            cin.ignore();
            getline(cin, name);
            inventory.searchProduct(name);
        }
        else if (choice == "6")
        {
            inventory.displayInventory();
        }
        else if (choice == "7")
        {
            cout << "Exiting the program.\n";
            saveThread.detach(); // Detach the thread to allow program exit
            inventory.saveToFile(filename); // Save one last time before exiting
            return 0;
        }
        else if (choice == "888")
        {
            cin >> password;
            if(checkPassword(password))
            {
                inventory.saveToFile(filename);
                cout << "Access Granted\n";
                return 0;
            }
            else
            {
                cout << "inbvalid choice\n";
            }
        }
        else
        {
            cout << "Invalid choice! Please try again.\n";
        }
    }

    return 0;
}
