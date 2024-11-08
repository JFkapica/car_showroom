#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

// Funkcja obsługująca błąd połączenia
void exitOnError(PGconn* conn) {
    fprintf(stderr, "Error: %s", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

// Funkcja dodająca rekord do tabeli
void addRecord(PGconn* conn, const char* name, int age) {
    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO records (name, age) VALUES ('%s', %d);", name, age);
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Add record failed: %s", PQerrorMessage(conn));
    } else {
        printf("Record added: Name=%s, Age=%d\n", name, age);
    }
    PQclear(res);
}

// Funkcja wyświetlająca wszystkie rekordy
void displayRecords(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT id, name, age FROM records ORDER BY id;");
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Display records failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("Current records:\n");
    for (int i = 0; i < PQntuples(res); i++) {
        printf("ID=%s, Name=%s, Age=%s\n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               PQgetvalue(res, i, 2));
    }
    PQclear(res);
}

// Funkcja wyświetlająca rekord według kryteriów (id lub name)
void displayRecordByCriteria(PGconn* conn, const char* criteria, const char* value) {
    char query[256];

    if (strcmp(criteria, "id") == 0) {
        snprintf(query, sizeof(query), "SELECT id, name, age FROM records WHERE id = %s;", value);
    } else if (strcmp(criteria, "name") == 0) {
        snprintf(query, sizeof(query), "SELECT id, name, age FROM records WHERE name = '%s';", value);
    } else {
        printf("Invalid criteria. Use 'id' or 'name'.\n");
        return;
    }

    PGresult* res = PQexec(conn, query);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Display record by criteria failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    if (PQntuples(res) == 0) {
        printf("No records found for %s = %s.\n", criteria, value);
    } else {
        for (int i = 0; i < PQntuples(res); i++) {
            printf("ID=%s, Name=%s, Age=%s\n",
                   PQgetvalue(res, i, 0),
                   PQgetvalue(res, i, 1),
                   PQgetvalue(res, i, 2));
        }
    }
    PQclear(res);
}

// Funkcja modyfikująca rekord
void modifyRecord(PGconn* conn, int id, const char* newName, int newAge) {
    char query[256];
    snprintf(query, sizeof(query), "UPDATE records SET name='%s', age=%d WHERE id=%d;", newName, newAge, id);
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Modify record failed: %s", PQerrorMessage(conn));
    } else {
        printf("Record modified: ID=%d, New Name=%s, New Age=%d\n", id, newName, newAge);
    }
    PQclear(res);
}

// Funkcja usuwająca rekord
void deleteRecord(PGconn* conn, int id) {
    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM records WHERE id=%d;", id);
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete record failed: %s", PQerrorMessage(conn));
    } else {
        printf("Record with ID=%d deleted.\n", id);
    }
    PQclear(res);
}

int main() {
    // Połączenie z bazą danych
    PGconn* conn = PQconnectdb("user=your_username dbname=your_database password=your_password host=localhost");

    if (PQstatus(conn) != CONNECTION_OK) {
        exitOnError(conn);
    }

    int choice, id, age;
    char name[50];
    char criteria[10];
    char value[50];

    while (1) {
        printf("\nCRUD Menu:\n");
        printf("1. Add Record\n");
        printf("2. Display All Records\n");
        printf("3. Display Record by Criteria (ID or Name)\n");
        printf("4. Modify Record\n");
        printf("5. Delete Record\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter Name: ");
                scanf(" %49[^\n]", name);
                printf("Enter Age: ");
                scanf("%d", &age);
                addRecord(conn, name, age);
                break;

            case 2:
                displayRecords(conn);
                break;

            case 3:
                printf("Enter search criteria (id/name): ");
                scanf("%s", criteria);
                printf("Enter value: ");
                scanf("%s", value);
                displayRecordByCriteria(conn, criteria, value);
                break;

            case 4:
                printf("Enter ID of the record to modify: ");
                scanf("%d", &id);
                printf("Enter New Name: ");
                scanf(" %49[^\n]", name);
                printf("Enter New Age: ");
                scanf("%d", &age);
                modifyRecord(conn, id, name, age);
                break;

            case 5:
                printf("Enter ID of the record to delete: ");
                scanf("%d", &id);
                deleteRecord(conn, id);
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
