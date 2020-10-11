/**  Do not modify this file  **/

#include "Bank.h"
#include <stdlib.h>
#include <unistd.h>


int *BANK_accounts;	//Array for storing account values

#define WAIT_TIME 10000

/*
 *  Intialize back accounts
 *  Input:  int n - Number of bank accounts
 *  Return:  1 if succeeded, 0 if error
 */
int initialize_accounts( int n )
{
	BANK_accounts = (int *) malloc(sizeof(int) * n);
	if(BANK_accounts == NULL) return 0;

	int i;
	for( i = 0; i < n; i++)
	{
		BANK_accounts[i] = 0;
	}
	return 1;
}

/*
 *  Read a bank account
 *  Input:  int ID - Id of bank account to read
 *  Return:  Value of bank account ID
 */
int read_account( int ID )
{
	usleep( WAIT_TIME );
	return BANK_accounts[ID - 1];
}

/*
 *  Write value to bank account
 *  Input:  int ID - Id of bank account to write to
 *  Input:  int value - value to write to account
 */
void write_account( int ID, int value)
{
	usleep( WAIT_TIME );
	BANK_accounts[ID - 1] = value;
}

/*
 * Deallocate the memory for bank accounts
 */
 void free_accounts()
 {
 	free(BANK_accounts);
 }