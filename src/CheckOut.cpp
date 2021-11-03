#include <iostream>
#include <occi.h>
#include <iomanip> 
#include "../ShoppingCart.h"
#include "../Utils.h"

//Maximum products the cart is allowed hold
const unsigned int CART_SIZE = 10;

	int main(void)
	{
		//OCCI Variables
		oracle::occi::Environment* env = nullptr;
		oracle::occi::Connection* conn = nullptr;
		oracle::occi::Statement * stmt = nullptr;
		oracle::occi::ResultSet* resSet = nullptr;
		//User Variables
		std::string str;
		std::string usr = "REMOVED";
		std::string pass = "REMOVED";
		std::string srv = "REMOVED";

		ShoppingCart cart[CART_SIZE];

		//if creation of connection is successful, continue with program, otherwise get and display error code.
		try {
			env = oracle::occi::Environment::createEnvironment(oracle::occi::Environment::DEFAULT);
			conn = env->createConnection(usr, pass, srv);

			//find_customer stored procedure
			//Takes in customer id and checks if database contains customer with given id, if does returns 1 else 0.
			stmt = conn->createStatement("CREATE OR REPLACE PROCEDURE find_customer (customer_id IN NUMBER, found OUT NUMBER) AS cno NUMBER := 1; BEGIN SELECT cust_no INTO cno FROM customers WHERE cust_no = customer_id; found:= 1; EXCEPTION WHEN no_data_found THEN found := 0; END find_customer;");
			stmt->execute();

			//find_product stored procedure 
			//Takes in a product id and searches database for product with matching id, returns 1 if found 0 if not.
			stmt = conn->createStatement("CREATE OR REPLACE PROCEDURE find_product (product_id IN NUMBER, price OUT products.prod_sell % TYPE) AS BEGIN SELECT prod_sell INTO price FROM products WHERE product_id = prod_no; EXCEPTION WHEN no_data_found THEN price:=0; END find_product;");
			stmt->execute();

			//Add order stored procedure
			//Pushes given values into the orders table
			stmt = conn->createStatement("CREATE OR REPLACE PROCEDURE add_order (customer_id IN NUMBER, new_order_id OUT NUMBER) AS BEGIN SELECT (MAX(order_no) +1 ) INTO new_order_id FROM orders; INSERT INTO orders VALUES (new_order_id, 34, customer_id, sysdate, 'S', null); EXCEPTION WHEN OTHERS THEN new_order_id:=0; END;");
			stmt->execute();

			//add_orderlne stored procedure
			//Adds given values into orderlines table
			stmt = conn->createStatement("CREATE OR REPLACE PROCEDURE add_orderlines(orderid IN orderlines.order_no % TYPE, itemid IN orderlines.order_no % TYPE, productid IN orderlines.prod_no % TYPE, quantity IN orderlines.qty % TYPE, price IN orderlines.price % TYPE) AS BEGIN INSERT INTO orderlines(order_no , line_no, prod_no, price, qty) VALUES (orderid, itemid, productid, price, quantity); END add_orderlines;");
			stmt->execute();
	
			int menuControl = 0;;
			// loop this until user enteres 0 in menu
			do
			{
				menuControl = mainMenu();
				switch (menuControl)
				{
				case 1:
						int id;
						std::cout << "Enter the customer ID: ";
						std::cin >> id;
						std::cin.ignore();
						if (std::cin.fail())
						{
							std::cout << "The customer does not exist." << std::endl;
						}
						else
						{	//done will = 1 when id matches id in database
							if (!(customerLogin(conn, id)))
							{
								std::cout << "The customer does not exist." << std::endl;
							}
							else
							{
								int itemCount = addToCart(conn, cart);
								displayProducts(cart, itemCount);
								checkout(conn,cart,id,itemCount);
							}
						}
					break;
				default:
					break;
				}
			} while (menuControl);
			//Terminate Connection
			env->terminateConnection(conn);
			oracle::occi::Environment::terminateEnvironment(env);
		}
		catch (oracle::occi::SQLException& sqlExcp)
		{//If any of the above fails (creating/updating procedures) print error code and terminate connection
			std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
			env->terminateConnection(conn);
			oracle::occi::Environment::terminateEnvironment(env);
		}
		return 0;
}