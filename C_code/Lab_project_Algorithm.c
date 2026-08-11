#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_DEVICES 100
#define DEFAULT_TARIFF_RATE 0.15 // $0.15 per kWh (Configurable)
#define ANOMALY_THRESHOLD_KWH 5.0 // >5 kWh/day flagged as unusually high

// ==========================================
// MODULE 1: DATA STRUCTURES & DATA MODELS
// ==========================================

typedef struct {
    int id;
    char name[50];
    char room[50];
    double power_watts;      // Rated power in Watts
    double hours_per_day;    // Daily usage in hours
    double daily_kwh;        // Calculated daily energy
    double daily_cost;       // Calculated daily cost
} Device;

typedef struct {
    Device devices[MAX_DEVICES];
    int count;
    double electricity_rate; // Tariff rate per kWh
} EnergySystem;

// Max-Heap Priority Queue structure for ranking high-consumption devices
typedef struct {
    Device heap[MAX_DEVICES];
    int size;
} PriorityQueue;

// ==========================================
// MODULE 2: CORE CALCULATIONS
// ==========================================

void update_device_metrics(Device *d, double rate) {
    // kWh = (Watts * Hours) / 1000
    d->daily_kwh = (d->power_watts * d->hours_per_day) / 1000.0;
    d->daily_cost = d->daily_kwh * rate;
}

void recalculate_system_totals(EnergySystem *sys) {
    for (int i = 0; i < sys->count; i++) {
        update_device_metrics(&sys->devices[i], sys->electricity_rate);
    }
}

double get_total_daily_kwh(const EnergySystem *sys) {
    double total = 0;
    for (int i = 0; i < sys->count; i++) {
        total += sys->devices[i].daily_kwh;
    }
    return total;
}

double get_total_daily_cost(const EnergySystem *sys) {
    double total = 0;
    for (int i = 0; i < sys->count; i++) {
        total += sys->devices[i].daily_cost;
    }
    return total;
}

// ==========================================
// MODULE 3: SEARCHING & SORTING ALGORITHMS
// ==========================================

// Linear search by device name (case-insensitive substring search)
int search_device_by_name(const EnergySystem *sys, const char *query) {
    printf("\n--- Search Results for \"%s\" ---\n", query);
    int found = 0;
    for (int i = 0; i < sys->count; i++) {
        if (strstr(sys->devices[i].name, query) != NULL ||
            strstr(sys->devices[i].room, query) != NULL) {
            printf("[%d] %s (%s) | Power: %.1fW | Usage: %.1f hrs/day | Daily: %.2f kWh ($%.2f)\n",
                   sys->devices[i].id, sys->devices[i].name, sys->devices[i].room,
                   sys->devices[i].power_watts, sys->devices[i].hours_per_day,
                   sys->devices[i].daily_kwh, sys->devices[i].daily_cost);
            found++;
        }
    }
    return found;
}

// QuickSort Partition by Daily kWh (Descending Order)
int partition_by_consumption(Device arr[], int low, int high) {
    double pivot = arr[high].daily_kwh;
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (arr[j].daily_kwh >= pivot) { // Descending comparison
            i++;
            Device temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    Device temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

// QuickSort Algorithm implementation
void quicksort_devices(Device arr[], int low, int high) {
    if (low < high) {
        int pi = partition_by_consumption(arr, low, high);
        quicksort_devices(arr, low, pi - 1);
        quicksort_devices(arr, pi + 1, high);
    }
}

void sort_devices_by_energy(EnergySystem *sys) {
    if (sys->count == 0) return;
    quicksort_devices(sys->devices, 0, sys->count - 1);
    printf("\n[+] Devices successfully sorted by daily energy consumption (Highest -> Lowest).\n");
}

// ==========================================
// MODULE 4: PRIORITY QUEUE (MAX-HEAP)
// ==========================================

void pq_swap(Device *a, Device *b) {
    Device temp = *a;
    *a = *b;
    *b = temp;
}

void pq_insert(PriorityQueue *pq, Device dev) {
    if (pq->size >= MAX_DEVICES) return;
    pq->heap[pq->size] = dev;
    int current = pq->size;
    pq->size++;

    // Heapify Up
    while (current > 0 && pq->heap[current].daily_kwh > pq->heap[(current - 1) / 2].daily_kwh) {
        pq_swap(&pq->heap[current], &pq->heap[(current - 1) / 2]);
        current = (current - 1) / 2;
    }
}

Device pq_extract_max(PriorityQueue *pq) {
    Device dummy = {0};
    if (pq->size <= 0) return dummy;

    Device max_dev = pq->heap[0];
    pq->heap[0] = pq->heap[pq->size - 1];
    pq->size--;

    // Heapify Down
    int current = 0;
    while (2 * current + 1 < pq->size) {
        int left = 2 * current + 1;
        int right = 2 * current + 2;
        int largest = left;

        if (right < pq->size && pq->heap[right].daily_kwh > pq->heap[left].daily_kwh) {
            largest = right;
        }

        if (pq->heap[current].daily_kwh >= pq->heap[largest].daily_kwh) break;

        pq_swap(&pq->heap[current], &pq->heap[largest]);
        current = largest;
    }

    return max_dev;
}

void display_priority_ranking(const EnergySystem *sys) {
    PriorityQueue pq = { .size = 0 };
    for (int i = 0; i < sys->count; i++) {
        pq_insert(&pq, sys->devices[i]);
    }

    printf("\n=======================================================\n");
    printf("     PRIORITY RANKING: HIGH ENERGY CONSUMERS (MAX-HEAP) \n");
    printf("=======================================================\n");
    int rank = 1;
    while (pq.size > 0) {
        Device dev = pq_extract_max(&pq);
        printf(" Rank #%d | %-15s (%-10s) | %.2f kWh/day | $%.2f/day\n",
               rank++, dev.name, dev.room, dev.daily_kwh, dev.daily_cost);
    }
    printf("=======================================================\n");
}

// ==========================================
// MODULE 5: SMART ANALYSIS & RECOMMENDATIONS
// ==========================================

void detect_anomalies(const EnergySystem *sys) {
    printf("\n--- High Consumption Anomaly Detection ---\n");
    int flagged = 0;
    for (int i = 0; i < sys->count; i++) {
        if (sys->devices[i].daily_kwh >= ANOMALY_THRESHOLD_KWH) {
            printf("[ALERT] %s (%s) exceeds threshold! Consumption: %.2f kWh/day (Threshold: %.1f kWh)\n",
                   sys->devices[i].name, sys->devices[i].room,
                   sys->devices[i].daily_kwh, ANOMALY_THRESHOLD_KWH);
            flagged++;
        }
    }
    if (flagged == 0) {
        printf("[+] No abnormal energy spikes detected (all devices under %.1f kWh/day).\n", ANOMALY_THRESHOLD_KWH);
    }
}

void generate_energy_recommendations(const EnergySystem *sys) {
    printf("\n--- Smart Energy-Saving Recommendations ---\n");
    for (int i = 0; i < sys->count; i++) {
        const Device *d = &sys->devices[i];
        if (d->hours_per_day > 12.0 && d->power_watts > 100.0) {
            printf("* [%s] Running for %.1f hrs/day. Consider turning off when inactive to save up to $%.2f/month.\n",
                   d->name, d->hours_per_day, (d->daily_cost * 0.3) * 30);
        }
        if (d->power_watts >= 1500.0) {
            printf("* [%s] High wattage device (%.0fW). Upgrading to an inverter-based model could save ~20-30%% energy.\n",
                   d->name, d->power_watts);
        }
    }
}

void generate_daily_report(const EnergySystem *sys) {
    double total_kwh = get_total_daily_kwh(sys);
    double total_daily_cost = get_total_daily_cost(sys);
    double estimated_monthly_cost = total_daily_cost * 30.0;

    printf("\n=======================================================\n");
    printf("               DAILY ENERGY & BILL REPORT              \n");
    printf("=======================================================\n");
    printf(" Active Tariff Rate          : $%.3f per kWh\n", sys->electricity_rate);
    printf(" Total Registered Devices    : %d\n", sys->count);
    printf(" Total Daily Consumption     : %.3f kWh\n", total_kwh);
    printf(" Total Daily Bill            : $%.2f\n", total_daily_cost);
    printf(" Estimated Monthly Bill (30d): $%.2f\n", estimated_monthly_cost);
    printf("-------------------------------------------------------\n");
    printf(" ID | Name            | Room       | Watts | Hrs  | kWh/Day | $/Day \n");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < sys->count; i++) {
        const Device *d = &sys->devices[i];
        printf(" %2d | %-15s | %-10s | %5.0f | %4.1f | %7.2f | %5.2f\n",
               d->id, d->name, d->room, d->power_watts, d->hours_per_day, d->daily_kwh, d->daily_cost);
    }
    printf("=======================================================\n");
}

// ==========================================
// MODULE 6: USER INTERFACE & MENU DRIVER
// ==========================================

void add_device_interactive(EnergySystem *sys) {
    if (sys->count >= MAX_DEVICES) {
        printf("[!] Device capacity reached.\n");
        return;
    }
    Device d;

    // Find the highest existing ID to prevent duplicates if devices were removed
    int max_id = 0;
    for (int i = 0; i < sys->count; i++) {
        if (sys->devices[i].id > max_id) {
            max_id = sys->devices[i].id;
        }
    }
    d.id = max_id + 1;

    printf("\nEnter Device Name (e.g., Air Conditioner): ");
    scanf(" %[^\n]s", d.name);
    printf("Enter Room (e.g., Living Room): ");
    scanf(" %[^\n]s", d.room);
    printf("Enter Power Rating (in Watts): ");
    scanf("%lf", &d.power_watts);
    printf("Enter Daily Operational Hours (0-24): ");
    scanf("%lf", &d.hours_per_day);

    update_device_metrics(&d, sys->electricity_rate);
    sys->devices[sys->count++] = d;
    printf("[+] Device added successfully! (Assigned ID: %d)\n", d.id);
}

// NEW FUNCTION: Array Deletion Logic
void remove_device_interactive(EnergySystem *sys) {
    if (sys->count == 0) {
        printf("[!] No devices to remove.\n");
        return;
    }

    int target_id;
    printf("\nEnter the ID of the device to remove (check the Daily Report for IDs): ");
    if (scanf("%d", &target_id) != 1) {
        printf("[!] Invalid input.\n");
        while(getchar() != '\n'); // clear input buffer
        return;
    }

    int found_index = -1;
    for (int i = 0; i < sys->count; i++) {
        if (sys->devices[i].id == target_id) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        printf("[+] Removing device: %s (%s)\n", sys->devices[found_index].name, sys->devices[found_index].room);

        // Shift all elements after the deleted item one position left
        for (int i = found_index; i < sys->count - 1; i++) {
            sys->devices[i] = sys->devices[i + 1];
        }

        sys->count--; // Reduce the total device count
        printf("[+] Device removed successfully!\n");
    } else {
        printf("[!] Error: No device found with ID %d.\n", target_id);
    }
}

void load_sample_data(EnergySystem *sys) {
    Device samples[] = {
        {1, "AC (1.5 Ton)", "Bedroom 1", 1800, 8.0, 0, 0},
        {2, "Refrigerator", "Kitchen", 200, 24.0, 0, 0},
        {3, "Water Heater", "Bathroom", 2000, 1.5, 0, 0},
        {4, "Desktop PC", "Study", 350, 10.0, 0, 0},
        {5, "LED TV", "Living Room", 100, 6.0, 0, 0}
    };
    int num_samples = sizeof(samples) / sizeof(samples[0]);
    for (int i = 0; i < num_samples; i++) {
        update_device_metrics(&samples[i], sys->electricity_rate);
        sys->devices[sys->count++] = samples[i];
    }
}

int main() {
    EnergySystem sys = { .count = 0, .electricity_rate = DEFAULT_TARIFF_RATE };

    // Seed system with initial realistic device data
    load_sample_data(&sys);

    int choice;
    char search_buf[50];

    do {
        printf("\n==================================================\n");
        printf("      SMART ENERGY MONITORING SYSTEM (v1.1)       \n");
        printf("==================================================\n");
        printf(" 1. Add New Device\n");
        printf(" 2. Remove a Device\n"); // <-- NEW OPTION
        printf(" 3. Generate Daily Report & Monthly Bill Estimate\n");
        printf(" 4. Search Device by Name / Room\n");
        printf(" 5. QuickSort Devices by Energy Consumption\n");
        printf(" 6. Priority Queue Ranking (Max-Heap Top Consumers)\n");
        printf(" 7. Run High Consumption Anomaly Check\n");
        printf(" 8. View Energy-Saving Recommendations\n");
        printf(" 9. Change Electricity Tariff Rate (Current: $%.3f/kWh)\n", sys.electricity_rate);
        printf(" 10. Exit\n");
        printf("--------------------------------------------------\n");
        printf("Select an option (1-10): ");

        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); // clear buffer
            choice = 0; // force default case
        }

        switch (choice) {
            case 1:
                add_device_interactive(&sys);
                break;
            case 2:
                remove_device_interactive(&sys); // <-- CALL NEW FUNCTION
                break;
            case 3:
                generate_daily_report(&sys);
                break;
            case 4:
                printf("Enter search term (Device name/Room): ");
                scanf(" %[^\n]s", search_buf);
                search_device_by_name(&sys, search_buf);
                break;
            case 5:
                sort_devices_by_energy(&sys);
                generate_daily_report(&sys);
                break;
            case 6:
                display_priority_ranking(&sys);
                break;
            case 7:
                detect_anomalies(&sys);
                break;
            case 8:
                generate_energy_recommendations(&sys);
                break;
            case 9:
                printf("Enter new tariff rate per kWh ($): ");
                scanf("%lf", &sys.electricity_rate);
                recalculate_system_totals(&sys);
                printf("[+] Tariff rate updated to $%.3f/kWh.\n", sys.electricity_rate);
                break;
            case 10:
                printf("\nExiting Energy Monitor System. Goodbye!\n");
                break;
            default:
                printf("[!] Invalid choice. Please enter a number between 1 and 10.\n");
        }
    } while (choice != 10);

    return 0;
}