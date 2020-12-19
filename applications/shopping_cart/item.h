// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_ITEM_H
#define MOCK_KEY_VALUE_STORE_ITEM_H

#include <string>

class item {
public:
    std::string name;
    int id;
    double price;

    item(std::string name, int id, double price) {
        this->name = name;
        this->id = id;
        this->price = price;
    }

    bool operator==(const item &i2) const {
        return ((name == i2.name) && (id == i2.id) && (price == i2.price));
    }
};

item shoes("shoes", 1, 100.0);
item book("book", 2, 200.0);
item umbrella("umbrella", 3, 300.0);
item ball("ball", 4, 400.0);
item bat("bat", 5, 500.0);
item table("table", 6, 600.0);

#endif //MOCK_KEY_VALUE_STORE_ITEM_H
