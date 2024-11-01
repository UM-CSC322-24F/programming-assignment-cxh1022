#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME 128
#define MAX_PLACE 20
#define MAX_TRAILER 6

typedef char NameString[MAX_NAME];
typedef char PlaceString[MAX_PLACE];
typedef char TrailorString[MAX_TRAILER];

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

typedef union {
    int SlipNumber;
    char BayLetter;
    TrailorString TrailorTag;
    int StorageSpace;
} ExtraInfoType;

typedef struct {
    NameString BoatName;
    int Length;
    PlaceType Place;
    ExtraInfoType ExtraInfo;
    double MoneyOwed;
} BoatRecordType;

BoatRecordType *Boats[MAX_BOATS] = {NULL};
//-----------------------------------------------------------------------------
// Function declarations
void * Malloc(size_t Size);
PlaceType StringToPlaceType(char * PlaceString);
char * PlaceToString(PlaceType Place);
int Compare(const void *a, const void *b);
void LoadBoats(BoatRecordType * Boats[MAX_BOATS], char *filename, int *boatCount);
void PrintInventory(BoatRecordType * Boats[MAX_BOATS], int boatCount);
void AddBoat(BoatRecordType * Boats[MAX_BOATS], int *boatCount, char *boatData);
void RemoveBoat(BoatRecordType * Boats[MAX_BOATS], int *boatCount, char *boatName);
void AcceptPayment(BoatRecordType * Boats[MAX_BOATS], int index, double Payment);
void NewMonth(BoatRecordType *Boats[MAX_BOATS], int boatCount);
void SaveBoats(BoatRecordType *Boats[MAX_BOATS], char *filename, int boatCount);
//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  FILE *file;
  BoatRecordType *Boats[MAX_BOATS] = {};
  int boatCount = 0;
  NameString Name = {'\0'};
  char option;
  char boatData[100];

 // Validates arguments and opens file
  if (argc >= 2) {
    if ((file = fopen(argv[1], "r")) == NULL) {
      perror("Error opening File Type");
      exit(EXIT_FAILURE);
    }

  } else {
    exit(EXIT_FAILURE);
  }

  //Sends welcome message
  printf("Welcome to the Boat Management System\n");
  printf("-------------------------------------\n");

//Calls the load boats function and sorts alphabetically
  LoadBoats(Boats, argv[1], &boatCount);
  qsort(Boats, boatCount, sizeof(BoatRecordType *), Compare);

  do {
    printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
    scanf(" %c", &option);

    switch (toupper(option)) {
      //This will print inventory of all boats
      case 'I':
	PrintInventory(Boats, boatCount);
	break;
	
      //This will add a new boat to the inventory based on data entered
      case 'A':
	printf("Please enter the boat data in CSV format: ");
	scanf(" %s", boatData);

	AddBoat(Boats, &boatCount, boatData);
        qsort(Boats, boatCount, sizeof(BoatRecordType *), Compare);
	break;

      //This removes boat selected by the user if it exists
      case 'R':
	printf("Please enter the boat name: ");
	scanf(" %[^\n]", Name);
	RemoveBoat(Boats, &boatCount, Name);
	break;
	
      //This will accept payment if it is not more than the amount owed
      //As long as the boat is found
      case 'P':
	printf("Please enter the boat name:");
        scanf(" %[^\n]", Name);
	int doesExist = 0;
	double Payment;
	// we must verify the boat exists before calling the acceptpayment function
	for (int i = 0; i < boatCount; i++) { 
	  if (strcasecmp(Boats[i]->BoatName, Name) == 0) {
	    printf("Please enter the amount to be paid:");
	    scanf("%lf", &Payment);
	    AcceptPayment(Boats, i, Payment);
	    doesExist = 1;
	    break;	   
	  }
	}if(doesExist == 0){
	   printf("No boat with that name\n");
	}break;
	
      //This will incur new monthly payments
      case 'M':
        NewMonth(Boats, boatCount);
	break;

      //This exits and sends a message to the user
      case 'X':
	printf("\nExiting the Boat Management System\n");
	SaveBoats(Boats, argv[1], boatCount);
        break;

      //If the user chooses an invalid option, this will execute
      default:
	printf("Invalid option %c\n", option);
	break;
    }
  } while (toupper(option) != 'X');

  // Closes file
  if (fclose(file) == EOF) {
    perror("Closing boat file");
    exit(EXIT_FAILURE);
  }
  
  // Frees up memory
  for (int i = 0; i <= boatCount+1; i++) {
    free(Boats[i]);
  }
  return(EXIT_SUCCESS);
}
//-----------------------------------------------------------------------------
//This function allocates Memory
void * Malloc(size_t Size) {

    void * Memory;

    if ((Memory = malloc(Size)) == NULL) {
        perror("Memory Allocation error");
        exit(EXIT_FAILURE);
    } else {
        return(Memory);
    }
}
//-----------------------------------------------------------------------------
//Convert a string to a place
PlaceType StringToPlaceType(char * PlaceString) {
  if (!strcasecmp(PlaceString,"slip")) {
    return(slip);
  }
  if (!strcasecmp(PlaceString,"land")) {
    return(land);
  }
  if (!strcasecmp(PlaceString,"trailor")) {
    return(trailor);
  }
  if (!strcasecmp(PlaceString,"storage")) {
    return(storage);
  }

  return(no_place);
}
//-----------------------------------------------------------------------------
//Convert a place to a string
char * PlaceToString(PlaceType Place) {
  switch (Place) {
    case slip:
      return("slip");

    case land:
      return("land");

    case trailor:
      return("trailor");

    case storage:
      return("storage");

    case no_place:
      return("no_place");

    default:
      printf("How the faaark did I get here?\n");
      exit(EXIT_FAILURE);
      break;
  }
}
//-----------------------------------------------------------------------------
//This function compares boat and is utilized to sort the list
int Compare(const void *a, const void *b) {
    BoatRecordType *boatA = *(BoatRecordType **)a;
    BoatRecordType *boatB = *(BoatRecordType **)b;
    return strcasecmp(boatA->BoatName, boatB->BoatName);
}
//-----------------------------------------------------------------------------
//This function loads the boat file input from user
void LoadBoats(BoatRecordType *Boats[MAX_BOATS], char *filename, int *boatCount) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Error opening file %s \n", filename);
  }

  while (*boatCount < MAX_BOATS) {
    Boats[*boatCount] = (BoatRecordType*)  Malloc(sizeof(BoatRecordType));
    BoatRecordType *boat = Boats[*boatCount];
    PlaceString placeStr;
   
    int input = fscanf(file, "%[^,],%d,%[^,],", boat->BoatName, &boat->Length, placeStr);
    if (input == EOF) {
      free(boat);
      break;
    }
    boat->Place = StringToPlaceType(placeStr);
				    
    switch(boat->Place) {
      case slip:
	fscanf(file, "%d,", &boat->ExtraInfo.SlipNumber);
	break;
    case land:
      fscanf(file, "%c,", &boat->ExtraInfo.BayLetter);
      break;
    case trailor:
      fscanf(file, "%[^,],", boat->ExtraInfo.TrailorTag);
      break;
    case storage:
      fscanf(file, "%d,", &boat->ExtraInfo.StorageSpace);
      break;
    case no_place:
      printf("Invalid place\n");
      exit(EXIT_FAILURE);
    }

    fscanf(file, "%lf\n", &boat->MoneyOwed);
    (*boatCount)++;
  }  
  fclose(file);
}
//-----------------------------------------------------------------------------
//This function adds boat from user input
void AddBoat(BoatRecordType *Boats[MAX_BOATS], int *boatCount, char *boatData) {

  int i = *boatCount;
  PlaceString placeStr;

  if (i >= MAX_BOATS) {
    printf("The marina is full right now... Come back another time!\n");
    return;
  }
  Boats[i] = (BoatRecordType *)Malloc(sizeof(BoatRecordType));
  sscanf(boatData, "%[^,],%d,%[^,]", Boats[i]->BoatName, &Boats[i]->Length, placeStr);
  Boats[i]->Place = StringToPlaceType(placeStr);

  switch(Boats[i]->Place) {
    case slip:
      sscanf(boatData, "%*[^,],%*[^,],%*[^,],%d", &Boats[i]->ExtraInfo.SlipNumber);
      break;

    case land:
      sscanf(boatData, "%*[^,],%*[^,],%*[^,],%c", &Boats[i]->ExtraInfo.BayLetter);
      break;
    case trailor:
      sscanf(boatData, "%*[^,],%*[^,],%*[^,],%[^,]", &Boats[i]->ExtraInfo.TrailorTag);
      break;
    case storage:
      sscanf(boatData, "%*[^,],%*[^,],%*[^,],%d", &Boats[i]->ExtraInfo.StorageSpace);
      break;
    default:
      printf("%s.\n", PlaceToString(Boats[i]->Place));
      break;
  }
  
  sscanf(boatData, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &Boats[i]->MoneyOwed);
  ++(*boatCount);

}
//-----------------------------------------------------------------------------
//Function for removing boats
void RemoveBoat(BoatRecordType *Boats[MAX_BOATS], int *boatCount, char *boatName) {
  for (int i = 0; i < *boatCount; i++) {
    if (strcasecmp(Boats[i]->BoatName, boatName) == 0) {
      free(Boats[i]);
	  
      for (int j = i; j < *boatCount - 1; j++) {
	Boats[j] = Boats[j + 1];
      }
      (*boatCount)--;
      return;
    }

  }
  printf("No boat with that name\n");// if boat is not found, it will reach this point
  return;
}
//-----------------------------------------------------------------------------
void AcceptPayment(BoatRecordType *Boats[MAX_BOATS], int index, double Payment) {
  if (Payment > Boats[index]->MoneyOwed) {
    printf("That is more than the amount owed, $%.2lf\n", Boats[index]->MoneyOwed);
    return;
  }
  Boats[index]->MoneyOwed -= Payment;
}
//-----------------------------------------------------------------------------
//This function indicates it is a new month so the fees are incurred based on length and type
void NewMonth(BoatRecordType *Boats[MAX_BOATS], int boatCount) {

  for (int i = 0; i < boatCount; i++) {
    double monthlyCharge = 0.00;
    switch (Boats[i]->Place) {
      case slip:
	monthlyCharge = 12.50 * Boats[i]->Length;
        break;
	  
      case land:
        monthlyCharge = 14.00 * Boats[i]->Length;
        break;

      case trailor:
        monthlyCharge = 25.00 * Boats[i]->Length;
        break;
	  
      case storage:
        monthlyCharge = 11.20 * Boats[i]->Length;
        break;

      default:
        printf("%s - cannot update monthly charge.\n", PlaceToString(Boats[i]->Place));
        break;
    }
    Boats[i]->MoneyOwed += monthlyCharge;
  }
}

//-----------------------------------------------------------------------------
//This prints all boats saved in the Boats pointer array
void PrintInventory(BoatRecordType *Boats[MAX_BOATS], int boatCount) {
  for (int i = 0; i < boatCount; i++) {
    BoatRecordType *Boat = Boats[i];
    printf("%-20s %5d'  %-5s ", Boat->BoatName, Boat->Length, PlaceToString(Boat->Place));
    switch (Boat->Place) {
      case slip:
	printf("%3s %d", "#", Boat->ExtraInfo.SlipNumber);
	break;
	
      case land:
	printf("%6c", Boat->ExtraInfo.BayLetter);
	break;
	
      case trailor:
	printf("%6s", Boat->ExtraInfo.TrailorTag);
	break;

      case storage:
        printf("%3s %d", "#", Boat->ExtraInfo.StorageSpace);
	break;
	
      case no_place:
	break;
	
      default:
	break;
    }
    printf("   Owes $%5.2f\n", Boat->MoneyOwed);

  }
}
//-----------------------------------------------------------------------------
//This function will save all of the boats including the ones added before exiting
void SaveBoats(BoatRecordType *Boats[MAX_BOATS], char *filename, int boatCount) {
  FILE *file = fopen(filename, "w");

  if (!file) {
    printf("Error opening file %c\n", *filename);
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < boatCount; i++) {
    BoatRecordType *boat = Boats[i];

      
    fprintf(file, "%s,%d,%s,", boat->BoatName, boat->Length, PlaceToString(boat->Place));

    switch (boat->Place) {
      case slip:
        fprintf(file, "%d,", boat->ExtraInfo.SlipNumber);
        break;
	
      case land:
        fprintf(file, "%c,", boat->ExtraInfo.BayLetter);
	break;

      case trailor:
	fprintf(file, "%s,", boat->ExtraInfo.TrailorTag);
        break;
	
      case storage:
	fprintf(file, "%d,", boat->ExtraInfo.StorageSpace);
        break;
	
      default:
	fprintf(file, ",");
    }
    
    fprintf(file, "%.2f\n", boat->MoneyOwed);
  }
  if (fclose(file) == EOF) {
    perror("Closing file");
    exit(EXIT_FAILURE);
  }
}

//--------------------------------------------------
//Finito
	
