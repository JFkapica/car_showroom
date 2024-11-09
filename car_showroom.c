#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

// Deklaracje funkcji
void exitOnError(PGconn* conn);
void addCar(PGconn* conn, const char* brand, const char* model, const char* color, int year_of_production, double price);
void displayAllCars(PGconn* conn);
void displayCarByCriteria(PGconn* conn, const char* criteria, const char* value);
void modifyCar(PGconn* conn, int car_id, const char* newBrand, const char* newModel, const char* newColor, int newYear, double newPrice);
void deleteCar(PGconn* conn, int car_id);

int main() {
    // Połączenie z bazą danych
    PGconn* conn = PQconnectdb("user=your_username dbname=your_database password=your_password host=localhost");

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
        printf("6. Exit\n");
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
                printf("Enter search criteria (car_id/brand): ");
                scanf("%s", criteria);
                printf("Enter value: ");
                scanf("%s", value);
                displayCarByCriteria(conn, criteria, value);
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

            case 6:
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

void displayCarByCriteria(PGconn* conn, const char* criteria, const char* value) {
    char query[512];

    if (strcmp(criteria, "car_id") == 0) {
        snprintf(query, sizeof(query), "SELECT car_id, brand, model, color, year_of_production, price FROM cars WHERE car_id = %s;", value);
    } else if (strcmp(criteria, "brand") == 0) {
        snprintf(query, sizeof(query), "SELECT car_id, brand, model, color, year_of_production, price FROM cars WHERE brand = '%s';", value);
    } else {
        printf("Invalid criteria. Use 'car_id' or 'brand'.\n");
        return;
    }

    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Display car by criteria failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    if (PQntuples(res) == 0) {
        printf("No cars found for %s = %s.\n", criteria, value);
    } else {
        for (int i = 0; i < PQntuples(res); i++) {
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
