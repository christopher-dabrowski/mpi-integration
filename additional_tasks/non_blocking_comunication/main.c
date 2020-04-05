// Wykorzystam komunikację nie blokującą do zrobienia prostej symulacji
// klienta zamawiającego co jakiś czas pizzę z restauracji

#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

typedef enum PizzaType
{
    Margarita,
    Neapolitan,
    Sicilian,
    Greek
} PizzaType;

const char* pizzaTypeToString(PizzaType pizza)
{
    switch (pizza)
    {
    case Margarita:
        return "Margarita";
    case Neapolitan:
        return "Neapolitan";
    case Sicilian:
        return "Sicilian";
    case Greek:
        return "Greek";
    default:
        return "Empty plate";
    }
}

// Random number from range [from, to)
int randomInt(int from, int to)
{
    return rand() % (to - from ) + from;
}

PizzaType choosePizza()
{
    return (PizzaType) randomInt(0, 4);
}

int main(int argc, char const *argv[])
{
    MPI_Init(NULL, NULL);
    srand(time(NULL));

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_size < 2)
    {
        fputs("Program requires at lest 2 processes to run\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (world_rank == 0) // Pizza store
    {
        MPI_Request request;
        PizzaType orderedPizza;
        int requestComplite = 0;
        MPI_Irecv(&orderedPizza, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);

        while (true)
        {
            MPI_Test(&request, &requestComplite, MPI_STATUS_IGNORE);
            // MPI_Recv(&orderedPizza, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (requestComplite)
            {
                printf("\033[0;31mPizzeria is preparing %s\033[0m\n", pizzaTypeToString(orderedPizza));
                sleep(3);
                printf("\033[0;31mPizzeria: %s is REDY!\033[0m\n", pizzaTypeToString(orderedPizza));

                // Get redy for new order
                MPI_Irecv(&orderedPizza, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
            }
            else
            {
                puts("\033[0;31mNo orders are currently placed\033[0m");
            }


            puts("\033[0;31mPizzeria is busy doing business...\033[0m");
            sleep(5);
            puts("\033[0;31mPizzeria checks for placed orders\033[0m");
        }

    }
    if (world_rank == 1) // Customer
    {
        while (true)
        {
            sleep(1);
            PizzaType order = choosePizza();
            printf("Customer wants %s pizza\n", pizzaTypeToString(order));

            // Send request
            MPI_Request request;
            MPI_Isend(&order, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
            printf("Customer placed his order\n");

            int requestComplite = 0;
            while (!requestComplite)
            {
                printf("Customer is doing his work\n");
                sleep(1);
                printf("Customer finished work and checks if his pizza is redy\n");

            }
        }

    }


    MPI_Finalize();
    return 0;
}