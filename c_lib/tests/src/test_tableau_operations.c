#include <stdlib.h>
#include "widget.h"
#include "tableau_operations.h"
#include "stdarg.h"

#define ASSERT_SLICES_EQUAL(tab, i, val_x, val_z, val_r) { \
assert(val_x == tab->slices_x[i][0]); \
assert(val_z == tab->slices_z[i][0]); \
assert(val_r == tab->phases[0]);  \
}

#define WIDGET_SIZE (40000)

// Check that the X segment has full rank 
void test_full_rank_X(const size_t n_qubits)
{
    widget_t* wid = widget_create(n_qubits, n_qubits);
    for (size_t i = 0; i < n_qubits; i++)
    {   
        assert(tableau_slice_empty_x(wid->tableau, i));
    }
    
    tableau_remove_zero_X_columns(wid->tableau, wid->queue); 

    for (size_t i = 0; i < n_qubits; i++)
    {   
        assert(!tableau_slice_empty_x(wid->tableau, i));
    }

    widget_destroy(wid);

    return;
} 

tableau_t* tableau_random_create()
{
    tableau_t* tab = tableau_create(sizeof(size_t));
    for (size_t i = 0; i < sizeof(size_t); i++)
    {
       tab->slices_x[i][0] = rand(); 
       tab->slices_z[i][0] = rand(); 
    }     
    tab->phases[0] = rand(); 

    return tab;
} 


// Helper Function
void test_ident(tableau_t* tab, size_t targ, size_t n_args, ...)
{
    va_list argptr;
    va_start(argptr, n_args);
    void (*fn)(tableau_t*, size_t) = NULL; 
     
    while (n_args > 0)
    {
        fn = va_arg(argptr, void*); 
        fn(tab, targ); 
        n_args--;
    }
    return;
}


void test_cliffords()
{

    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t); i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        test_ident(tab, i, 2, tableau_X, tableau_X);

        ASSERT_SLICES_EQUAL(tab, i, val_x, val_z, val_r); 

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        test_ident(tab, i, 2, tableau_Z, tableau_Z);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        test_ident(tab, i, 2,
        tableau_H,
        tableau_H);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        test_ident(tab, i, 4,
        tableau_X,
        tableau_H,
        tableau_Z,
        tableau_H
        );

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_Z(tab, i);

        tableau_H(tab, i);
        tableau_X(tab, i);
        tableau_H(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_Y(tab, i);
        tableau_Y(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_Y(tab, i);

        tableau_Z(tab, i);
        tableau_X(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_Y(tab, i);

        tableau_X(tab, i);
        tableau_Z(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_Z(tab, i);
        tableau_S(tab, i);
        tableau_S(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_Z(tab, i);
        tableau_R(tab, i);
        tableau_R(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_R(tab, i);
        tableau_S(tab, i);
        
        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_SX(tab, i);
        tableau_R(tab, i);
        tableau_X(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_RX(tab, i);
        tableau_S(tab, i);
        tableau_X(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_HZ(tab, i);
        tableau_H(tab, i);
        tableau_Z(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  


        tableau_SH(tab, i);
        tableau_R(tab, i);
        tableau_H(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_RH(tab, i);
        tableau_S(tab, i);
        tableau_H(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_HS(tab, i);
        tableau_R(tab, i);
        tableau_H(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_HSX(tab, i);
        tableau_H(tab, i);
        tableau_R(tab, i);
        tableau_X(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_HRX(tab, i);
        tableau_H(tab, i);
        tableau_S(tab, i);
        tableau_X(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_SHY(tab, i);
        tableau_R(tab, i);
        tableau_H(tab, i);
        tableau_Y(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_RHY(tab, i);
        tableau_S(tab, i);
        tableau_H(tab, i);
        tableau_Y(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_HSH(tab, i);
        tableau_H(tab, i);
        tableau_R(tab, i);
        tableau_H(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_HRH(tab, i);
        tableau_H(tab, i);
        tableau_S(tab, i);
        tableau_H(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_RHS(tab, i);
        tableau_S(tab, i);
        tableau_H(tab, i);
        tableau_R(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

        tableau_SHR(tab, i);
        tableau_R(tab, i);
        tableau_H(tab, i);
        tableau_S(tab, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  
        assert(val_r == tab->phases[0]);  

    }   
    tableau_destroy(tab);
}

/*
 * Tests CNOTs self inverse
 *
 */
void test_cnot_1()
{
    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t) - 1; i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        const size_t val_x_nxt = tab->slices_x[i + 1][0];  
        const size_t val_z_nxt = tab->slices_z[i + 1][0];  

        tableau_CNOT(tab, i, i + 1);
        tableau_CNOT(tab, i, i + 1);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  

        assert(val_x_nxt == tab->slices_x[i + 1][0]);  
        assert(val_z_nxt == tab->slices_z[i + 1][0]);  

        assert(val_r == tab->phases[0]);  

    }
    tableau_destroy(tab);
    return;
}



/*
 * Tests swapping property of CNOTs
 */
void test_cnot_2()
{
    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t) - 1; i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        const size_t val_x_nxt = tab->slices_x[i + 1][0];  
        const size_t val_z_nxt = tab->slices_z[i + 1][0];  

        tableau_CNOT(tab, i, i + 1);
        tableau_CNOT(tab, i + 1, i);
        tableau_CNOT(tab, i, i + 1);

        assert(val_x_nxt == tab->slices_x[i][0]);  
        assert(val_z_nxt == tab->slices_z[i][0]);  

        assert(val_x == tab->slices_x[i + 1][0]);  
        assert(val_z == tab->slices_z[i + 1][0]);  

        tableau_CNOT(tab, i, i + 1);
        tableau_CNOT(tab, i + 1, i);
        tableau_CNOT(tab, i, i + 1);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  

        assert(val_x_nxt == tab->slices_x[i + 1][0]);  
        assert(val_z_nxt == tab->slices_z[i + 1][0]);  

        assert(val_r == tab->phases[0]);  


    }
    tableau_destroy(tab);
    return;
}


void test_cnot_3()
{
    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t) - 1; i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        const size_t val_x_nxt = tab->slices_x[i + 1][0];  
        const size_t val_z_nxt = tab->slices_z[i + 1][0];  

        tableau_H(tab, i);
        tableau_H(tab, i + 1);
        tableau_CNOT(tab, i, i + 1);
        tableau_H(tab, i);
        tableau_H(tab, i + 1);
        tableau_CNOT(tab, i + 1, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  

        assert(val_x_nxt == tab->slices_x[i + 1][0]);  
        assert(val_z_nxt == tab->slices_z[i + 1][0]);  

        assert(val_r == tab->phases[0]);  
    }
    tableau_destroy(tab);
}

/*
 * Test CZ(a, b) CZ(a, b) == I
 */
void test_cz_1()
{
    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t) - 1; i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        const size_t val_x_nxt = tab->slices_x[i + 1][0];  
        const size_t val_z_nxt = tab->slices_z[i + 1][0];  

        tableau_CZ(tab, i, i + 1);
        tableau_CZ(tab, i, i + 1);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  

        assert(val_x_nxt == tab->slices_x[i + 1][0]);  
        assert(val_z_nxt == tab->slices_z[i + 1][0]);  
        assert(val_r == tab->phases[0]);  
    }
    tableau_destroy(tab);
    return;
}


/*
 * Test: 
 * ---.---.--
 *    |   |
 * -H-.-H-+--
 *  Is kr(I, I)
 */
void test_cz_2()
{
    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t) - 1; i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        const size_t val_x_nxt = tab->slices_x[i + 1][0];  
        const size_t val_z_nxt = tab->slices_z[i + 1][0];  

        tableau_H(tab, i + 1);
        tableau_CZ(tab, i, i + 1);
        tableau_H(tab, i + 1);
        tableau_CNOT(tab, i, i + 1);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  

        assert(val_x_nxt == tab->slices_x[i + 1][0]);  
        assert(val_z_nxt == tab->slices_z[i + 1][0]);  

        assert(val_r == tab->phases[0]);  
    }
    tableau_destroy(tab);
    return;
}

/*
 * Test: 
 * -H-.-H-+--
 *    |   |
 * ---.---.--
 *  Is kr(I, I)
 */
void test_cz_3()
{
    tableau_t* tab = tableau_random_create();
    for (size_t i = 0; i < sizeof(size_t) - 1; i++)
    {
        const size_t val_x = tab->slices_x[i][0];  
        const size_t val_z = tab->slices_z[i][0];  
        const size_t val_r = tab->phases[0];  

        const size_t val_x_nxt = tab->slices_x[i + 1][0];  
        const size_t val_z_nxt = tab->slices_z[i + 1][0];  

        tableau_H(tab, i);
        tableau_CZ(tab, i, i + 1);
        tableau_H(tab, i);
        tableau_CNOT(tab, i + 1, i);

        assert(val_x == tab->slices_x[i][0]);  
        assert(val_z == tab->slices_z[i][0]);  

        assert(val_x_nxt == tab->slices_x[i + 1][0]);  
        assert(val_z_nxt == tab->slices_z[i + 1][0]);  

        assert(val_r == tab->phases[0]);  
    }
    tableau_destroy(tab);
    return;
}


void test_non_local()
{
    test_cnot_1();
    test_cnot_2();
    test_cnot_3();
    test_cz_1();
    test_cz_2();
    test_cz_3();
}

int main()
{
    for (size_t i = CACHE_SIZE_BITS; i < CACHE_SIZE_BITS * 30 + 1; i += CACHE_SIZE_BITS)
    {
        test_full_rank_X(i);
    }  

    tableau_t* tab = tableau_random_create(); 
    tableau_destroy(tab);

    test_cliffords();
    test_non_local();

    return 0;
}
