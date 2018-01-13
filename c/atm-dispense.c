/* Write a program to dispense currencies in an ATM
 * Optimize as much as possible
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

/* Given */
#define NCUR 11                      /* Number of denominations available */

uint64_t total_amt = 0;                  /* Total amount available in ATM */
uint32_t arry_num_denoms[NCUR] = {0,};        /* denominations array */
uint32_t arry_denoms[NCUR] = {1, 2, 5,     /* denomination split */
                              10, 20, 50,
                              100, 200, 500,
                              1000, 2000};

uint32_t arry_dispense[NCUR] = {0,};

/*
 * denom_start : a binary search to evaluate the starting point
 */
uint32_t denom_start(uint32_t amt)
{
        int lo = 0, hi = NCUR - 1, mid;

        if (amt > arry_denoms[hi])
                return hi;

        while (lo < hi) {
                mid = (lo + hi) / 2;
                if ((mid < hi - 1) && (arry_denoms[mid + 1] > amt))
                    break;
                if (arry_denoms[mid] > amt) {
                        hi = mid;
                } else {
                        lo = mid;
                }
        }

        return mid;
}

void print_array(int ncur, uint32_t arry[])
{
        int i;
        printf("arry [");
        for (i = ncur - 1; i >= 0; i--) {
                printf("%d \t", arry[i]);
        }
        printf("]\n");
}
int dispense(uint32_t amt)
{
        int i;
        uint32_t amt_bck = amt;
        if (amt > total_amt)
                return -1;      /* Cannot dispense, ATM doesn't have enough money */

        // Figure out where to start.
        i = denom_start(amt);

        for (; amt && i >= 0; i--) {
                if (!arry_denoms[i])
                        continue;
                uint32_t num_disp = amt / arry_denoms[i];
                num_disp = (num_disp < arry_num_denoms[i]) ? num_disp : arry_num_denoms[i];
                uint32_t curr_dispense = arry_denoms[i] * num_disp;
                arry_dispense[i] = num_disp;
                amt -= curr_dispense;
                arry_num_denoms[i] -= num_disp;
                printf("dispensing %d * %d = %d\n", arry_denoms[i], num_disp,
                       curr_dispense);
        }

        if (amt) {
                printf("Couldn't dispense, left:%d\n", amt);
                print_array(NCUR, arry_dispense);
                return -1;
        }

        total_amt -= amt_bck;   /* update total amount as we dispensed */
        print_array(NCUR, arry_dispense);
        return 0;
}

uint64_t fill_random_amount(uint32_t ncur, uint32_t arr[])
{
        int i;
        uint64_t total = 0;
        uint32_t val;

        /* Open /dev/urandom for random seed */
        int fd = open("/dev/urandom", O_RDONLY);

        read(fd, &val, 4);

        srand(val);
        for (i = ncur - 1; i >= 0; i--) {
                uint32_t c = rand() % 10;
                arr[i] = c;
                total += arr[i] * arry_denoms[i];
        }

        close(fd);
        printf("Filled random denoms\n");
        return total;
}

int main(void)
{
        uint amt;

        total_amt = fill_random_amount(NCUR, arry_num_denoms);

        while(total_amt) {
                print_array(NCUR, arry_denoms);
                print_array(NCUR, arry_num_denoms);
                printf("total amount: %lld\n", total_amt);
                printf("Enter amount to dispense: ");
                scanf("%d", &amt);
                fflush(stdout);
                dispense(amt);
        }
}
