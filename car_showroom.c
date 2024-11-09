#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

// Deklaracje funkcji
void exitOnError(PGconn* conn);
void addCar(PGconn* conn, const char* brand, const char* model, const char* color, int year_of_production, double price);
void displayAllCars(PGconn* conn);
void displayCarByCriteria(PGconn* conn);
void modifyCar(PGconn* conn, int car_id, const char* newBrand, const char* newModel, const char* newColor, int newYear, double newPrice);
void deleteCar(PGconn* conn, int car_id);

int main() {
    // Połączenie z bazą danych
    PGconn* conn = PQconnectdb("user=dbuser dbname=dbuser password=dbuser");

    if (PQstatus(conn) != CONNECTION_OK) {
        exitOnError(conn);
    }

    int choice, car_id, year_of_production;
    double price;
    char brand[30], model[60], color[30];
    char criteria[20], value[60];

    while (1) {
        printf("\nCar Database CRUD Menu:\n");
        printf("1. Add Car\n");
        printf("2. Display All Cars\n");
        printf("3. Display Car by Criteria (Car ID or Brand)\n");
        printf("4. Modify Car\n");
        printf("5. Delete Car\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter Brand: ");
                scanf(" %29[^\n]", brand);
                printf("Enter Model: ");
                scanf(" %59[^\n]", model);
                printf("Enter Color: ");
                scanf(" %29[^\n]", color);
                printf("Enter Year of Production: ");
                scanf("%d", &year_of_production);
                printf("Enter Price: ");
                scanf("%lf", &price);
                addCar(conn, brand, model, color, year_of_production, price);
                break;

            case 2:
                displayAllCars(conn);
                break;

            case 3:
                displayCarByCriteria(conn);
                break;

            case 4:
                printf("Enter Car ID of the car to modify: ");
                scanf("%d", &car_id);
                printf("Enter New Brand: ");
                scanf(" %29[^\n]", brand);
                printf("Enter New Model: ");
                scanf(" %59[^\n]", model);
                printf("Enter New Color: ");
                scanf(" %29[^\n]", color);
                printf("Enter New Year of Production: ");
                scanf("%d", &year_of_production);
                printf("Enter New Price: ");
                scanf("%lf", &price);
                modifyCar(conn, car_id, brand, model, color, year_of_production, price);
                break;

            case 5:
                printf("Enter Car ID of the car to delete: ");
                scanf("%d", &car_id);
                deleteCar(conn, car_id);
                break;

            case 0:
                PQfinish(conn);
                printf("Exiting program.\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

// Definicje funkcji

void exitOnError(PGconn* conn) {
    fprintf(stderr, "Error: %s", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

void addCar(PGconn* conn, const char* brand, const char* model, const char* color, int year_of_production, double price) {
    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO cars (brand, model, color, year_of_production, price) VALUES ('%s', '%s', '%s', %d, %.2f);",
             brand, model, color, year_of_production, price);
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Add car failed: %s", PQerrorMessage(conn));
    } else {
        printf("Car added: Brand=%s, Model=%s, Color=%s, Year=%d, Price=%.2f\n",
               brand, model, color, year_of_production, price);
    }
    PQclear(res);
}

void displayAllCars(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT car_id, brand, model, color, year_of_production, price FROM cars ORDER BY car_id;");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Display all cars failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("Current cars:\n");
    for (int i = 0; i < PQntuples(res); i++) {
        printf("Car ID=%s, Brand=%s, Model=%s, Color=%s, Year=%s, Price=%s\n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               PQgetvalue(res, i, 2),
               PQgetvalue(res, i, 3),
               PQgetvalue(res, i, 4),
               PQgetvalue(res, i, 5));
    }
    PQclear(res);
}

void displayCarByCriteria(PGconn* conn) {
    int criteria_choice;
    char query[512];
    PGresult* res;

    printf("\nSelect search criteria:\n");
    printf("1. Search by Car ID\n");
    printf("2. Search by Brand\n");
    printf("3. Search by Model\n");
    printf("4. Search by Color\n");
    printf("5. Search by Year Range\n");
    printf("6. Search by Price Range\n");
    printf("Enter your choice: ");
    scanf("%d", &criteria_choice);

    switch (criteria_choice) {
        case 1: {
            int car_id;
            printf("Enter Car ID: ");
            scanf("%d", &car_id);
            snprintf(query, sizeof(query), 
                     "SELECT * FROM cars WHERE car_id = %d;", car_id);
            break;
        }
        case 2: {
            char brand[30];
            printf("Enter Brand: ");
            scanf(" %29[^\n]", brand);
            snprintf(query, sizeof(query), 
                     "SELECT * FROM cars WHERE brand = '%s';", brand);
            break;
        }
        case 3: {
            char model[60];
            printf("Enter Model: ");
            scanf(" %59[^\n]", model);
            snprintf(query, sizeof(query), 
                     "SELECT * FROM cars WHERE model = '%s';", model);
            break;
        }
        case 4: {
            char color[30];
            printf("Enter Color: ");
            scanf(" %29[^\n]", color);
            snprintf(query, sizeof(query), 
                     "SELECT * FROM cars WHERE color = '%s';", color);
            break;
        }
        case 5: {
            int year_start, year_end;
            printf("Enter start year: ");
            scanf("%d", &year_start);
            printf("Enter end year: ");
            scanf("%d", &year_end);
            snprintf(query, sizeof(query), 
                     "SELECT * FROM cars WHERE year_of_production BETWEEN %d AND %d;", 
                     year_start, year_end);
            break;
        }
        case 6: {
            double price_min, price_max;
            printf("Enter minimum price: ");
            scanf("%lf", &price_min);
            printf("Enter maximum price: ");
            scanf("%lf", &price_max);
            snprintf(query, sizeof(query), 
                     "SELECT * FROM cars WHERE price BETWEEN %.2f AND %.2f;", 
                     price_min, price_max);
            break;
        }
        default:
            printf("Invalid choice.\n");
            return;
    }

    // Wykonaj zapytanie i wyświetl wyniki
    res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error executing query: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    // Wyświetlanie wyników
    int rows = PQntuples(res);
    if (rows == 0) {
        printf("No results found.\n");
    } else {
        for (int i = 0; i < rows; i++) {
            printf("Car ID=%s, Brand=%s, Model=%s, Color=%s, Year=%s, Price=%s\n",
                   PQgetvalue(res, i, 0),
                   PQgetvalue(res, i, 1),
                   PQgetvalue(res, i, 2),
                   PQgetvalue(res, i, 3),
                   PQgetvalue(res, i, 4),
                   PQgetvalue(res, i, 5));
        }
    }
    PQclear(res);
}

void modifyCar(PGconn* conn, int car_id, const char* newBrand, const char* newModel, const char* newColor, int newYear, double newPrice) {
    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE cars SET brand='%s', model='%s', color='%s', year_of_production=%d, price=%.2f WHERE car_id=%d;",
             newBrand, newModel, newColor, newYear, newPrice, car_id);
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Modify car failed: %s", PQerrorMessage(conn));
    } else {
        printf("Car modified: Car ID=%d, New Brand=%s, New Model=%s, New Color=%s, New Year=%d, New Price=%.2f\n",
               car_id, newBrand, newModel, newColor, newYear, newPrice);
    }
    PQclear(res);
}

void deleteCar(PGconn* conn, int car_id) {
    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM cars WHERE car_id=%d;", car_id);
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete car failed: %s", PQerrorMessage(conn));
    } else {
        printf("Car with Car ID=%d deleted.\n", car_id);
    }
    PQclear(res);
}
