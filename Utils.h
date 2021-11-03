#ifndef UTILS_H
#define UTILS_H
#include <occi.h>

int mainMenu();
int customerLogin(oracle::occi::Connection* conn, int customerId);
int addToCart(oracle::occi::Connection* conn, struct ShoppingCart cart[]);
double findProduct(oracle::occi::Connection* conn, int product_id);
void displayProducts(struct ShoppingCart cart[], int productCount);
int checkout(oracle::occi::Connection* conn, struct ShoppingCart cart[], int customerId, int productCount);

#endif // !UTILS_H

