/*
David Ernesto Gomez Gutierrez
20/junio/2015
*/
#include <stdio.h>
#include <string.h>
#include "lp_lib.h"

/*
Variables globales
*/
int demandas_ofertas[6] = {0};
int costos[9] = {600,700,500,320,300,350,500,480,510};
int resultado[9] = {0};
int funcion_objetivo = 0.0;
FILE *archivo_entrada;
FILE *archivo_salida;

void leerArchivo(char *cadena)
{
  char ch;
  char linea[100];
  strcpy(linea,"");
  char caracter[2];
  int numero_temporal = 0;
  int index = 0;


  archivo_entrada = fopen(cadena,"r") ;
  if(!archivo_entrada)
  {
    perror("(1)ERROR No se puede leer archivo o No existe ");
    exit(0);
  }

  while( (ch = fgetc(archivo_entrada)) != EOF)
  {

    if(ch == '\n')
    {
      char *token;
      token = strtok(linea, " ");

      while (token != NULL)
      {
        numero_temporal = atoi(token);
        if(numero_temporal<0)
        {
          perror("(4)ERROR Datos de entrada no validos.");
          exit(-1);
        }
        demandas_ofertas[index] = numero_temporal;
        token = strtok(NULL, " ");
        index++;
      }

      strcpy(linea,"");
    }
    else
    {
      caracter[0] = ch;
      caracter[1] = '\0';
      strncat(linea,caracter,1);
    }
  }

  fclose(archivo_entrada);

}


int ejecutarSimplex()
{
  lprec *lp;

  REAL *row = NULL;
  int *colno = NULL;
  int i,j,k=0;
  char str[5];
  lp = make_lp(0,9);//crea modelo con 9 variables, 0 fil 9 col

  if(lp == NULL) return 0;

  for(i=1;i<10;i++)
  {
    sprintf(str,"x%i",i);

    set_col_name(lp,i,str); //reservo espacio para variables del modelo
    set_binary(lp, i, TRUE);
  }

  //reservo espacio para listas
  colno = (int *) malloc(9 * sizeof(*colno));
  row = (REAL *) malloc(9* sizeof(*row));

  if((colno == NULL) || (row == NULL)) return 0;/*si no se crean*/

  set_add_rowmode(lp, TRUE);/*hacer modelo fila*fila es "mas rapido"*/

  /*
	en este trozo de codigo se agregan todas las restricciones
  */

  for(j=0;j<3;j++)
  {
    i = 0;

    colno[i] = j+1;
    row[i++] = demandas_ofertas[0];

    colno[i] = j+4;
    row[i++] = demandas_ofertas[1];

    colno[i] = j+7;
    row[i++] = demandas_ofertas[2];

    if(!add_constraintex(lp,i,row,colno,GE,demandas_ofertas[3+j])) return 0;

    i = 0;
    row[i++] = 1;
    row[i++] = 1;
    row[i++] = 1;

    if(!add_constraintex(lp,i,row,colno,LE,1)) return 0;
  }

  i = 0;
  colno[i] = 1;
  row[i++] = 1;

  colno[i] = 2;
  row[i++] = 1;

  colno[i] = 3;
  row[i++] = 1;

  if(!add_constraintex(lp,i,row,colno,LE,1)) return 0;


  i = 0;
  colno[i] = 4;
  row[i++] = 1;

  colno[i] = 5;
  row[i++] = 1;

  colno[i] = 6;
  row[i++] = 1;

  if(!add_constraintex(lp,i,row,colno,LE,1)) return 0;

  i = 0;
  colno[i] = 7;
  row[i++] = 1;

  colno[i] = 8;
  row[i++] = 1;

  colno[i] = 9;
  row[i++] = 1;

  if(!add_constraintex(lp,i,row,colno,LE,1)) return 0;


  /*se agrega la funcion objetivo*/
   set_add_rowmode(lp, FALSE);
   k = -1;
   i = 0;

   for(j=1;j<10;j++)
   {
      k++;
      colno[i] = j;
      row[i++] = costos[j-1] * demandas_ofertas[k+3] ;


      if(k==2) k=-1;

    }

  if(!set_obj_fnex(lp, i, row, colno)) return 0;
  set_minim(lp);

  write_lp(lp, "model.lp");
  //write_LP(lp, stdout);
  //libero memoria

  if(solve(lp) != OPTIMAL)
  {
    perror("(2)ERROR Algo ha salido mal el modelo es no factible");
  }
  else
  {
    //printf("Valor funcion objetivo: %f\n", get_objective(lp));
    funcion_objetivo = get_objective(lp);
    get_variables(lp, row);

    for(i=0;i<9;i++)
    {
      //printf("%s: %f \n",get_col_name(lp, i+1), row[i]);
      resultado[i] = row[i];
    }


  }

  delete_lp(lp);
  free(row);
  free(colno);

}

void escribirArchivo()
{
  int planta=1,i=0,j=0;
  char cadena_temporal[50];
  archivo_salida = fopen("salida.data","w");

  for (i = 0;  i < 9; i++)
  {
    j++;
    printf("%i\n",j);
    printf("asd%i \n",resultado[i]);
    if(resultado[i])
    {
      switch (i)
      {
        case 0:
        case 3:
        case 6:
        sprintf(cadena_temporal,"Planta %i: Palmira. \n",planta);
        fputs(cadena_temporal, archivo_salida);
        break;

        case 1:
        case 4:
        case 7:
        sprintf(cadena_temporal,"Planta %i: Tulua. \n",planta);
        fputs(cadena_temporal, archivo_salida);
        break;

        case 2:
        case 5:
        case 8:
        sprintf(cadena_temporal,"Planta %i: Cali. \n",planta);
        fputs(cadena_temporal, archivo_salida);
        break;
      }

    }
    funcion_objetivo += resultado[i] * costos[i];
    if(j==3){planta++;j=0;}

  }

  sprintf(cadena_temporal,"Funcion Objetivo: %i \n",funcion_objetivo);
  fputs(cadena_temporal, archivo_salida);

}



int main(int argc, char const *argv[])
{
  if(argc < 2)
  {
    perror("(3)ERROR Hacen falta argumentos.");
    return 0;
  }


  leerArchivo((char *)argv[1]);
  ejecutarSimplex();
  escribirArchivo();
  return 0;
}
