#include <iostream>
#include <iomanip>
#include "../Utils.h"
#include "../ShoppingCart.h"
//Maximum number of allowed products in cart
const unsigned int CART_SIZE = 10;

int customerLogin(oracle::occi::Connection* conn, int customerId)
{
	try
	{//Checks if passed customer ID argument matches a customer id in the data base if so return 1, else 0
		oracle::occi::Statement* statement = conn->createStatement();
		statement->setSQL("BEGIN find_customer(:1, :2); END;");
		statement->setNumber(1, customerId);
		statement->registerOutParam(2, oracle::occi::Type::OCCIINT);
		statement->execute();
		customerId = statement->getInt(2);
		return customerId;
	}
	catch (oracle::occi::SQLException& sqlExcp)
	{//If above fails, catch exception, print ORA code and return 0
		std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
		return 0;
	}
}

int mainMenu()
{
	int value;
	bool needNum = true;
	std::cout << "*************Welcome!********************" << std::endl;
	std::cout << "1)      Login" << std::endl;
	std::cout << "0)      Exit" << std::endl;
	std::cout << "Enter an option(0 - 1): ";
	while (needNum)
	{
		std::cin >> value;
		std::cin.ignore();
		if (value != 1 && value != 0)
		{
			std::cout << "\n*************Welcome!********************" << std::endl;
			std::cout << "1)      Login" << std::endl;
			std::cout << "0)      Exit" << std::endl;
			std::cout << "You entered a wrong value. Please enter an option (0-1): ";
		}
		else
		{
			needNum = false;
		}

	}
	return value;
}

void displayProducts(struct ShoppingCart cart[], int productCount)
{
	double total = 0;
	std::cout << "------- Ordered Products -----------------------------" << std::endl;
	//loop through and display all products in cart, also add up total for display after
	for (int i = 0; i < productCount; i++)
	{
		std::cout << "---Item " << i + 1 << "  Product ID: " << cart[i].product_id << "  Price: $" << cart[i].price << "  Quantity: " << cart[i].quantity << std::endl;
		total += (cart[i].price * cart[i].quantity);
	}
	//print total
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << std::fixed;
	std::cout << std::setprecision(2);
	std::cout << "Total: $" << total;
	std::cout << std::endl << "=======================================================" << std::endl;
}

int addToCart(oracle::occi::Connection* conn, ShoppingCart cart[])
{
	std::cout << "\n-------------- Add Products to Cart --------------";
	int productsInCart = 0;
	//Add desired amount of items to card (Cart size enforced)
	for (int i = 0; i < CART_SIZE; i++)
	{
		int id = 0;
		int done;
		int qty = 0;

		do
		{
			done = 0;
			std::cout << "\nEnter the product ID: ";
			std::cin >> id;
			std::cin.ignore();
			//if product is found, display price, add to cart else: error and re-rentry
			if (findProduct(conn, id))
			{
				done = 1;
				std::cout << "Product Price: " << findProduct(conn, id) << std::endl;
				std::cout << "Enter the product Quantity: ";
				std::cin >> qty;
				std::cin.ignore(); // Added
				++productsInCart;
				//Add item details
				cart[i].product_id = id;
				cart[i].price = findProduct(conn, id);
				cart[i].quantity = qty;
			}
			else
				std::cout << "\nThe product does not exist. Please try again...\n";
		} while (!done);
		int cont;
		//If space in cart, ask user if they're done or want to continue adding items, 
		//else finish function and return number of products added
		if (productsInCart < CART_SIZE)
		{
			do
			{
				std::cout << "Enter 1 to add more products or 0 to checkout: ";
				std::cin >> cont;
				std::cin.ignore();
				if (cont == 0)
				{
					return productsInCart;
				}
			} while (cont != 0 && cont != 1);

		}
	}
	return productsInCart;
}

double findProduct(oracle::occi::Connection* conn, int product_id)
{//See if passed product id matches a product in the products table, if so return price of product else, 0
	try
	{
		oracle::occi::Statement* statement = conn->createStatement();
		statement->setSQL("BEGIN find_product(:1, :2); END;");
		statement->setNumber(1, product_id);
		statement->registerOutParam(2, oracle::occi::Type::OCCIINT);
		statement->execute();

		return statement->getInt(2);
	}//If above process fails, display ORA error code and return 0
	catch (oracle::occi::SQLException& sqlExcp)
	{
		std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
		return 0;
	}
}

int checkout(oracle::occi::Connection* conn, struct ShoppingCart cart[], int customerId, int productCount)
{
	int ret = 0;
	std::cout << "Would you like to checkout? (Y/y or N/n): ";
	int getChoice = 1;
	//This loop gets information from user to see if they want to check out or cancel order
	do
	{
		char choice;
		choice = getchar();
		std::cin.ignore();
		if (choice == 'Y' || choice == 'y')
		{
			getChoice = 0;
		}
		else if (choice == 'N' || choice == 'n')
		{
			std::cout << "The order is cancelled." << std::endl;
			return 0;
		}
		else
		{
			std::cout << "Wrong input. Please try again..." << std::endl;
		}
	} while (getChoice);
	int orderId = 0;
	try
	{
		//Create and execute statement to add completed order to order table
		oracle::occi::Statement* statement = conn->createStatement();
		statement->setSQL("BEGIN add_order(:1, :2); END;");
		statement->setNumber(1, customerId);
		statement->registerOutParam(2, oracle::occi::Type::OCCIINT);
		statement->execute();
		orderId = statement->getInt(2);

		//In a loop, update orderlines table with information about the order
		for (int i = 0; i < productCount; i++)
		{
			//Create and execute statement to add order details to orderlines table
			oracle::occi::Statement* statement = conn->createStatement();
			statement->setSQL("BEGIN add_orderlines(:1, :2, :3, :4, :5); END;");
			statement->setNumber(1, orderId);
			statement->setNumber(2, (i + 1));
			statement->setNumber(3, cart[i].product_id);
			statement->setNumber(4, cart[i].price);
			statement->setDouble(5, cart[i].quantity);
			statement->execute();
		}
		std::cout << "\nThe order is successfully completed.\n";
	}//If any of the statements in this function fail...
	catch (oracle::occi::SQLException& sqlExcp) {
		std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
}