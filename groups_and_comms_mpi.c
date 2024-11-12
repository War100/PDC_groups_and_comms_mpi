#include <mpi.h>
#include <stdio.h>

#define P 24 // Кількість необхідних процесів

int main(int argc, char *argv[]) {
	int rank, size, world_size;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Вивід помилки у випадку невідповідної кількості процесів
	if (world_size != P) {
		if (rank == 0) {
			printf("Error: Number of processes should be %d\n", P);
		}
		MPI_Finalize();
		return 1;	// Код повернення у разі помилки кільксоті процесів
	}

	// 1. Створення загального комунікатора
	MPI_Comm all_comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &all_comm);

	// 2. Розділення на парні та непарні комунікатори
	MPI_Comm even_comm, odd_comm;
	if (rank % 2 == 0) {
		MPI_Comm_split(MPI_COMM_WORLD, 0, rank, &even_comm);
		odd_comm = MPI_COMM_NULL;
	} else {
		MPI_Comm_split(MPI_COMM_WORLD, 1, rank, &odd_comm);
		even_comm = MPI_COMM_NULL;
	}

	// 3. Створення груп у кожному комунікаторі
	MPI_Group world_group, even_group, odd_group, group1, group2, group3;
	MPI_Group all_group1, all_group2, all_group3; // Групи комунікатора для всіх процесів

	MPI_Barrier(MPI_COMM_WORLD);

	// 4. Наповнення створених груп необхідними процесами
	if (all_comm != MPI_COMM_NULL) { // Комунікатор для всіх процесів
		int ranks_to_incl_group1[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		int ranks_to_excl_group2[10] = { 6, 7, 8, 9,10,11,12,13,14,15};
		int ranks_to_incl_group3[10] = { 0,14,15,16,17,18,19,20,21,22};

		MPI_Comm_group(all_comm, &world_group);

		MPI_Group_incl(world_group, 10, ranks_to_incl_group1, &all_group1);
		MPI_Group_excl(world_group, 10, ranks_to_excl_group2, &all_group2);
		MPI_Group_incl(world_group, 10, ranks_to_incl_group3, &all_group3);
	}
	if (even_comm != MPI_COMM_NULL) { // Комунікатор для парних процесів
		int ranks_to_incl_group1[6] = { 0, 2, 4, 6, 8,10};

		MPI_Comm_group(even_comm, &even_group);

		MPI_Group_incl(even_group, 6, ranks_to_incl_group1, &group1);
		MPI_Group_difference(even_group, group1, &group2);
		MPI_Group_excl(even_group, 6, ranks_to_incl_group1, &group3);
	}
	if (odd_comm != MPI_COMM_NULL) { // Комунікатор для непарних процесів
		int ranks_to_incl_group1[6] = {1, 3, 5, 7, 9,11};

		MPI_Comm_group(odd_comm, &odd_group);

		MPI_Group_incl(odd_group, 6, ranks_to_incl_group1, &group1);
		MPI_Group_excl(odd_group, 6, ranks_to_incl_group1, &group2);
		MPI_Group_union(group2, group1, &group3);
	}

	// 5. Вивід інформації про групи та ранги
	int rank_in_group, group_size;
	if (all_comm != MPI_COMM_NULL) {
		MPI_Group_rank(all_group1, &rank_in_group);
		MPI_Group_size(all_group1, &group_size);

		// Вивід кількості процесів у групах
		if (rank_in_group == 0)
			printf("all_comm:\n");
		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group1: size = %d\n", rank, rank_in_group, group_size);
		}
		MPI_Group_rank(all_group2, &rank_in_group);
		MPI_Group_size(all_group2, &group_size);

		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group2: size = %d\n", rank, rank_in_group, group_size);
		}
		MPI_Group_rank(all_group3, &rank_in_group);
		MPI_Group_size(all_group3, &group_size);

		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group3: size = %d\n", rank, rank_in_group, group_size);
		}
		// Перевіряємо, чи процес належить до групи, отримуючи його ранг у кожній групі
		int rank_in_group1, rank_in_group2, rank_in_group3;
		MPI_Group_rank(all_group1, &rank_in_group1);
		MPI_Group_rank(all_group2, &rank_in_group2);
		MPI_Group_rank(all_group3, &rank_in_group3);

		// Процеси, які є тільки в одній групі
		if (rank_in_group1 != MPI_UNDEFINED && rank_in_group2 == MPI_UNDEFINED && rank_in_group3 == MPI_UNDEFINED) {
			printf("Process %d is only in group 1\n", rank);
		}
		if (rank_in_group2 != MPI_UNDEFINED && rank_in_group1 == MPI_UNDEFINED && rank_in_group3 == MPI_UNDEFINED) {
			printf("Process %d is only in group 2\n", rank);
		}
		if (rank_in_group3 != MPI_UNDEFINED && rank_in_group1 == MPI_UNDEFINED && rank_in_group2 == MPI_UNDEFINED) {
			printf("Process %d is only in group 3\n", rank);
		}

		// Процеси, які є у всіх трьох групах одночасно
		if (rank_in_group1 != MPI_UNDEFINED && rank_in_group2 != MPI_UNDEFINED && rank_in_group3 != MPI_UNDEFINED) {
			printf("Process %d is in all groups\n", rank);
		}
		MPI_Barrier(all_comm);

	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (even_comm != MPI_COMM_NULL) {
		MPI_Barrier(even_comm);
		MPI_Group_rank(group1, &rank_in_group);
		MPI_Group_size(group1, &group_size);

		// Вивід кількості процесів у групах
		if (rank_in_group == 0)
			printf("even_comm:\n");
		MPI_Barrier(even_comm);
		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group1: size = %d\n", rank, rank_in_group, group_size);
		}

		MPI_Group_rank(group2, &rank_in_group);
		MPI_Group_size(group2, &group_size);

		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group2: size = %d\n", rank, rank_in_group, group_size);
		}
		MPI_Group_rank(group3, &rank_in_group);
		MPI_Group_size(group3, &group_size);

		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group3: size = %d\n", rank, rank_in_group, group_size);
		}
		// Перевіряємо, чи процес належить до групи, отримуючи його ранг у кожній групі
		int rank_in_group1, rank_in_group2, rank_in_group3;
		MPI_Group_rank(group1, &rank_in_group1);
		MPI_Group_rank(group2, &rank_in_group2);
		MPI_Group_rank(group3, &rank_in_group3);

		// Процеси, які є тільки в одній групі
		if (rank_in_group1 != MPI_UNDEFINED && rank_in_group2 == MPI_UNDEFINED && rank_in_group3 == MPI_UNDEFINED) {
			printf("Process %d is only in group 1\n", rank);
		}
		if (rank_in_group2 != MPI_UNDEFINED && rank_in_group1 == MPI_UNDEFINED && rank_in_group3 == MPI_UNDEFINED) {
			printf("Process %d is only in group 2\n", rank);
		}
		if (rank_in_group3 != MPI_UNDEFINED && rank_in_group1 == MPI_UNDEFINED && rank_in_group2 == MPI_UNDEFINED) {
			printf("Process %d is only in group 3\n", rank);
		}

		// Процеси, які є у всіх трьох групах одночасно
		if (rank_in_group1 != MPI_UNDEFINED && rank_in_group2 != MPI_UNDEFINED && rank_in_group3 != MPI_UNDEFINED) {
			printf("Process %d is in all groups\n", rank);
		}
		MPI_Barrier(even_comm);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if (odd_comm != MPI_COMM_NULL) {
		MPI_Barrier(odd_comm);
		MPI_Group_rank(group1, &rank_in_group);
		MPI_Group_size(group1, &group_size);

		// Вивід кількості процесів у групах
		if (rank_in_group == 0)
			printf("odd_comm:\n");
		MPI_Barrier(odd_comm);
		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group1: size = %d\n", rank, rank_in_group, group_size);
		}
		MPI_Group_rank(group2, &rank_in_group);
		MPI_Group_size(group2, &group_size);

		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group2: size = %d\n", rank, rank_in_group, group_size);
		}
		MPI_Group_rank(group3, &rank_in_group);
		MPI_Group_size(group3, &group_size);

		if (rank_in_group == 4) {
			printf("\tP:%d) Process %d in group3: size = %d\n", rank, rank_in_group, group_size);
			// Перевіряємо, чи процес належить до групи, отримуючи його ранг у кожній групі

			int rank_in_group1, rank_in_group2, rank_in_group3;
			MPI_Group_rank(group1, &rank_in_group1);
			MPI_Group_rank(group2, &rank_in_group2);
			MPI_Group_rank(group3, &rank_in_group3);

			// Процеси, які є тільки в одній групі
			if (rank_in_group1 != MPI_UNDEFINED && rank_in_group2 == MPI_UNDEFINED && rank_in_group3 == MPI_UNDEFINED) {
				printf("Process %d is only in group 1\n", rank);
			}
			if (rank_in_group2 != MPI_UNDEFINED && rank_in_group1 == MPI_UNDEFINED && rank_in_group3 == MPI_UNDEFINED) {
				printf("Process %d is only in group 2\n", rank);
			}
			if (rank_in_group3 != MPI_UNDEFINED && rank_in_group1 == MPI_UNDEFINED && rank_in_group2 == MPI_UNDEFINED) {
				printf("Process %d is only in group 3\n", rank);
			}

			// Процеси, які є у всіх трьох групах одночасно
			if (rank_in_group1 != MPI_UNDEFINED && rank_in_group2 != MPI_UNDEFINED && rank_in_group3 != MPI_UNDEFINED) {
				printf("Process %d is in all groups\n", rank);
			}
		}
		MPI_Barrier(odd_comm);
	}

	// Кінець виконання
	// (Очищення від груп - не реалізовано)
	MPI_Finalize();

	return 0; // Код повернення у разі успіху
}
